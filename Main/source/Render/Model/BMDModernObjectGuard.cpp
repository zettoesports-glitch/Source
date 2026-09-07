#include "stdafx.h"

#if jdk_shader_local330

#include "ZzzObject.h"

#include <fstream>
#include <unordered_set>

bool BMDModernShouldForceLegacyObject(const OBJECT* object)
{
    if (object == NULL)
        return false;

    const bool forceLegacy =
        object->Kind == KIND_NPC ||
        object->Kind == KIND_MONSTER;

    if (!forceLegacy)
        return false;

    // Log each live OBJECT address once. Besides documenting the temporary
    // safety quarantine, this gives us concrete per-instance identity and
    // position data for the shared-BMD transform bug (one NPC appearing at
    // another NPC's position).
    static std::unordered_set<const OBJECT*> loggedObjects;
    if (loggedObjects.insert(object).second)
    {
        std::ofstream logFile("Data\\ModernBMD.log", std::ios::out | std::ios::app);
        if (logFile.is_open())
        {
            logFile
                << "[ModernBMD] object-instance guard: forcing legacy renderer"
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
