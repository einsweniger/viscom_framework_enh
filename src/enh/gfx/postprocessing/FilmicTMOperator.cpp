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
#include <imgui.h>

namespace viscom::enh {

    FilmicTMOperator::FilmicTMOperator(ApplicationNodeBase* app) :
        renderable_(app->CreateFullscreenQuad("tm/filmic.frag")),
        uniformIds_(renderable_->GetGPUProgram()->GetUniformLocations({ "sourceTex" })),
        filmicUBO_(std::make_unique<GLUniformBuffer>("filmicBuffer", sizeof(FilmicTMParameters), app->GetUBOBindingPoints()))
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
    }

    void FilmicTMOperator::ApplyTonemappingInternal(GLuint sourceTex)
    {
        filmicUBO_->UploadData(0, sizeof(FilmicTMParameters), &params_);
        filmicUBO_->BindBuffer();

        gl::glUseProgram(renderable_->GetGPUProgram()->getProgramId());

        gl::glActiveTexture(gl::GL_TEXTURE0 + 0);
        gl::glBindTexture(gl::GL_TEXTURE_2D, sourceTex);
        gl::glUniform1i(uniformIds_[0], 0);
        renderable_->Draw();
        gl::glUseProgram(0);
    }

    void FilmicTMOperator::ApplyTonemapping(GLuint sourceTex, const FrameBuffer* fbo, std::size_t drawBufferIndex)
    {
        fbo->DrawToFBO(std::vector<std::size_t>{drawBufferIndex}, [this, sourceTex]() { ApplyTonemappingInternal(sourceTex); });
    }

    void FilmicTMOperator::ApplyTonemapping(GLuint sourceTex, const FrameBuffer* fbo)
    {
        fbo->DrawToFBO([this, sourceTex]() { ApplyTonemappingInternal(sourceTex); });
    }
}
