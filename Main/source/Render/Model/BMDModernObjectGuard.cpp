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

    // Keep the real GL state disabled while making the legacy state tracker
    // believe culling is already enabled. Opaque RenderMesh passes call
    // DisableAlphaBlend(), which normally calls EnableCullFace(); with the
    // tracker held true that call becomes a no-op and the one-sided clothing
    // remains visible. Nested layout scopes reassert the same state.
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

    // Remote player/BotBuffer rollout exposed class/equipment deformation on
    // Elf/BK while a simple SM remained visually correct. Keep remote players
    // on the known-good legacy renderer by default while the multi-pose atlas
    // and linked-item transform path are isolated. Set ForceLegacyRemotePlayers=0
    // explicitly for controlled diagnostics; the setting is read once per client.
    static const bool forceLegacyRemotePlayers =
        GetPrivateProfileIntA("ModernRenderer", "ForceLegacyRemotePlayers", 1,
                              ".\\Data\\Custom\\config.ini") != 0;

    const bool remotePlayerLike = isRemotePlayerLike && forceLegacyRemotePlayers;

    // NPC/monster shared-BMD instances remain quarantined until the remote
    // player/BotBuffer composite-render path has full per-instance parity.
    const bool npcOrMonster =
        object->Kind == KIND_NPC ||
        object->Kind == KIND_MONSTER;

    const bool forceLegacy = npcOrMonster || remotePlayerLike;

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

    const char* reason = remotePlayerLike ? "remote-player/bot" : "npc/monster";

    // Log each live OBJECT address once. Besides documenting the temporary
    // safety quarantine, this gives us concrete per-instance identity and
    // position data for the shared-BMD transform bug. Include the BMD name so
    // visual-only legacy regressions (for example one-sided clothing meshes)
    // can be isolated to the exact asset without broad renderer changes.
    static std::unordered_set<const OBJECT*> loggedObjects;
    if (loggedObjects.insert(object).second)
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
                << " model=" << modelName
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
