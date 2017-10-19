/**
 * @file   DepthOfField.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.02.11
 *
 * @brief  Implementation of the depth of field effect.
 */

#include "DepthOfField.h"
#include "enh/ApplicationNodeBase.h"
#include "enh/gfx/gl/GLTexture.h"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace viscom::enh {

    DepthOfField::DepthOfField(const glm::ivec2 sourceSize, ApplicationNodeBase* app) :
        cocProgram_(app->GetGPUProgramManager().GetResource("dof/coc", std::vector<std::string>{ "dof/coc.cp" })),
        cocUniformIds_(cocProgram_->GetUniformLocations({ "colorTex", "depthTex", "targetTex", "focusZ", "scale", "clipInfo" })),
        combineProgram_(app->GetGPUProgramManager().GetResource("dof/combineDoF", std::vector<std::string>{ "dof/combineDoF.cp" })),
        combineUniformIds_(combineProgram_->GetUniformLocations({ "cocTex", "sourceFrontTex", "sourceBackTex", "targetTex" })),
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
        params_.apertureRadius_ = 0.001f;

        Resize(sourceSize);
    }

    DepthOfField::~DepthOfField() = default;

    void DepthOfField::RenderParameterSliders()
    {
        if (ImGui::TreeNode("DepthOfField Parameters"))
        {
            ImGui::InputFloat("DoF Focus", &params_.focusZ_, 0.005f);
            ImGui::InputFloat("Aperture Radius", &params_.apertureRadius_, 0.00001f);
            ImGui::TreePop();
        }
    }

    void DepthOfField::ApplyEffect(const CameraHelper& cam, const GLTexture* color, const GLTexture* depth, const GLTexture* targetRT)
    {
        const glm::vec2 groupSize{ 32.0f, 16.0f };

        auto targetSize = glm::vec2(sourceRTSize_);
        auto numGroups = glm::ivec2(glm::ceil(targetSize / groupSize));

        auto focalLength = CalculateFocalLength(cam) * targetSize.y;
        auto maxCoCRadius = CalculateMaxCoCRadius(cam);
        auto imaxCoCRadius = static_cast<int>(maxCoCRadius);
        auto nearBlurRadius = static_cast<int>(glm::ceil(glm::max(static_cast<float>(sourceRTSize_.y) / 100.0f, 12.0f)));
        auto invNearBlurRadius = 1.0f / glm::max(static_cast<float>(nearBlurRadius), 0.0001f);
        // const auto scale = (params.apertureRadius * focalLength) / (params.focusZ - focalLength);
        const auto scale = (params_.apertureRadius_ * focalLength) / (params_.focusZ_ * maxCoCRadius);
        glm::vec3 clipInfo(2.0f * cam.GetNearZ() * cam.GetFarZ(), cam.GetFarZ() - cam.GetNearZ(), cam.GetFarZ() + cam.GetNearZ());

        gl::glUseProgram(cocProgram_->getProgramId());
        gl::glUniform1i(cocUniformIds_[0], 0);
        gl::glUniform1i(cocUniformIds_[1], 1);
        gl::glUniform1i(cocUniformIds_[2], 0);
        gl::glUniform1f(cocUniformIds_[3], params_.focusZ_);
        gl::glUniform1f(cocUniformIds_[4], scale);
        gl::glUniform3fv(cocUniformIds_[5], 1, glm::value_ptr(clipInfo));
        color->ActivateTexture(gl::GL_TEXTURE0);
        depth->ActivateTexture(gl::GL_TEXTURE1);
        cocRT_->ActivateImage(0, 0, gl::GL_WRITE_ONLY);
        gl::glDispatchCompute(numGroups.x, numGroups.y, 1);
        gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
        gl::glFinish();

        gl::glUseProgram(hBlurProgram_->getProgramId());
        gl::glUniform1i(hBlurUniformIds_[0], 0);
        gl::glUniform1i(hBlurUniformIds_[1], 0);
        gl::glUniform1i(hBlurUniformIds_[2], 1);
        gl::glUniform1i(hBlurUniformIds_[3], imaxCoCRadius);
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
        gl::glUniform1i(vBlurUniformIds_[4], imaxCoCRadius);
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
        gl::glUniform1i(combineUniformIds_[0], 0);
        gl::glUniform1i(combineUniformIds_[1], 1);
        gl::glUniform1i(combineUniformIds_[2], 2);
        gl::glUniform1i(combineUniformIds_[3], 0);
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
        TextureDescriptor texDesc{ 16, gl::GL_RGBA32F, gl::GL_RGBA, gl::GL_FLOAT };
        glm::uvec2 size(screenSize.x, screenSize.y);
        cocRT_ = std::make_unique<viscom::enh::GLTexture>(size.x, size.y, texDesc, nullptr);

        blurRTs_[0][0] = std::make_unique<viscom::enh::GLTexture>(size.x / RT_SIZE_FACTOR, size.y, texDesc, nullptr);
        blurRTs_[0][1] = std::make_unique<viscom::enh::GLTexture>(size.x / RT_SIZE_FACTOR, size.y, texDesc, nullptr);
        blurRTs_[1][0] = std::make_unique<viscom::enh::GLTexture>(size.x / RT_SIZE_FACTOR, size.y / RT_SIZE_FACTOR, texDesc, nullptr);
        blurRTs_[1][1] = std::make_unique<viscom::enh::GLTexture>(size.x / RT_SIZE_FACTOR, size.y / RT_SIZE_FACTOR, texDesc, nullptr);
    }

    float DepthOfField::CalculateFocalLength(const CameraHelper& cam) const
    {
        const auto scale = 2.0f * glm::tan(cam.GetFOV() * 0.5f);
        return 1.0f / scale;
        // return static_cast<float>(sourceRTSize.y) / scale;
    }

    float DepthOfField::CalculateCoCRadius(const CameraHelper& cam, float z) const
    {
        // TODO: this returns negative values. [2/12/2016 Sebastian Maisch]
        auto focalLength = CalculateFocalLength(cam);
        // const float rzmeters = (z - params.focusZ) * params.apertureRadius / (-params.focusZ);
        // const float rimeters = rzmeters / z;
        auto resultMeters = (glm::abs(z - params_.focusZ_) * params_.apertureRadius_ * focalLength) / (z * (params_.focusZ_ - focalLength));
        return resultMeters;
    }

    float DepthOfField::CalculateMaxCoCRadius(const CameraHelper& cam) const
    {
        auto maxR = glm::max(CalculateCoCRadius(cam, cam.GetNearZ()), CalculateCoCRadius(cam, cam.GetFarZ()));
        return glm::ceil(glm::min(sourceRTSize_.y * maxR, sourceRTSize_.x * 0.02f));
    }
}
