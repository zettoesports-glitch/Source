#include "stdafx.h"

#if jdk_shader_local330

#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzBMD.h"
#include "ZzzOpenglUtil.h"

#include <cstring>
#include <fstream>
#include <unordered_set>

namespace
{
    int g_LegacyDoubleSidedDepth = 0;
    GLboolean g_LegacyDoubleSidedPreviousGlCull = GL_FALSE;
    bool g_LegacyDoubleSidedPreviousCullTracker = false;

    const CHARACTER* FindCharacterForObject(const OBJECT* object)
    {
        if (object == NULL || CharactersClient == NULL)
            return NULL;

        for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
        {
            const CHARACTER* character = &CharactersClient[i];
            if (&character->Object == object)
                return character;
        }

        return NULL;
    }

    int GetRemoteBaseClass(const OBJECT* object)
    {
        const CHARACTER* character = FindCharacterForObject(object);
        if (character == NULL)
            return -1;

        // CharacterManager::GetBaseClass() is Class & 0x7. Keep this guard
        // dependency-light while preserving promoted-class compatibility.
        return static_cast<int>(character->Class) & 0x7;
    }

    const char* RemoteBaseClassName(int baseClass)
    {
        switch (baseClass)
        {
        case CLASS_WIZARD: return "wizard";
        case CLASS_KNIGHT: return "knight";
        case CLASS_ELF: return "elf";
        case CLASS_DARK: return "magic-gladiator";
        case CLASS_DARK_LORD: return "dark-lord";
        case CLASS_SUMMONER: return "summoner";
#ifdef PBG_ADD_NEWCHAR_MONK
        case CLASS_RAGEFIGHTER: return "rage-fighter";
#endif
        default: return "unknown";
        }
    }

    bool IsMerchantFemaleLegacyDoubleSidedCandidate(const OBJECT* object)
    {
        if (object == NULL || Models == NULL || object->Type < 0)
            return false;

        const char* modelName = Models[object->Type].Name;
        if (modelName == NULL || modelName[0] == '\0')
            return false;

        // The affected NPC is loaded as Data2\\Npc\\merchant_f.smd in this
        // client data set. Match the asset name rather than a numeric model id
        // so the workaround remains isolated if enum values move.
        return std::strstr(modelName, "merchant_f") != NULL;
    }
}

bool BMDModernLegacyCullSuppressed()
{
    return g_LegacyDoubleSidedDepth > 0;
}

bool BMDModernBeginLegacyDoubleSidedObject(const OBJECT* object)
{
    if (!IsMerchantFemaleLegacyDoubleSidedCandidate(object))
        return false;

    if (g_LegacyDoubleSidedDepth == 0)
    {
        g_LegacyDoubleSidedPreviousGlCull = glIsEnabled(GL_CULL_FACE);
        g_LegacyDoubleSidedPreviousCullTracker = CullFaceEnable;

        static bool logged = false;
        if (!logged)
        {
            logged = true;
            std::ofstream logFile("Data\\ModernBMD.log", std::ios::out | std::ios::app);
            if (logFile.is_open())
            {
                logFile
                    << "[ModernBMD] legacy double-sided override active for merchant_f"
                    << " type=" << object->Type
                    << " model=" << Models[object->Type].Name
                    << " reason=one-sided-clothing-cull"
                    << "\n";
            }
        }
    }

    ++g_LegacyDoubleSidedDepth;

    // Disable culling physically. CoreGLCompat's filtered glEnable path blocks
    // attempts to re-enable GL_CULL_FACE while this scoped override is active.
    // Keep the legacy tracker at true so its common opaque path remains stable.
    glDisable(GL_CULL_FACE);
    CullFaceEnable = true;
    return true;
}

void BMDModernEndLegacyDoubleSidedObject(bool active)
{
    if (!active || g_LegacyDoubleSidedDepth <= 0)
        return;

    --g_LegacyDoubleSidedDepth;
    if (g_LegacyDoubleSidedDepth == 0)
    {
        if (g_LegacyDoubleSidedPreviousGlCull == GL_TRUE)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);

        CullFaceEnable = g_LegacyDoubleSidedPreviousCullTracker;
    }
    else
    {
        // A nested renderer scope ended while the merchant is still being
        // rendered. Reassert the temporary physical/logical split.
        glDisable(GL_CULL_FACE);
        CullFaceEnable = true;
    }
}

