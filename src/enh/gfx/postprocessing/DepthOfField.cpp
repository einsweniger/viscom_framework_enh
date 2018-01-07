/**
 * @file   DepthOfField.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.02.11
 *
 * @brief  Implementation of the depth of field effect.
 */

#include "DepthOfField.h"
#include "core/gfx/FrameBuffer.h"
#include "enh/ApplicationNodeBase.h"
#include "enh/gfx/gl/GLTexture.h"
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

namespace viscom::enh {

    namespace dof {
        struct DoFPassParams {
            GLuint colorTex_;
            GLuint depthTex_;
            const FrameBuffer* fullResRT_;
            const FrameBuffer* lowResRT_;
            glm::vec2 projParams_;
            glm::vec2 cocParams_;
        };

        std::array<glm::vec3, 48> circleBokeh = {
            glm::vec3(1.000000f, 0.000000f, 2.0f),
            glm::vec3(0.707107f, 0.707107f, 2.0f),
            glm::vec3(-0.000000f, 1.000000f, 2.0f),
            glm::vec3(-0.707107f, 0.707107f, 2.0f),
            glm::vec3(-1.000000f, -0.000000f, 2.0f),
            glm::vec3(-0.707106f, -0.707107f, 2.0f),
            glm::vec3(0.000000f, -1.000000f, 2.0f),
            glm::vec3(0.707107f, -0.707107f, 2.0f),

            glm::vec3(1.000000f, 0.000000f, 4.0f),
            glm::vec3(0.923880f, 0.382683f, 4.0f),
            glm::vec3(0.707107f, 0.707107f, 4.0f),
            glm::vec3(0.382683f, 0.923880f, 4.0f),
            glm::vec3(-0.000000f, 1.000000f, 4.0f),
            glm::vec3(-0.382684f, 0.923879f, 4.0f),
            glm::vec3(-0.707107f, 0.707107f, 4.0f),
            glm::vec3(-0.923880f, 0.382683f, 4.0f),
            glm::vec3(-1.000000f, -0.000000f, 4.0f),
            glm::vec3(-0.923879f, -0.382684f, 4.0f),
            glm::vec3(-0.707106f, -0.707107f, 4.0f),
            glm::vec3(-0.382683f, -0.923880f, 4.0f),
            glm::vec3(0.000000f, -1.000000f, 4.0f),
            glm::vec3(0.382684f, -0.923879f, 4.0f),
            glm::vec3(0.707107f, -0.707107f, 4.0f),
            glm::vec3(0.923880f, -0.382683f, 4.0f),

            glm::vec3(1.000000f, 0.000000f, 6.0f),
            glm::vec3(0.965926f, 0.258819f, 6.0f),
            glm::vec3(0.866025f, 0.500000f, 6.0f),
            glm::vec3(0.707107f, 0.707107f, 6.0f),
            glm::vec3(0.500000f, 0.866026f, 6.0f),
            glm::vec3(0.258819f, 0.965926f, 6.0f),
            glm::vec3(-0.000000f, 1.000000f, 6.0f),
            glm::vec3(-0.258819f, 0.965926f, 6.0f),
            glm::vec3(-0.500000f, 0.866025f, 6.0f),
            glm::vec3(-0.707107f, 0.707107f, 6.0f),
            glm::vec3(-0.866026f, 0.500000f, 6.0f),
            glm::vec3(-0.965926f, 0.258819f, 6.0f),
            glm::vec3(-1.000000f, -0.000000f, 6.0f),
            glm::vec3(-0.965926f, -0.258820f, 6.0f),
            glm::vec3(-0.866025f, -0.500000f, 6.0f),
            glm::vec3(-0.707106f, -0.707107f, 6.0f),
            glm::vec3(-0.499999f, -0.866026f, 6.0f),
            glm::vec3(-0.258819f, -0.965926f, 6.0f),
            glm::vec3(0.000000f, -1.000000f, 6.0f),
            glm::vec3(0.258819f, -0.965926f, 6.0f),
            glm::vec3(0.500000f, -0.866025f, 6.0f),
            glm::vec3(0.707107f, -0.707107f, 6.0f),
            glm::vec3(0.866026f, -0.499999f, 6.0f),
            glm::vec3(0.965926f, -0.258818f, 6.0f),
        };
    }

