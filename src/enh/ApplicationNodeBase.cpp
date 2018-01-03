/**
 * @file   ApplicationNodeImplementation.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.11.30
 *
 * @brief  Implementation of the application node class.
 */

#include "ApplicationNodeBase.h"

#include "core/glfw.h"
#include "enh/gfx/mesh/SimpleMeshRenderer.h"
#include <glbinding/Binding.h>
#include <glbinding/callbacks.h>
#include <glbinding/gl/gl.h>

void ecb(const glbinding::FunctionCall & call) {
    std::stringstream callOut;
    callOut << call.function->name() << "(";
    for (unsigned i = 0; i < call.parameters.size(); ++i)
    {
        callOut << call.parameters[i]->asString();
        if (i < call.parameters.size() - 1)
            callOut << ", ";
    }
    callOut << ")";

    if (call.returnValue)
        callOut << " -> " << call.returnValue->asString();

    LOG(DBUG) << callOut.str();

    const auto error = gl::glGetError();

    auto doprint = true;
    std::string errorStr;
    switch (error)
    {
    case gl::GL_INVALID_ENUM:
        errorStr = "GL_INVALID_ENUM";
        break;
    case gl::GL_INVALID_VALUE:
        errorStr = "GL_INVALID_VALUE";
        break;
    case gl::GL_INVALID_OPERATION:
        errorStr = "GL_INVALID_OPERATION";
        break;
    case gl::GL_INVALID_FRAMEBUFFER_OPERATION:
        errorStr = "GL_INVALID_FRAMEBUFFER_OPERATION";
        break;
    case gl::GL_OUT_OF_MEMORY:
        errorStr = "GL_OUT_OF_MEMORY";
        break;
    case gl::GL_STACK_UNDERFLOW:
        errorStr = "GL_STACK_UNDERFLOW";
        break;
    case gl::GL_STACK_OVERFLOW:
        errorStr = "GL_STACK_OVERFLOW";
        break;
    case gl::GL_TABLE_TOO_LARGE:
        errorStr = "GL_TABLE_TOO_LARGE";
        break;
    case gl::GL_TEXTURE_TOO_LARGE_EXT:
        errorStr = "GL_TEXTURE_TOO_LARGE_EXT";
        break;
    default:
        doprint = false;
        break;
    }

    if (doprint) {
        LOG(WARNING) << "Error: " << errorStr;
    }
}

namespace viscom::enh {

    ApplicationNodeBase::ApplicationNodeBase(ApplicationNodeInternal* appNode) :
        viscom::ApplicationNodeBase{ appNode }
    {
    }

    ApplicationNodeBase::~ApplicationNodeBase() = default;

    void ApplicationNodeBase::InitOpenGL()
    {
        {
            using namespace glbinding;
            Binding::initialize();
#ifdef VISCOM_OGL_DEBUG_MSGS
            setCallbackMaskExcept(CallbackMask::After | CallbackMask::ParametersAndReturnValue, { "glGetError" });
            setAfterCallback(ecb);
#endif // VISCOM_OGL_DEBUG_MSGS
        }

        simpleMeshes_ = std::make_unique<SimpleMeshRenderer>(this);
    }

}
