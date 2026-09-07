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

    // BotBuffer objects are sent through the player-like viewport path. Until
    // every shared BMD transform is captured per OBJECT instance, keep all
    // remote player-like objects on the legacy renderer. The local Hero stays
    // eligible for the modern path so Matrix4x4/atlas validation can continue.
    const bool remotePlayerLike =
        object->Kind == KIND_PLAYER &&
        Hero != NULL &&
        object != &Hero->Object;

    const bool npcOrMonster =
        object->Kind == KIND_NPC ||
        object->Kind == KIND_MONSTER;

    const bool forceLegacy = npcOrMonster || remotePlayerLike;

    if (!forceLegacy)
        return false;

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