    DepthOfField::DepthOfField(ApplicationNodeBase* app) :
        app_{ app },
        cocQuad_{ "dof/coc.frag", app },
        cocUniformIds_{ cocQuad_.GetGPUProgram()->GetUniformLocations({ "depthTex", "projParams", "cocParams" }) },
        downsampleQuad_{ "dof/downsample.frag", app },
        downsampleUniformIds_{ downsampleQuad_.GetGPUProgram()->GetUniformLocations({ "colorTex", "cocTex" }) },
        tileMinMaxCoCQuad_{ FullscreenQuad{"dof/tileMinMaxXCoC.frag", "dof/tileMinMaxCoC.frag", std::vector<std::string>{ "HORIZONTAL" }, app},
            FullscreenQuad{"dof/tileMinMaxYCoC.frag", "dof/tileMinMaxCoC.frag", std::vector<std::string>{ "VERTICAL" }, app} },
        tileMinMaxCoCUniformIds_{ tileMinMaxCoCQuad_[0].GetGPUProgram()->GetUniformLocations({ "cocTex" }), 
            tileMinMaxCoCQuad_[1].GetGPUProgram()->GetUniformLocations({ "cocTex" }) },
        nearCoCBlurQuad_{ FullscreenQuad{"dof/nearCoCBlurX.frag", "dof/nearCoCBlur.frag", std::vector<std::string>{ "HORIZONTAL" }, app},
            FullscreenQuad{ "dof/nearCoCBlurY.frag", "dof/nearCoCBlur.frag", std::vector<std::string>{ "VERTICAL" }, app } },
        nearCoCBlurUniformIds_{ nearCoCBlurQuad_[0].GetGPUProgram()->GetUniformLocations({ "cocTex" }), nearCoCBlurQuad_[1].GetGPUProgram()->GetUniformLocations({ "cocTex" }) },
        dofQuad_{ "dof/dof.frag", app },
        dofUniformIds_{ dofQuad_.GetGPUProgram()->GetUniformLocations({ "cocTex", "cocNearBlurTex", "colorTex", "colorMulCoCFarTex", "offsets" }) },
        fillQuad_{ "dof/fill.frag", app },
        fillUniformIds_{ fillQuad_.GetGPUProgram()->GetUniformLocations({ "cocTex", "cocNearBlurTex", "dofNearTex", "dofFarTex" }) },
        compositeQuad_{ "dof/composite.frag", app },
        compositeUniformIds_{ compositeQuad_.GetGPUProgram()->GetUniformLocations({ "colorTex", "cocTex", "cocHalfTex", "cocNearBlurHalfTex", "dofNearHalfTex", "dofFarHalfTex", "hgTex", "blend" }) }
    {
        params_.focusZ_ = 3.3f;
        params_.imageDistance_ = 0.005f;
        params_.fStops_ = 5.6f;
        params_.fStopsMin_ = 1.8f;
        params_.fStopsMax_ = 22.0f;
        params_.bokehShape_ = 7;
        params_.rotateBokehMax_ = glm::pi<float>() / 3.0f;
        params_.blendFactor_ = 1.0f;

        FrameBufferDescriptor fullResRTDesc{ { FrameBufferTextureDescriptor{ static_cast<GLenum>(gl::GL_RGB32F) } }, {} }; // CoC near/far/depth
        fullResRTs_ = app_->CreateOffscreenBuffers(fullResRTDesc);

        FrameBufferDescriptor lowResRTDesc{ {
                FrameBufferTextureDescriptor{ static_cast<GLenum>(gl::GL_RGB32F) }, // 0: color / nearFieldFill
                FrameBufferTextureDescriptor{ static_cast<GLenum>(gl::GL_RGB32F) }, // 1: colorMulCoCFar / farFieldFilled
                FrameBufferTextureDescriptor{ static_cast<GLenum>(gl::GL_RGB32F) }, // 2: nearField
                FrameBufferTextureDescriptor{ static_cast<GLenum>(gl::GL_RGB32F) }, // 3: farField
                FrameBufferTextureDescriptor{ static_cast<GLenum>(gl::GL_RG32F) }, // 4: CoC near/far
                FrameBufferTextureDescriptor{ static_cast<GLenum>(gl::GL_RG32F) }, // 5: CoC near/far ping
                FrameBufferTextureDescriptor{ static_cast<GLenum>(gl::GL_RG32F) } // 6: CoC near/far pong <- final
            }, {} };
        lowResRTs_ = app_->CreateOffscreenBuffers(lowResRTDesc, 2);

        downsamplePassDrawBuffers_ = { 0, 1, 4 };
        tilePassDrawBuffers_[0] = { 5 };
        tilePassDrawBuffers_[1] = { 6 };
        dofPassDrawBuffers_ = { 2, 3 };
        fillPassDrawBuffers_ = { 0, 1 };
    }