bool BMDModernShouldForceLegacyObject(const OBJECT* object)
{
    if (object == NULL)
        return false;

    const bool isRemotePlayerLike =
        object->Kind == KIND_PLAYER &&
        Hero != NULL &&
        object != &Hero->Object;

    // Ordinary map/world objects keep Kind == 0 from OBJECT::Initialize().
    // Animated scenery is not part of the current character-focused ModernBMD
    // rollout yet, so keep it on the established legacy transform/render path.
    const bool worldObject = object->Kind == 0;

    // Safe default remains legacy. ForceLegacyRemotePlayers=0 explicitly opens
    // the diagnostic rollout. RemotePlayerClass then allows only one base class
    // (-1 = any; 0 wizard, 1 knight/BK, 2 elf), and RemotePlayerSingleObject=1
    // keeps only the first matching live OBJECT on the modern candidate path.
    static const bool forceLegacyRemotePlayers =
        GetPrivateProfileIntA("ModernRenderer", "ForceLegacyRemotePlayers", 1,
                              ".\\Data\\Custom\\config.ini") != 0;
    static const int remotePlayerClass =
        GetPrivateProfileIntA("ModernRenderer", "RemotePlayerClass", -1,
                              ".\\Data\\Custom\\config.ini");
    static const bool remotePlayerSingleObject =
        GetPrivateProfileIntA("ModernRenderer", "RemotePlayerSingleObject", 1,
                              ".\\Data\\Custom\\config.ini") != 0;

    const int remoteBaseClass = isRemotePlayerLike ? GetRemoteBaseClass(object) : -1;
    const bool remoteClassRejected =
        isRemotePlayerLike && !forceLegacyRemotePlayers &&
        remotePlayerClass >= 0 && remoteBaseClass != remotePlayerClass;

    static const OBJECT* selectedRemoteObject = NULL;
    bool remoteObjectRejected = false;
    if (isRemotePlayerLike && !forceLegacyRemotePlayers && !remoteClassRejected && remotePlayerSingleObject)
    {
        if (selectedRemoteObject == NULL)
            selectedRemoteObject = object;
        remoteObjectRejected = selectedRemoteObject != object;
    }

    const bool remotePlayerLike =
        isRemotePlayerLike &&
        (forceLegacyRemotePlayers || remoteClassRejected || remoteObjectRejected);

    // NPC/monster shared-BMD instances remain quarantined until the remote
    // player/BotBuffer composite-render path has full per-instance parity.
    const bool npcOrMonster =
        object->Kind == KIND_NPC ||
        object->Kind == KIND_MONSTER;

    const bool forceLegacy = worldObject || npcOrMonster || remotePlayerLike;

    if (!forceLegacy)
    {
        if (isRemotePlayerLike)
        {
            static std::unordered_set<const OBJECT*> loggedModernRemoteObjects;
            if (loggedModernRemoteObjects.size() < 8u &&
                loggedModernRemoteObjects.insert(object).second)
            {
                std::ofstream logFile("Data\\ModernBMD.log", std::ios::out | std::ios::app);
                if (logFile.is_open())
                {
                    logFile
                        << "[ModernBMD] object-instance rollout: remote-player/bot allowed to modern candidate"
                        << " object=" << object
                        << " type=" << object->Type
                        << " class=" << remoteBaseClass
                        << " className=" << RemoteBaseClassName(remoteBaseClass)
                        << " classFilter=" << remotePlayerClass
                        << " singleObject=" << (remotePlayerSingleObject ? 1 : 0)
                        << " position=(" << object->Position[0]
                        << "," << object->Position[1]
                        << "," << object->Position[2] << ")"
                        << " scale=" << object->Scale
                        << " rollback=ForceLegacyRemotePlayers"
                        << "\n";
                }
            }
        }
        return false;
    }

    const char* reason = worldObject ? "world-object" : "npc/monster";
    if (remotePlayerLike)
    {
        if (forceLegacyRemotePlayers)
            reason = "remote-player/bot";
        else if (remoteClassRejected)
            reason = "remote-class-filter";
        else if (remoteObjectRejected)
            reason = "remote-single-object-filter";
    }

    // Avoid flooding diagnostics with the many one-bone walls/stones in map
    // object pools. Log animated/skinned world objects, while preserving full
    // diagnostics for NPC/monster and remote-player quarantine decisions.
    bool shouldLog = !worldObject;
    if (worldObject && Models != NULL && object->Type >= 0)
        shouldLog = Models[object->Type].NumBones > 1;

    static std::unordered_set<const OBJECT*> loggedObjects;
    if (shouldLog && loggedObjects.insert(object).second)
    {
        std::ofstream logFile("Data\\ModernBMD.log", std::ios::out | std::ios::app);
        if (logFile.is_open())
        {
            const char* modelName = "<unavailable>";
            if (Models != NULL && object->Type >= 0 && Models[object->Type].Name[0] != '\0')
                modelName = Models[object->Type].Name;

            logFile
                << "[ModernBMD] object-instance guard: forcing legacy renderer"
                << " reason=" << reason
                << " object=" << object
                << " kind=" << static_cast<unsigned int>(object->Kind)
                << " type=" << object->Type
                << " model=" << modelName;

            if (isRemotePlayerLike)
            {
                logFile
                    << " class=" << remoteBaseClass
                    << " className=" << RemoteBaseClassName(remoteBaseClass)
                    << " classFilter=" << remotePlayerClass
                    << " singleObject=" << (remotePlayerSingleObject ? 1 : 0);
            }

            logFile
                << " position=(" << object->Position[0]
                << "," << object->Position[1]
                << "," << object->Position[2] << ")"
                << " scale=" << object->Scale
                << "\n";
        }
    }

    return true;
}

#endif // jdk_shader_local330
