#include "stdafx.h"

#if jdk_shader_local330

#include "ZzzObject.h"
#include "ZzzCharacter.h"

#include <fstream>
#include <unordered_set>

bool BMDModernShouldForceLegacyObject(const OBJECT* object)
{
    if (object == NULL)
        return false;

    const bool isRemotePlayerLike =
        object->Kind == KIND_PLAYER &&
        Hero != NULL &&
        object != &Hero->Object;

    // Staged rollout: remote players/BotBuffer are now allowed to reach the
    // modern BMD path so the immutable per-command palette/transform snapshot
    // can be validated with multiple live OBJECT instances. Keep an emergency
    // runtime rollback that does not require recompilation.
    const bool forceLegacyRemotePlayers =
        GetPrivateProfileIntA("ModernRenderer", "ForceLegacyRemotePlayers", 0,
                              ".\\Data\\Custom\\config.ini") != 0;

    const bool remotePlayerLike = isRemotePlayerLike && forceLegacyRemotePlayers;

    // NPC/monster shared-BMD instances remain quarantined until the remote
    // player/BotBuffer rollout proves that independent live instances keep
    // their own pose and transform under the modern batch/atlas path.
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
    // position data for the shared-BMD transform bug.
    static std::unordered_set<const OBJECT*> loggedObjects;
    if (loggedObjects.insert(object).second)
    {
        std::ofstream logFile("Data\\ModernBMD.log", std::ios::out | std::ios::app);
        if (logFile.is_open())
        {
            logFile
                << "[ModernBMD] object-instance guard: forcing legacy renderer"
                << " reason=" << reason
                << " object=" << object
                << " kind=" << static_cast<unsigned int>(object->Kind)
                << " type=" << object->Type
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