    DepthOfField::~DepthOfField() = default;

    void DepthOfField::RenderParameterSliders()
    {
        if (ImGui::TreeNode("DepthOfField Parameters"))
        {
            ImGui::InputFloat("Focus Plane", &params_.focusZ_, 0.005f);
            ImGui::InputFloat("Image Distance", &params_.imageDistance_, 0.001f);
            if (ImGui::InputFloat("f-Stops", &params_.fStops_, 0.1f)) recalcBokeh_ = true;
            if (ImGui::InputFloat("f-Stops Min", &params_.fStopsMin_, 0.1f)) recalcBokeh_ = true;
            if (ImGui::InputFloat("f-Stops Max", &params_.fStopsMax_, 0.1f)) recalcBokeh_ = true;
            if (ImGui::InputInt("Bokeh Shape", &params_.bokehShape_)) recalcBokeh_ = true;
            if (ImGui::InputFloat("Max Bokeh Rotation", &params_.rotateBokehMax_, 0.5f)) recalcBokeh_ = true;
            ImGui::InputFloat("Blend Factor", &params_.blendFactor_, 0.1f);
            ImGui::TreePop();
        }
    }

    void DepthOfField::RecalcBokeh()
    {
        auto f = (params_.fStops_ - params_.fStopsMin_) / (params_.fStopsMax_ - params_.fStopsMin_);
        auto bokehRotation = f * params_.rotateBokehMax_;
        auto N = static_cast<float>(params_.bokehShape_);
        auto piDivN = glm::pi<float>() / N;
        for (std::size_t i = 0; i < dof::circleBokeh.size(); ++i) {
            float theta = glm::acos(dof::circleBokeh[i].x);
            if (dof::circleBokeh[i].y < 0.0f) theta = glm::two_pi<float>() - theta;

            float newTheta = theta + bokehRotation;

            float rScale = glm::cos(piDivN) / glm::cos(theta - 2.0f * piDivN * glm::floor((N * theta + glm::pi<float>()) / glm::two_pi<float>()));
            float newR = dof::circleBokeh[i].z * glm::pow(rScale, f);

            bokehTaps_[i] = newR * glm::vec2(glm::cos(newTheta), glm::sin(newTheta));
        }

        recalcBokeh_ = false;
    }

