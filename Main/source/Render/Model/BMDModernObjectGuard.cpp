#include "stdafx.h"

#if jdk_shader_local330

#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzBMD.h"
#include "ZzzOpenglUtil.h"

#include <cstring>
#include <fstream>
#include <unordered_set>
#include <vector>

namespace
{
    int g_LegacyDoubleSidedDepth = 0;
    GLboolean g_LegacyDoubleSidedPreviousGlCull = GL_FALSE;
    bool g_LegacyDoubleSidedPreviousCullTracker = false;
    const OBJECT* g_SelectedRemoteObject = NULL;
    const OBJECT* g_SelectedNpcMonsterObject = NULL;
    std::vector<const OBJECT*> g_RenderScopeStack;

    struct ObjectLogKey
    {
        const OBJECT* Address;
        int Kind;
        int Type;
        int BaseClass;

        bool operator==(const ObjectLogKey& other) const
        {
            return Address == other.Address &&
                   Kind == other.Kind &&
                   Type == other.Type &&
                   BaseClass == other.BaseClass;
        }
    };

    struct ObjectLogKeyHash
    {
        size_t operator()(const ObjectLogKey& key) const
        {
            size_t value = reinterpret_cast<size_t>(key.Address);
            value ^= static_cast<size_t>(key.Kind + 17) * static_cast<size_t>(2654435761u);
            value ^= static_cast<size_t>(key.Type + 257) * static_cast<size_t>(2246822519u);
            value ^= static_cast<size_t>(key.BaseClass + 37) * static_cast<size_t>(3266489917u);
            return value;
        }
    };

    ObjectLogKey MakeObjectLogKey(const OBJECT* object, int baseClass)
    {
        ObjectLogKey key;
        key.Address = object;
        key.Kind = object != NULL ? static_cast<int>(object->Kind) : -1;
        key.Type = object != NULL ? object->Type : -1;
        key.BaseClass = baseClass;
        return key;
    }

    bool IsRemotePlayerLikeObject(const OBJECT* object)
    {
        return object != NULL &&
               object->Kind == KIND_PLAYER &&
               Hero != NULL &&
               object != &Hero->Object;
    }

    bool IsNpcOrMonsterObject(const OBJECT* object)
    {
        return object != NULL &&
               (object->Kind == KIND_NPC || object->Kind == KIND_MONSTER);
    }

    bool ForceLegacyRemotePlayers()
    {
        static const bool value =
            GetPrivateProfileIntA("ModernRenderer", "ForceLegacyRemotePlayers", 1,
                                  ".\\Data\\Custom\\config.ini") != 0;
        return value;
    }

    int RemotePlayerClassFilter()
    {
        static const int value =
            GetPrivateProfileIntA("ModernRenderer", "RemotePlayerClass", -1,
                                  ".\\Data\\Custom\\config.ini");
        return value;
    }

    bool RemotePlayerSingleObject()
    {
        static const bool value =
            GetPrivateProfileIntA("ModernRenderer", "RemotePlayerSingleObject", 1,
                                  ".\\Data\\Custom\\config.ini") != 0;
        return value;
    }

    bool NpcMonsterRolloutEnabled()
    {
        static const bool value =
            GetPrivateProfileIntA("ModernRenderer", "NpcMonsterRollout", 0,
                                  ".\\Data\\Custom\\config.ini") != 0;
        return value;
    }

    int NpcMonsterTypeFilter()
    {
        static const int value =
            GetPrivateProfileIntA("ModernRenderer", "NpcMonsterType", -1,
                                  ".\\Data\\Custom\\config.ini");
        return value;
    }

    bool NpcMonsterSingleObject()
    {
        static const bool value =
            GetPrivateProfileIntA("ModernRenderer", "NpcMonsterSingleObject", 1,
                                  ".\\Data\\Custom\\config.ini") != 0;
        return value;
    }

    bool IsStaleSelectedRemoteObject(const OBJECT* object)
    {
        if (object == NULL)
            return true;
        if (!object->Live)
            return true;
        if (object->Kind != KIND_PLAYER)
            return true;
        if (Hero != NULL && object == &Hero->Object)
            return true;
        return false;
    }

