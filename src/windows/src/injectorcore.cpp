#include <vector>
#include <algorithm>

#include "windows/injectorcore.h"
#include "windows/classresolver.h"
#include "windows/functionresolver.h"
#include "windows/utility.h"
#include "windows/behaviorchangerfactory.h"

namespace InjectorPP
{
bool InjectorCore::m_isSymInitialized = false;

InjectorCore::InjectorCore()
    : m_behaviorChanger(NULL),
      m_classResolver(NULL),
      m_functionResolver(NULL),
      m_symbolInfoHelper(NULL),
      m_currentProcessHandler(NULL)
{
    this->m_currentProcessHandler = GetCurrentProcess();
    this->m_behaviorChanger = BehaviorChangerFactory::create();
    this->m_functionResolver = new FunctionResolver(this->m_currentProcessHandler);
    this->m_classResolver = new ClassResolver(this->m_currentProcessHandler, this->m_functionResolver);
    this->m_symbolInfoHelper = new SymbolInfoHelper();
}

InjectorCore::~InjectorCore()
{
    // Release all allocated type instances.
    std::vector<void *>::iterator it;
    for (it = this->m_allocatedTypeInstances.begin(); it != this->m_allocatedTypeInstances.end(); ++it)
    {
        if (*it != NULL)
        {
            delete *it;
            *it = NULL;
        }
    }

    // Release all fake classes.
    std::vector<Class *>::iterator classIt;
    for (classIt = this->m_resolvedClasses.begin(); classIt != this->m_resolvedClasses.end(); ++classIt)
    {
        if (*classIt != NULL)
        {
            delete *classIt;
            *classIt = NULL;
        }
    }

    // Release all original function asms and recover the replaced functions.
    this->recoverAllReplacedFunctions();

    if (this->m_symbolInfoHelper != NULL)
    {
        delete this->m_symbolInfoHelper;
        this->m_symbolInfoHelper = NULL;
    }

    if (this->m_classResolver != NULL)
    {
        delete this->m_classResolver;
        this->m_classResolver = NULL;
    }

    if (this->m_functionResolver != NULL)
    {
        delete this->m_functionResolver;
        this->m_functionResolver = NULL;
    }

    if (this->m_behaviorChanger != NULL)
    {
        delete m_behaviorChanger;
        m_behaviorChanger = NULL;
    }
}

void InjectorCore::initialize()
{
    if (!InjectorCore::m_isSymInitialized)
    {
        // Let's initialize the whole symbol system.
        if (SymInitialize(this->m_currentProcessHandler, NULL, TRUE) == FALSE)
        {
            throw;
        }

        InjectorCore::m_isSymInitialized = true;
    }
}

void *InjectorCore::getVirtualMethodAddress(void *classInstance, const std::string &virtualMethodName)
{
    void *result = NULL;

    // The method name stored in pdb may be decorated. We should decorate our
    // virtual method name either.
    std::string decoratedMethodName = "?" + virtualMethodName + "@";

    ULONG64 *vptrForClass = *(ULONG64 **)*(ULONG64 **)&classInstance;
    ULONG64 **virtualMethodAddress = (ULONG64 **)vptrForClass;
    while (*virtualMethodAddress != NULL)
    {
        ULONG64 address = (ULONG64)*virtualMethodAddress;
        std::string funcSymName = this->m_functionResolver->getMethodSymbolFromAddress(address);

        // Is this function symbol name contains the virtual method name?
        if (funcSymName.find(decoratedMethodName) != std::string::npos || funcSymName.find("::" + virtualMethodName) != std::string::npos)
        {
            // Yes! We found the virtual method address!
            result = *virtualMethodAddress;

            break;
        }

        virtualMethodAddress += 1;
    }

    if (result == NULL)
    {
        throw;
    }

    return result;
}

void InjectorCore::replaceFunction(void *srcFunc, void *destFunc, bool isSourceMemberFunction, bool isSourceStaticMemberFunction, bool isSourceVirtualMemberFunction)
{
    bool isComplexReturn = false;
    ResolvedType returnType = this->m_functionResolver->getMethodReturnTypeFromAddress((ULONG64)srcFunc);
    if (returnType.SymbolTag != SymTagEnum::SymTagBaseType && returnType.SymbolTag != SymTagEnum::SymTagPointerType)
    {
        isComplexReturn = true;
    }

    // Save the original asm code.
    // Useful when we recover the function behavior.
    OriginalFuncASM *originalFuncAsm = new OriginalFuncASM();

    FunctionType functionType = FunctionType::GlobalFunction;
    if (isSourceMemberFunction)
    {
        if (isSourceStaticMemberFunction)
        {
            functionType = FunctionType::StaticMemberFunction;
        }
        else
        {
            functionType = isSourceVirtualMemberFunction ? FunctionType::VirtualMemberFunction : FunctionType::NonVirtualMemberFunction;
        }
    }
    FunctionReturnType functionReturnType = isComplexReturn ? FunctionReturnType::ComplexReturnType : FunctionReturnType::BasicReturnType;

    this->m_behaviorChanger->replaceFunction((ULONG64)srcFunc, (ULONG64)destFunc, originalFuncAsm, functionType, functionReturnType);

    if (!this->saveOriginalFuncASM(originalFuncAsm))
    {
        delete originalFuncAsm;
        originalFuncAsm = NULL;
    }
}

void InjectorCore::recoverAllReplacedFunctions()
{
    // Release all original function asms.
    std::vector<OriginalFuncASM *>::iterator ofaIt;
    for (ofaIt = this->m_originalFunctionASMVector.begin(); ofaIt != this->m_originalFunctionASMVector.end(); ++ofaIt)
    {
        if (*ofaIt != NULL)
        {
            // Recover the original function behavior.
            this->m_behaviorChanger->directWriteToFunction((*ofaIt)->funcAddress, (*ofaIt)->asmCode, 6);

            delete *ofaIt;
            *ofaIt = NULL;
        }
    }

    this->m_originalFunctionASMVector.clear();
}

bool InjectorCore::saveOriginalFuncASM(OriginalFuncASM *originalFuncASM)
{
    if (originalFuncASM->funcAddress == 0)
    {
        return false;
    }

    std::vector<OriginalFuncASM *>::iterator it = this->m_originalFunctionASMVector.begin();
    for (; it != this->m_originalFunctionASMVector.end(); ++it)
    {
        if ((*it)->funcAddress == originalFuncASM->funcAddress)
        {
            // The original asm already been stored.
            return false;
        }
    }

    this->m_originalFunctionASMVector.push_back(originalFuncASM);

    return true;
}
}
