/**
 * @file   FilmicTMOperator.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.12.21
 *
 * @brief  Implementation of the filmic tone-mapping operator.
 */

#include "FilmicTMOperator.h"
#include "enh/ApplicationNodeBase.h"
#include "core/gfx/FrameBuffer.h"
#include "enh/gfx/gl/GLUniformBuffer.h"
#include "enh/gfx/gl/GLTexture.h"
#include "enh/core/serializationHelper.h"
#include <imgui.h>

namespace viscom::enh {

    FilmicTMOperator::FilmicTMOperator(ApplicationNodeBase* app) :
        renderable_(app->CreateFullscreenQuad("tm/filmic.fp")),
        uniformIds_(renderable_->GetGPUProgram()->getUniformLocations({ "sourceTex" })),
        filmicUBO_(new GLUniformBuffer("filmicBuffer", sizeof(FilmicTMParameters), app->GetUBOBindingPoints()))
    {
        params_.sStrength_ = 0.15f;
        params_.linStrength_ = 0.5f;
        params_.linAngle_ = 0.1f;
        params_.toeStrength_ = 0.2f;
        params_.toeNumerator_ = 0.02f;
        params_.toeDenominator_ = 0.3f;
        params_.white_ = 11.2f;
        params_.exposure_ = 2.0f;

        app->GetUBOBindingPoints()->BindBufferBlock(renderable_->GetGPUProgram()->getProgramId(), "filmicBuffer");

        // Alternative values:
        /*params.sStrength = 0.22f;
        params.linStrength = 0.3f;
        params.linAngle = 0.1f;
        params.toeStrength = 0.2f;
        params.toeNumerator = 0.1f;
        params.toeDenominator = 0.3f;
        params.white = 11.2f;
        params.exposure = 2.0f;*/
    }

    /** Default destructor. */
    FilmicTMOperator::~FilmicTMOperator() = default;

    void FilmicTMOperator::RenderParameterSliders()
    {
        if (ImGui::TreeNode("Filmic TM Parameters"))
        {
            ImGui::InputFloat("Shoulder Strength", &params_.sStrength_, 0.01f);
            ImGui::InputFloat("Linear Strength", &params_.linStrength_, 0.1f);
            ImGui::InputFloat("Linear Angle", &params_.linAngle_, 0.01f);
            ImGui::InputFloat("Toe Strength", &params_.toeStrength_, 0.1f);
            ImGui::InputFloat("Toe Numerator", &params_.toeNumerator_, 0.01f);
            ImGui::InputFloat("Toe Denominator", &params_.toeDenominator_, 0.1f);
            ImGui::InputFloat("White", &params_.white_, 0.1f);
            ImGui::TreePop();
        }
        // no gamma on sRGB targets
        // TwAddVarRW(bar, "Gamma", TW_TYPE_FLOAT, &params.gamma, " label='Gamma' min=1.0 max=3.0 step=0.1");
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    // ReSharper disable once CppMemberFunctionMayBeConst
    void FilmicTMOperator::Resize(const glm::uvec2&)
    {
    }

    void FilmicTMOperator::ApplyTonemapping(GLTexture* sourceRT, FrameBuffer& fbo)
    {
        filmicUBO_->UploadData(0, sizeof(FilmicTMParameters), &params_);
        filmicUBO_->BindBuffer();

        fbo.DrawToFBO([this, sourceRT]() {
            gl::glUseProgram(renderable_->GetGPUProgram()->getProgramId());
            sourceRT->ActivateTexture(gl::GL_TEXTURE0);
            gl::glUniform1i(uniformIds_[0], 0);
            renderable_->Draw();
            gl::glUseProgram(0);
        });
    }

    void FilmicTMOperator::SaveParameters(std::ostream& ostr) const
    {
        serializeHelper::write(ostr, std::string("FilmicTMOperator"));
        serializeHelper::write(ostr, VERSION);
        serializeHelper::write(ostr, params_);
    }

    void FilmicTMOperator::LoadParameters(std::istream& istr)
    {
        std::string clazzName;
        unsigned int version;
        serializeHelper::read(istr, clazzName);
        if (clazzName != "FilmicTMOperator") throw std::runtime_error("Serialization Error: wrong class.");
        serializeHelper::read(istr, version);
        if (version > VERSION) throw std::runtime_error("Serialization Error: wrong version.");

        serializeHelper::read(istr, params_);
    }
}