    void ClearSelectedRemoteIfStale()
    {
        if (g_SelectedRemoteObject == NULL || !IsStaleSelectedRemoteObject(g_SelectedRemoteObject))
            return;

        static bool staleClearLogged = false;
        if (!staleClearLogged)
        {
            staleClearLogged = true;
            std::ofstream logFile("Data\\ModernBMD.log", std::ios::out | std::ios::app);
            if (logFile.is_open())
            {
                logFile
                    << "[ModernBMD] remote-rollout isolation: selected remote object went stale; allowing reselection"
                    << " object=" << g_SelectedRemoteObject
                    << " kind=" << static_cast<unsigned int>(g_SelectedRemoteObject->Kind)
                    << " live=" << (g_SelectedRemoteObject->Live ? 1 : 0)
                    << "\n";
            }
        }

        g_SelectedRemoteObject = NULL;
    }

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

    bool RemoteClassAllowed(const OBJECT* object)
    {
        if (!IsRemotePlayerLikeObject(object) || ForceLegacyRemotePlayers())
            return false;

        const int classFilter = RemotePlayerClassFilter();
        if (classFilter < 0)
            return true;

        return GetRemoteBaseClass(object) == classFilter;
    }

    bool RemoteObjectAllowedByIsolation(const OBJECT* object)
    {
        if (!RemoteClassAllowed(object))
            return false;

        if (!RemotePlayerSingleObject())
            return true;

        ClearSelectedRemoteIfStale();
        return g_SelectedRemoteObject != NULL && object == g_SelectedRemoteObject;
    }

    bool NpcMonsterMatchesFilter(const OBJECT* object)
    {
        if (!NpcMonsterRolloutEnabled() || !IsNpcOrMonsterObject(object))
            return false;

        const int typeFilter = NpcMonsterTypeFilter();
        return typeFilter < 0 || object->Type == typeFilter;
    }

    bool IsStaleSelectedNpcMonsterObject(const OBJECT* object)
    {
        if (object == NULL)
            return true;
        if (!object->Live)
            return true;
        if (!NpcMonsterMatchesFilter(object))
            return true;
        return false;
    }

    void ClearSelectedNpcMonsterIfStale()
    {
        if (g_SelectedNpcMonsterObject == NULL ||
            !IsStaleSelectedNpcMonsterObject(g_SelectedNpcMonsterObject))
        {
            return;
        }

        static bool staleClearLogged = false;
        if (!staleClearLogged)
        {
            staleClearLogged = true;
            std::ofstream logFile("Data\\ModernBMD.log", std::ios::out | std::ios::app);
            if (logFile.is_open())
            {
                logFile
                    << "[ModernBMD] npc/monster rollout: selected object went stale; allowing reselection"
                    << " object=" << g_SelectedNpcMonsterObject
                    << " kind=" << static_cast<unsigned int>(g_SelectedNpcMonsterObject->Kind)
                    << " type=" << g_SelectedNpcMonsterObject->Type
                    << " live=" << (g_SelectedNpcMonsterObject->Live ? 1 : 0)
                    << "\n";
            }
        }

        g_SelectedNpcMonsterObject = NULL;
    }

    bool NpcMonsterObjectAllowedByIsolation(const OBJECT* object)
    {
        if (!NpcMonsterMatchesFilter(object))
            return false;

        // After a type has passed single-instance validation, this switch can
        // admit every live object of that same configured type without widening
        // the rollout to any other NPC/monster model. Setting it back to 1
        // restores the original first-live-object quarantine immediately.
        if (!NpcMonsterSingleObject())
            return true;

        ClearSelectedNpcMonsterIfStale();
        if (g_SelectedNpcMonsterObject == NULL)
            g_SelectedNpcMonsterObject = object;

        return object == g_SelectedNpcMonsterObject;
    }

