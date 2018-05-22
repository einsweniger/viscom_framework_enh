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
#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
#include <glbinding/Meta.h>
#include "enh/gfx/gl/GLTexture.h"

void ecb(const glbinding::FunctionCall & call) {
    std::stringstream callOut;
    callOut << call.function->name() << "(";
    for (unsigned i = 0; i < call.parameters.size(); ++i) {
        callOut << call.parameters[i]->asString();
        // callOut << call.parameters[i].get();
        if (i < call.parameters.size() - 1)
            callOut << ", ";
    }
    callOut << ")";

    if (call.returnValue)
        callOut << " -> " << call.returnValue->asString();
        //callOut << " -> " << call.returnValue.get();

    LOG(DBUG) << callOut.str();

    const auto error = gl::glGetError();

    if (gl::GL_NO_ERROR != error) {
        // LOG(WARNING) << "Error: " << glbinding::aux::Meta::getString(error);
        LOG(WARNING) << "Error: " << glbinding::Meta::getString(error);
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
            // Binding::initialize([](const char * name) { return glfwGetProcAddress(name); }, false);
            Binding::initialize();
#ifdef VISCOM_OGL_DEBUG_MSGS
            setCallbackMaskExcept(CallbackMask::After | CallbackMask::ParametersAndReturnValue, { "glGetError" });
            setAfterCallback(ecb);
#endif // VISCOM_OGL_DEBUG_MSGS
        }

        simpleMeshes_ = std::make_unique<SimpleMeshRenderer>(this);

        cubicWeightsTexture_ = std::make_unique<GLTexture>(256, TextureDescriptor{ 12, gl::GL_RGB32F, gl::GL_RGB, gl::GL_FLOAT });

        std::array<glm::vec3, 256> hg_precalc;
        for (std::size_t i = 0; i < hg_precalc.size(); ++i) {
            auto x = static_cast<float>(i) / static_cast<float>(hg_precalc.size());
            auto x2 = x * x;
            auto x3 = x2 * x;

            std::array<float, 4> w;
            w[0] = (1.0f - (3.0f * x) + (3.0f * x2) - x3) / 6.0f;
            w[1] = ((3.0f * x3) - (6.0f * x2) + 4.0f) / 6.0f;
            w[2] = (1.0f + (3.0f * x) + (3.0f * x2) - (3.0f * x3)) / 6.0f;
            w[3] = x3 / 6.0f;

            hg_precalc[i].z = w[0] + w[1];
            hg_precalc[i].x = 1.0f - (w[1] / hg_precalc[i].z) + x;
            hg_precalc[i].y = 1.0f + (w[3] / (w[2] + w[3])) - x;
        }

        cubicWeightsTexture_->SetData(hg_precalc.data());
        cubicWeightsTexture_->SampleLinear();
        cubicWeightsTexture_->SampleWrapRepeat();
    }

    void ApplicationNodeBase::CleanUp()
    {
        simpleMeshes_ = nullptr;
        cubicWeightsTexture_ = nullptr;
    }

}