    void DepthOfField::CoCPass(const dof::DoFPassParams& passParams)
    {
        passParams.fullResRT_->DrawToFBO([this, &passParams]() {
            gl::glUseProgram(cocQuad_.GetGPUProgram()->getProgramId());

            gl::glActiveTexture(gl::GL_TEXTURE0);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.depthTex_);

            gl::glUniform1i(cocUniformIds_[0], 0);
            gl::glUniform2fv(cocUniformIds_[1], 1, glm::value_ptr(passParams.projParams_));
            gl::glUniform2fv(cocUniformIds_[2], 1, glm::value_ptr(passParams.cocParams_));
            cocQuad_.Draw();
        });
    }

    void DepthOfField::DownsamplePass(const dof::DoFPassParams& passParams)
    {
        passParams.lowResRT_->DrawToFBO(downsamplePassDrawBuffers_, [this, &passParams]() {
            gl::glUseProgram(downsampleQuad_.GetGPUProgram()->getProgramId());

            gl::glActiveTexture(gl::GL_TEXTURE0);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.colorTex_);
            gl::glActiveTexture(gl::GL_TEXTURE1);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.fullResRT_->GetTextures()[0]);

            gl::glUniform1i(downsampleUniformIds_[0], 0);
            gl::glUniform1i(downsampleUniformIds_[1], 1);
            downsampleQuad_.Draw();
        });
    }

    void DepthOfField::TileMinMaxPass(const dof::DoFPassParams& passParams, std::size_t pass, std::size_t sourceTex)
    {
        passParams.lowResRT_->DrawToFBO(tilePassDrawBuffers_[pass], [this, &passParams, pass, sourceTex]() {
            gl::glUseProgram(tileMinMaxCoCQuad_[pass].GetGPUProgram()->getProgramId());

            gl::glActiveTexture(gl::GL_TEXTURE0);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.lowResRT_->GetTextures()[sourceTex]);

            gl::glUniform1i(tileMinMaxCoCUniformIds_[pass][0], 0);
            tileMinMaxCoCQuad_[pass].Draw();
        });
    }

    void DepthOfField::NearCoCBlurPass(const dof::DoFPassParams& passParams, std::size_t pass, std::size_t sourceTex)
    {
        passParams.lowResRT_->DrawToFBO(tilePassDrawBuffers_[pass], [this, &passParams, pass, sourceTex]() {
            gl::glUseProgram(nearCoCBlurQuad_[pass].GetGPUProgram()->getProgramId());

            gl::glActiveTexture(gl::GL_TEXTURE0);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.lowResRT_->GetTextures()[sourceTex]);

            gl::glUniform1i(nearCoCBlurUniformIds_[pass][0], 0);
            nearCoCBlurQuad_[pass].Draw();
        });
    }

    void DepthOfField::ComputeDoFPass(const dof::DoFPassParams& passParams)
    {
        passParams.lowResRT_->DrawToFBO(dofPassDrawBuffers_, [this, &passParams]() {
            gl::glUseProgram(dofQuad_.GetGPUProgram()->getProgramId());

            gl::glActiveTexture(gl::GL_TEXTURE0 + 0);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.lowResRT_->GetTextures()[4]);
            gl::glActiveTexture(gl::GL_TEXTURE0 + 1);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.lowResRT_->GetTextures()[6]);
            gl::glActiveTexture(gl::GL_TEXTURE0 + 2);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.lowResRT_->GetTextures()[0]);
            gl::glActiveTexture(gl::GL_TEXTURE0 + 3);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.lowResRT_->GetTextures()[1]);

            gl::glUniform1i(dofUniformIds_[0], 0);
            gl::glUniform1i(dofUniformIds_[1], 1);
            gl::glUniform1i(dofUniformIds_[2], 2);
            gl::glUniform1i(dofUniformIds_[3], 3);
            gl::glUniform2fv(dofUniformIds_[4], static_cast<gl::GLsizei>(bokehTaps_.size()), reinterpret_cast<float*>(bokehTaps_.data()));
            dofQuad_.Draw();
        });
    }

    void DepthOfField::FillPass(const dof::DoFPassParams& passParams)
    {
        passParams.lowResRT_->DrawToFBO(fillPassDrawBuffers_, [this, &passParams]() {
            gl::glUseProgram(fillQuad_.GetGPUProgram()->getProgramId());

            gl::glActiveTexture(gl::GL_TEXTURE0 + 0);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.lowResRT_->GetTextures()[4]);
            gl::glActiveTexture(gl::GL_TEXTURE0 + 1);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.lowResRT_->GetTextures()[6]);
            gl::glActiveTexture(gl::GL_TEXTURE0 + 2);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.lowResRT_->GetTextures()[2]);
            gl::glActiveTexture(gl::GL_TEXTURE0 + 3);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.lowResRT_->GetTextures()[3]);

            gl::glUniform1i(fillUniformIds_[0], 0);
            gl::glUniform1i(fillUniformIds_[1], 1);
            gl::glUniform1i(fillUniformIds_[2], 2);
            gl::glUniform1i(fillUniformIds_[3], 3);
            fillQuad_.Draw();
        });
    }

    void DepthOfField::CompositePass(const dof::DoFPassParams& passParams, const FrameBuffer* targetFBO)
    {
        targetFBO->DrawToFBO([this, &passParams]() {
            gl::glUseProgram(compositeQuad_.GetGPUProgram()->getProgramId());

            gl::glActiveTexture(gl::GL_TEXTURE0 + 0);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.colorTex_);
            gl::glActiveTexture(gl::GL_TEXTURE0 + 1);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.fullResRT_->GetTextures()[0]);
            gl::glActiveTexture(gl::GL_TEXTURE0 + 2);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.lowResRT_->GetTextures()[4]);
            gl::glActiveTexture(gl::GL_TEXTURE0 + 3);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.lowResRT_->GetTextures()[6]);
            gl::glActiveTexture(gl::GL_TEXTURE0 + 4);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.lowResRT_->GetTextures()[0]);
            gl::glActiveTexture(gl::GL_TEXTURE0 + 5);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.lowResRT_->GetTextures()[1]);
            app_->GetCubicWeightsTexture().ActivateTexture(gl::GL_TEXTURE0 + 6);

            gl::glUniform1i(compositeUniformIds_[0], 0);
            gl::glUniform1i(compositeUniformIds_[1], 1);
            gl::glUniform1i(compositeUniformIds_[2], 2);
            gl::glUniform1i(compositeUniformIds_[3], 3);
            gl::glUniform1i(compositeUniformIds_[4], 4);
            gl::glUniform1i(compositeUniformIds_[5], 5);
            gl::glUniform1i(compositeUniformIds_[6], 6);
            gl::glUniform1f(compositeUniformIds_[7], params_.blendFactor_);
            compositeQuad_.Draw();
        });
    }

    void DepthOfField::ApplyEffect(const CameraHelper& cam, GLuint colorTex, GLuint depthTex, const FrameBuffer* targetFBO)
    {
        dof::DoFPassParams passParams;
        passParams.colorTex_ = colorTex;
        passParams.depthTex_ = depthTex;
        passParams.fullResRT_ = app_->SelectOffscreenBuffer(fullResRTs_);
        passParams.lowResRT_ = app_->SelectOffscreenBuffer(lowResRTs_);
        passParams.projParams_.x = cam.GetPerspectiveMatrix()[2][2];
        passParams.projParams_.y = cam.GetPerspectiveMatrix()[3][2];

        // see https://stackoverflow.com/questions/6652253/getting-the-true-z-value-from-the-depth-buffer
        // linear solution should be: -B / (z_n + A)
        // also https://developer.nvidia.com/gpugems/GPUGems/gpugems_ch23.html
        // and http://www.crytek.com/download/Sousa_Graphics_Gems_CryENGINE3.pdf

        // auto resY = static_cast<float>(targetFBO->GetHeight());
        auto resX = static_cast<float>(targetFBO->GetHeight());
        // auto aspect = resY / resX;
        // auto filmY = 0.035f * aspect;
        auto cocPixelFactor = resX / 0.035f;
        float F = (params_.imageDistance_ * params_.focusZ_) / (params_.imageDistance_ + params_.focusZ_);
        float A = F / params_.fStops_;
        float cocDiv = passParams.projParams_.y * (F - params_.focusZ_);
        float cocBias = A * F * ((params_.focusZ_ * passParams.projParams_.x) + passParams.projParams_.y);
        passParams.cocParams_.x = cocPixelFactor * (A * F * params_.focusZ_) / cocDiv; // coc scale
        passParams.cocParams_.y = cocPixelFactor * cocBias / cocDiv; // coc bias

        if (recalcBokeh_) RecalcBokeh();

        CoCPass(passParams);

        DownsamplePass(passParams);

        TileMinMaxPass(passParams, 0, 4); // TileX min max pass
        TileMinMaxPass(passParams, 1, 5); // TileY min max pass

        NearCoCBlurPass(passParams, 0, 6); // blur near x pass
        NearCoCBlurPass(passParams, 1, 5); // blur near y pass

        ComputeDoFPass(passParams);

        FillPass(passParams);

        CompositePass(passParams, targetFBO);
    }
}