    bool ObjectAllowedByIsolation(const OBJECT* object)
    {
        return RemoteObjectAllowedByIsolation(object) ||
               NpcMonsterObjectAllowedByIsolation(object);
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

    bool IsSos3Bi01LegacyWorldAsset(const OBJECT* object)
    {
        if (object == NULL || object->Kind != 0 || Models == NULL || object->Type < 0)
            return false;

        const char* modelName = Models[object->Type].Name;
        if (modelName == NULL || modelName[0] == '\0')
            return false;

        // This animated world asset is the one currently producing exploded
        // geometry on the ModernBMD path. Keep only this exact asset on legacy;
        // Kind == 0 is also used by weapons, skills and helper objects.
        return std::strstr(modelName, "Object52\\sos3bi01.smd") != NULL;
    }
}

bool BMDModernIsSelectedRemoteRolloutObject(const OBJECT* object)
{
    ClearSelectedRemoteIfStale();
    return object != NULL && g_SelectedRemoteObject != NULL && object == g_SelectedRemoteObject;
}

void BMDModernPushRenderScope(const OBJECT* object)
{
    g_RenderScopeStack.push_back(object);
}

void BMDModernPopRenderScope()
{
    if (!g_RenderScopeStack.empty())
        g_RenderScopeStack.pop_back();
}

const OBJECT* BMDModernCurrentRenderScope()
{
    if (g_RenderScopeStack.empty())
        return NULL;
    return g_RenderScopeStack.back();
}

bool BMDModernAllowModernForCurrentRenderScope()
{
    static const bool remoteRolloutIsolation =
        GetPrivateProfileIntA("ModernRenderer", "RemoteRolloutIsolation", 0,
                              ".\\Data\\Custom\\config.ini") != 0;

    if (!remoteRolloutIsolation)
        return true;

    if (g_RenderScopeStack.empty())
        return false;

    return ObjectAllowedByIsolation(g_RenderScopeStack.back());
}

bool BMDModernAllowModernForCommand(const OBJECT* owner)
{
    static const bool remoteRolloutIsolation =
        GetPrivateProfileIntA("ModernRenderer", "RemoteRolloutIsolation", 0,
                              ".\\Data\\Custom\\config.ini") != 0;
    if (!remoteRolloutIsolation)
        return true;

    // A queued draw already carries the OBJECT that owned it when the command
    // was recorded. Do not consult the mutable render-scope stack here: flushes
    // can happen after a nested scope has popped or changed. The same immutable
    // owner gate now admits either the configured remote-player rollout or the
    // configured NPC/monster type rollout.
    return ObjectAllowedByIsolation(owner);
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

    const bool isRemotePlayerLike = IsRemotePlayerLikeObject(object);
    const bool legacySos3Bi01WorldAsset = IsSos3Bi01LegacyWorldAsset(object);

    // Safe default remains legacy. ForceLegacyRemotePlayers=0 explicitly opens
    // the player rollout. NPC/monster rollout is a separate opt-in; its type
    // filter stays mandatory while NpcMonsterSingleObject controls whether one
    // or all live instances of that exact type are admitted.
    const bool forceLegacyRemotePlayers = ForceLegacyRemotePlayers();
    const int remotePlayerClass = RemotePlayerClassFilter();
    const bool remotePlayerSingleObject = RemotePlayerSingleObject();
    const bool npcMonsterSingleObject = NpcMonsterSingleObject();

    const int remoteBaseClass = isRemotePlayerLike ? GetRemoteBaseClass(object) : -1;
    const bool remoteClassRejected =
        isRemotePlayerLike && !forceLegacyRemotePlayers &&
        remotePlayerClass >= 0 && remoteBaseClass != remotePlayerClass;

    bool remoteObjectRejected = false;
    if (isRemotePlayerLike && !forceLegacyRemotePlayers && !remoteClassRejected && remotePlayerSingleObject)
    {
        ClearSelectedRemoteIfStale();
        if (g_SelectedRemoteObject == NULL)
            g_SelectedRemoteObject = object;
        remoteObjectRejected = g_SelectedRemoteObject != object;
    }

    const bool remotePlayerLike =
        isRemotePlayerLike &&
        (forceLegacyRemotePlayers || remoteClassRejected || remoteObjectRejected);

    const bool npcOrMonster = IsNpcOrMonsterObject(object);
    const bool npcMonsterFilterMatched = NpcMonsterMatchesFilter(object);
    const bool npcMonsterAllowed =
        npcOrMonster && !legacySos3Bi01WorldAsset &&
        NpcMonsterObjectAllowedByIsolation(object);

    const bool forceLegacy =
        legacySos3Bi01WorldAsset ||
        (npcOrMonster && !npcMonsterAllowed) ||
        remotePlayerLike;

    if (!forceLegacy)
    {
        if (isRemotePlayerLike)
        {
            static std::unordered_set<ObjectLogKey, ObjectLogKeyHash> loggedModernRemoteObjects;
            const ObjectLogKey key = MakeObjectLogKey(object, remoteBaseClass);
            if (loggedModernRemoteObjects.size() < 32u &&
                loggedModernRemoteObjects.insert(key).second)
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
                        << " selected=" << (BMDModernIsSelectedRemoteRolloutObject(object) ? 1 : 0)
                        << " position=(" << object->Position[0]
                        << "," << object->Position[1]
                        << "," << object->Position[2] << ")"
                        << " scale=" << object->Scale
                        << " rollback=ForceLegacyRemotePlayers"
                        << "\n";
                }
            }
        }
        else if (npcMonsterAllowed)
        {
            static std::unordered_set<ObjectLogKey, ObjectLogKeyHash> loggedModernNpcMonsterObjects;
            const ObjectLogKey key = MakeObjectLogKey(object, -1);
            if (loggedModernNpcMonsterObjects.size() < 16u &&
                loggedModernNpcMonsterObjects.insert(key).second)
            {
                std::ofstream logFile("Data\\ModernBMD.log", std::ios::out | std::ios::app);
                if (logFile.is_open())
                {
                    const char* modelName = "<unavailable>";
                    if (Models != NULL && object->Type >= 0 && Models[object->Type].Name[0] != '\0')
                        modelName = Models[object->Type].Name;

                    logFile
                        << "[ModernBMD] object-instance rollout: npc/monster allowed to modern candidate"
                        << " object=" << object
                        << " kind=" << static_cast<unsigned int>(object->Kind)
                        << " type=" << object->Type
                        << " targetType=" << NpcMonsterTypeFilter()
                        << " singleObject=" << (npcMonsterSingleObject ? 1 : 0)
                        << " selected=" << (npcMonsterSingleObject && g_SelectedNpcMonsterObject == object ? 1 : 0)
                        << " model=" << modelName
                        << " position=(" << object->Position[0]
                        << "," << object->Position[1]
                        << "," << object->Position[2] << ")"
                        << " scale=" << object->Scale
                        << " rollback=NpcMonsterRollout"
                        << "\n";
                }
            }
        }
        return false;
    }

    const char* reason = legacySos3Bi01WorldAsset ? "world-asset-sos3bi01" : "npc/monster";
    if (remotePlayerLike)
    {
        if (forceLegacyRemotePlayers)
            reason = "remote-player/bot";
        else if (remoteClassRejected)
            reason = "remote-class-filter";
        else if (remoteObjectRejected)
            reason = "remote-single-object-filter";
    }
    else if (npcOrMonster && NpcMonsterRolloutEnabled())
    {
        if (!npcMonsterFilterMatched)
            reason = "npc/monster-type-filter";
        else if (npcMonsterSingleObject)
            reason = "npc/monster-single-object-filter";
    }

    // OBJECT slots are reused by the client. Key diagnostics by the observed
    // address + kind/type/class signature rather than by address alone, otherwise
    // an NPC logged from a slot can hide a later player (or another model) that
    // reuses that same OBJECT storage during the same validation session.
    static std::unordered_set<ObjectLogKey, ObjectLogKeyHash> loggedObjects;
    const ObjectLogKey key = MakeObjectLogKey(object, isRemotePlayerLike ? remoteBaseClass : -1);
    if (loggedObjects.insert(key).second)
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
            else if (npcOrMonster && NpcMonsterRolloutEnabled())
            {
                logFile
                    << " targetType=" << NpcMonsterTypeFilter()
                    << " singleObject=" << (npcMonsterSingleObject ? 1 : 0);
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