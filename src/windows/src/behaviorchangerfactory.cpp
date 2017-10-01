#include <stdexcept>
#include "windows/behaviorchangerfactory.h"
#ifdef WINENVIRONMENT32
#include "windows/x86windowsbehaviorchanger.h"
#endif

namespace InjectorPP
{
    BehaviorChangerFactory::BehaviorChangerFactory()
    {
    }

    BehaviorChangerFactory::~BehaviorChangerFactory()
    {
    }

    BehaviorChanger* BehaviorChangerFactory::create()
    {
#ifdef WINENVIRONMENT32
        return new X86WindowsBehaviorChanger();
#else
        throw std::logic_error("No supported BehaviorChanger found for current environment.");
#endif
    }
}