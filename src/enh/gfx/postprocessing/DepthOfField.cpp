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

    DepthOfField::DepthOfField(const glm::ivec2 sourceSize, ApplicationNodeBase* app) :
        cocQuad_{ "dof/coc.frag", app },
        cocUniformIds_{ cocQuad_.GetGPUProgram()->GetUniformLocations({ "depthTex", "cocParams" }) },
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
        compositeUniformIds_{ compositeQuad_.GetGPUProgram()->GetUniformLocations({ "colorTex", "cocFullTex", "cocSmallTex", "cocTexNearBlurred", "nearFieldFilled", "farFieldFilled", "..." }) },
        cocProgram_(app->GetGPUProgramManager().GetResource("dof/coc", std::vector<std::string>{ "dof/coc.cp" })),
        combineProgram_(app->GetGPUProgramManager().GetResource("dof/combineDoF", std::vector<std::string>{ "dof/combineDoF.cp" })),
        // combineUniformIds_(combineProgram_->GetUniformLocations({ "cocTex", "sourceFrontTex", "sourceBackTex", "targetTex" })),
        hBlurProgram_(app->GetGPUProgramManager().GetResource("dof/blurDoF_H_" + std::to_string(RT_SIZE_FACTOR),
            std::vector<std::string>{ "dof/blurDoF.cp" },
            std::vector<std::string>{ "HORIZONTAL", "SIZE_FACTOR " + std::to_string(RT_SIZE_FACTOR) })),
        hBlurUniformIds_(combineProgram_->GetUniformLocations({ "sourceTex", "targetFrontTex", "targetBackTex", "maxCoCRadius", "frontBlurRadius", "invFrontBlurRadius" })),
        vBlurProgram_(app->GetGPUProgramManager().GetResource("dof/blurDoF_" + std::to_string(RT_SIZE_FACTOR),
            std::vector<std::string>{ "dof/blurDoF.cp" },
            std::vector<std::string>{ "SIZE_FACTOR " + std::to_string(RT_SIZE_FACTOR) })),
        vBlurUniformIds_(combineProgram_->GetUniformLocations({ "sourceFrontTex", "sourceTex", "targetFrontTex", "targetBackTex", "maxCoCRadius", "frontBlurRadius", "invFrontBlurRadius" })),
        sourceRTSize_(sourceSize)
    {
        params_.focusZ_ = 2.3f;
        params_.imageDistance_ = 0.05f;
        params_.fStops_ = 2.2f;
        params_.fStopsMin_ = 1.6f;
        params_.fStopsMax_ = 5.6f;
        params_.bokehShape_ = 7;
        params_.rotateBokehMax_ = glm::pi<float>() / 3.0f;

        FrameBufferDescriptor fullResRTDesc{ { FrameBufferTextureDescriptor{ static_cast<GLenum>(gl::GL_RGB32F) } }, {} }; // CoC near/far/depth
        fullResRT_ = std::make_unique<FrameBuffer>(sourceSize.x, sourceSize.y, fullResRTDesc);

        FrameBufferDescriptor lowResRTDesc{ {
                FrameBufferTextureDescriptor{ static_cast<GLenum>(gl::GL_RGB32F) }, // 0: color / nearFieldFill
                FrameBufferTextureDescriptor{ static_cast<GLenum>(gl::GL_RGB32F) }, // 1: colorMulCoCFar / farFieldFilled
                FrameBufferTextureDescriptor{ static_cast<GLenum>(gl::GL_RGB32F) }, // 2: nearField
                FrameBufferTextureDescriptor{ static_cast<GLenum>(gl::GL_RGB32F) }, // 3: farField
                FrameBufferTextureDescriptor{ static_cast<GLenum>(gl::GL_RG32F) }, // 4: CoC near/far
                FrameBufferTextureDescriptor{ static_cast<GLenum>(gl::GL_RG32F) }, // 5: CoC near/far ping
                FrameBufferTextureDescriptor{ static_cast<GLenum>(gl::GL_RG32F) } // 6: CoC near/far pong <- final
            }, {} };
        lowResRT_ = std::make_unique<FrameBuffer>(sourceSize.x / 2, sourceSize.y / 2, lowResRTDesc);
        // Resize(sourceSize);

        downsamplePassDrawBuffers_ = { 0, 1, 4 };
        tilePassDrawBuffers_[0] = { 5 };
        tilePassDrawBuffers_[1] = { 6 };
        dofPassDrawBuffers_ = { 2, 3 };
        fillPassDrawBuffers_ = { 0, 1 };
        combinePassDrawBuffers_ = { };

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

        // TODO: texture (static?) in class .. maybe even in application?
        // remove bi-cubic code from composite shader to header file.
        TextureDescriptor texDesc{ 12, gl::GL_RGB32F, gl::GL_RGB, gl::GL_FLOAT };
        GLTexture hgTex{ 256, texDesc };
        hgTex.SetData(hg_precalc.data());
        hgTex.SampleLinear();
        hgTex.SampleWrapRepeat();
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
        fullResRT_->DrawToFBO([this, &passParams]() {
            gl::glUseProgram(cocQuad_.GetGPUProgram()->getProgramId());

            gl::glActiveTexture(gl::GL_TEXTURE0);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.depthTex_);

            gl::glUniform1i(cocUniformIds_[0], 0);
            gl::glUniform2fv(cocUniformIds_[1], 1, glm::value_ptr(passParams.cocParams_));
            cocQuad_.Draw();
        });
    }

    void DepthOfField::DownsamplePass(const dof::DoFPassParams& passParams)
    {
        lowResRT_->DrawToFBO(downsamplePassDrawBuffers_, [this, &passParams]() {
            gl::glUseProgram(downsampleQuad_.GetGPUProgram()->getProgramId());

            gl::glActiveTexture(gl::GL_TEXTURE0);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.colorTex_);
            gl::glActiveTexture(gl::GL_TEXTURE1);
            gl::glBindTexture(gl::GL_TEXTURE_2D, fullResRT_->GetTextures()[0]);

            gl::glUniform1i(downsampleUniformIds_[0], 0);
            gl::glUniform1i(downsampleUniformIds_[1], 1);
            downsampleQuad_.Draw();
        });
    }

    void DepthOfField::TileMinMaxPass(std::size_t pass, std::size_t sourceTex)
    {
        lowResRT_->DrawToFBO(tilePassDrawBuffers_[pass], [this, pass, sourceTex]() {
            gl::glUseProgram(tileMinMaxCoCQuad_[pass].GetGPUProgram()->getProgramId());

            gl::glActiveTexture(gl::GL_TEXTURE0);
            gl::glBindTexture(gl::GL_TEXTURE_2D, lowResRT_->GetTextures()[sourceTex]);

            gl::glUniform1i(tileMinMaxCoCUniformIds_[pass][0], 0);
            tileMinMaxCoCQuad_[pass].Draw();
        });
    }

    void DepthOfField::NearCoCBlurPass(std::size_t pass, std::size_t sourceTex)
    {
        lowResRT_->DrawToFBO(tilePassDrawBuffers_[pass], [this, pass, sourceTex]() {
            gl::glUseProgram(nearCoCBlurQuad_[pass].GetGPUProgram()->getProgramId());

            gl::glActiveTexture(gl::GL_TEXTURE0);
            gl::glBindTexture(gl::GL_TEXTURE_2D, lowResRT_->GetTextures()[sourceTex]);

            gl::glUniform1i(nearCoCBlurUniformIds_[pass][0], 0);
            nearCoCBlurQuad_[pass].Draw();
        });
    }

    void DepthOfField::ComputeDoFPass()
    {
        lowResRT_->DrawToFBO(dofPassDrawBuffers_, [this]() {
            gl::glUseProgram(dofQuad_.GetGPUProgram()->getProgramId());

            gl::glActiveTexture(gl::GL_TEXTURE0 + 0);
            gl::glBindTexture(gl::GL_TEXTURE_2D, lowResRT_->GetTextures()[4]);
            gl::glActiveTexture(gl::GL_TEXTURE0 + 1);
            gl::glBindTexture(gl::GL_TEXTURE_2D, lowResRT_->GetTextures()[6]);
            gl::glActiveTexture(gl::GL_TEXTURE0 + 2);
            gl::glBindTexture(gl::GL_TEXTURE_2D, lowResRT_->GetTextures()[0]);
            gl::glActiveTexture(gl::GL_TEXTURE0 + 3);
            gl::glBindTexture(gl::GL_TEXTURE_2D, lowResRT_->GetTextures()[1]);

            gl::glUniform1i(dofUniformIds_[0], 0);
            gl::glUniform1i(dofUniformIds_[1], 1);
            gl::glUniform1i(dofUniformIds_[2], 2);
            gl::glUniform1i(dofUniformIds_[3], 3);
            gl::glUniform2fv(dofUniformIds_[4], static_cast<gl::GLsizei>(bokehTaps_.size()), reinterpret_cast<float*>(bokehTaps_.data()));
            dofQuad_.Draw();
        });
    }

    void DepthOfField::FillPass()
    {
        lowResRT_->DrawToFBO(fillPassDrawBuffers_, [this]() {
            gl::glUseProgram(fillQuad_.GetGPUProgram()->getProgramId());

            gl::glActiveTexture(gl::GL_TEXTURE0 + 0);
            gl::glBindTexture(gl::GL_TEXTURE_2D, lowResRT_->GetTextures()[4]);
            gl::glActiveTexture(gl::GL_TEXTURE0 + 1);
            gl::glBindTexture(gl::GL_TEXTURE_2D, lowResRT_->GetTextures()[6]);
            gl::glActiveTexture(gl::GL_TEXTURE0 + 2);
            gl::glBindTexture(gl::GL_TEXTURE_2D, lowResRT_->GetTextures()[2]);
            gl::glActiveTexture(gl::GL_TEXTURE0 + 3);
            gl::glBindTexture(gl::GL_TEXTURE_2D, lowResRT_->GetTextures()[3]);

            gl::glUniform1i(fillUniformIds_[0], 0);
            gl::glUniform1i(fillUniformIds_[1], 1);
            gl::glUniform1i(fillUniformIds_[2], 2);
            gl::glUniform1i(fillUniformIds_[3], 3);
            fillQuad_.Draw();
        });
    }

    void DepthOfField::CompositePass()
    {
    }

    void DepthOfField::ApplyEffect(const CameraHelper& cam, GLuint colorTex, GLuint depthTex, const GLTexture* targetRT)
    {
        dof::DoFPassParams passParams;
        passParams.colorTex_ = colorTex;
        passParams.depthTex_ = depthTex;

        float F = (params_.imageDistance_ * params_.focusZ_) / (params_.imageDistance_ + params_.focusZ_);
        float A = F / params_.fStops_;
        float cocDiv = cam.GetPerspectiveMatrix()[3][2] * (F - params_.focusZ_);
        float cocBias = A * F * ((params_.focusZ_ * cam.GetPerspectiveMatrix()[2][2]) + cam.GetPerspectiveMatrix()[3][2]);
        passParams.cocParams_.x = (A * F * params_.focusZ_) / cocDiv; // coc scale
        passParams.cocParams_.y = cocBias / cocDiv; // coc bias

        if (recalcBokeh_) RecalcBokeh();

        CoCPass(passParams);

        DownsamplePass(passParams);

        TileMinMaxPass(0, 4); // TileX min max pass
        TileMinMaxPass(1, 5); // TileY min max pass

        NearCoCBlurPass(0, 6); // blur near x pass
        NearCoCBlurPass(1, 5); // blur near y pass

        ComputeDoFPass();

        FillPass();

        // combine pass

        const glm::vec2 groupSize{ 32.0f, 16.0f };

        auto targetSize = glm::vec2(sourceRTSize_);
        auto numGroups = glm::ivec2(glm::ceil(targetSize / groupSize));

        // auto focalLength = CalculateFocalLength(cam) * targetSize.y;
        // auto maxCoCRadius = CalculateMaxCoCRadius(cam);
        // auto imaxCoCRadius = static_cast<int>(maxCoCRadius);
        auto nearBlurRadius = static_cast<int>(glm::ceil(glm::max(static_cast<float>(sourceRTSize_.y) / 100.0f, 12.0f)));
        auto invNearBlurRadius = 1.0f / glm::max(static_cast<float>(nearBlurRadius), 0.0001f);
        // const auto scale = (params.apertureRadius * focalLength) / (params.focusZ - focalLength);
        // const auto scale = (params_.apertureRadius_ * focalLength) / (params_.focusZ_ * maxCoCRadius);

        // see https://stackoverflow.com/questions/6652253/getting-the-true-z-value-from-the-depth-buffer
        // linear solution should be: -B / (z_n + A)
        glm::vec3 clipInfo(2.0f * cam.GetNearPlane() * cam.GetFarPlane(), cam.GetFarPlane() - cam.GetNearPlane(), cam.GetFarPlane() + cam.GetNearPlane());

        gl::glUseProgram(cocProgram_->getProgramId());
        gl::glUniform1i(cocUniformIds_[0], 0);
        gl::glUniform1i(cocUniformIds_[1], 1);
        gl::glUniform1i(cocUniformIds_[2], 0);
        gl::glUniform1f(cocUniformIds_[3], params_.focusZ_);
        // gl::glUniform1f(cocUniformIds_[4], scale);
        gl::glUniform3fv(cocUniformIds_[5], 1, glm::value_ptr(clipInfo));
        // color->ActivateTexture(gl::GL_TEXTURE0);
        // depth->ActivateTexture(gl::GL_TEXTURE1);
        cocRT_->ActivateImage(0, 0, gl::GL_WRITE_ONLY);
        gl::glDispatchCompute(numGroups.x, numGroups.y, 1);
        gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
        gl::glFinish();

        gl::glUseProgram(hBlurProgram_->getProgramId());
        gl::glUniform1i(hBlurUniformIds_[0], 0);
        gl::glUniform1i(hBlurUniformIds_[1], 0);
        gl::glUniform1i(hBlurUniformIds_[2], 1);
        // gl::glUniform1i(hBlurUniformIds_[3], imaxCoCRadius);
        gl::glUniform1i(hBlurUniformIds_[4], nearBlurRadius);
        gl::glUniform1f(hBlurUniformIds_[5], invNearBlurRadius);
        cocRT_->ActivateTexture(gl::GL_TEXTURE0);
        blurRTs_[0][0]->ActivateImage(0, 0, gl::GL_WRITE_ONLY);;
        blurRTs_[0][1]->ActivateImage(1, 0, gl::GL_WRITE_ONLY);
        gl::glDispatchCompute(numGroups.x / RT_SIZE_FACTOR, numGroups.y, 1);
        gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
        gl::glFinish();

        gl::glUseProgram(vBlurProgram_->getProgramId());
        gl::glUniform1i(vBlurUniformIds_[0], 0);
        gl::glUniform1i(vBlurUniformIds_[1], 1);
        gl::glUniform1i(vBlurUniformIds_[2], 0);
        gl::glUniform1i(vBlurUniformIds_[3], 1);
        // gl::glUniform1i(vBlurUniformIds_[4], imaxCoCRadius);
        gl::glUniform1i(vBlurUniformIds_[5], nearBlurRadius);
        gl::glUniform1f(vBlurUniformIds_[6], invNearBlurRadius);
        blurRTs_[0][0]->ActivateTexture(gl::GL_TEXTURE0);
        blurRTs_[0][1]->ActivateTexture(gl::GL_TEXTURE1);
        blurRTs_[1][0]->ActivateImage(0, 0, gl::GL_WRITE_ONLY);
        blurRTs_[1][1]->ActivateImage(1, 0, gl::GL_WRITE_ONLY);
        gl::glDispatchCompute(numGroups.x / RT_SIZE_FACTOR, numGroups.y / RT_SIZE_FACTOR, 1);
        gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
        gl::glFinish();

        gl::glUseProgram(combineProgram_->getProgramId());
        // gl::glUniform1i(combineUniformIds_[0], 0); //-V525
        // gl::glUniform1i(combineUniformIds_[1], 1);
        // gl::glUniform1i(combineUniformIds_[2], 2);
        // gl::glUniform1i(combineUniformIds_[3], 0);
        /*combineProgram->SetUniform(combineUniformIds[2], blurTextureUnitIds);
        combineProgram->SetUniform(combineUniformIds[3], params.defocus);
        combineProgram->SetUniform(combineUniformIds[4], params.bloomIntensity);
        sourceRT->GetTextures()[0]->ActivateTexture(GL_TEXTURE0);
        for (unsigned int i = 0; i < NUM_PASSES; ++i) {
            blurRTs[i][1]->ActivateTexture(GL_TEXTURE1 + i);
        }*/
        cocRT_->ActivateTexture(gl::GL_TEXTURE0);
        blurRTs_[1][0]->ActivateTexture(gl::GL_TEXTURE1);
        blurRTs_[1][1]->ActivateTexture(gl::GL_TEXTURE2);
        targetRT->ActivateImage(0, 0, gl::GL_WRITE_ONLY);
        gl::glDispatchCompute(numGroups.x, numGroups.y, 1);
        gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
        gl::glFinish();
    }

    void DepthOfField::Resize(const glm::uvec2& screenSize)
    {
        sourceRTSize_ = screenSize;

        fullResRT_->Resize(sourceRTSize_.x, sourceRTSize_.y);
        lowResRT_->Resize(sourceRTSize_.x / 2, sourceRTSize_.y / 2);


        TextureDescriptor texDesc{ 12, gl::GL_RGB32F, gl::GL_RGB, gl::GL_FLOAT };
        // glm::uvec2 size(screenSize.x, screenSize.y);
        cocRT_ = std::make_unique<viscom::enh::GLTexture>(screenSize.x, screenSize.y, texDesc, nullptr);

        blurRTs_[0][0] = std::make_unique<viscom::enh::GLTexture>(screenSize.x / RT_SIZE_FACTOR, screenSize.y, texDesc, nullptr);
        blurRTs_[0][1] = std::make_unique<viscom::enh::GLTexture>(screenSize.x / RT_SIZE_FACTOR, screenSize.y, texDesc, nullptr);
        blurRTs_[1][0] = std::make_unique<viscom::enh::GLTexture>(screenSize.x / RT_SIZE_FACTOR, screenSize.y / RT_SIZE_FACTOR, texDesc, nullptr);
        blurRTs_[1][1] = std::make_unique<viscom::enh::GLTexture>(screenSize.x / RT_SIZE_FACTOR, screenSize.y / RT_SIZE_FACTOR, texDesc, nullptr);
    }

    // float DepthOfField::CalculateFocalLength(const CameraHelper& cam) const
    // {
    //     const auto scale = 2.0f;// *glm::tan(cam.GetFOV() * 0.5f);
    //     return 1.0f / scale;
    //     // return static_cast<float>(sourceRTSize.y) / scale;
    // }

    /**
     *  Calculates the circle of diffusion (CoC) based on current parameters and camera.
     *  @param cam the camera
     *  @param z the depth value to calculate the CoC for.
     *  @return the CoC (positive value for near field, negative value for far field)
     */
    // float DepthOfField::CalculateCoCRadius(const CameraHelper& cam, float z) const
    // {
    //     float focalLength = params_.fStops_ * 2.0f * params_.apertureRadius_;
    //     float coc = params_.focusZ_ - z;
    //     coc *= focalLength * 2.0f * params_.apertureRadius_;
    //     return coc / (z * (params_.focusZ_ - focalLength));
    // }

    // glm::vec2 DepthOfField::CalculateMaxCoCRadius(const CameraHelper& cam) const
    // {
    //     float cocNear = CalculateCoCRadius(cam, cam.GetNearPlane());
    //     float cocFar = CalculateCoCRadius(cam, cam.GetFarPlane());
    //     assert(cocNear >= 0.0f);
    //     assert(cocFar <= 0.0f);
    //     return glm::vec2(cocNear, cocFar);
    // }
}
