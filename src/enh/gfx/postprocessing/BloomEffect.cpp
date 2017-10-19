/**
 * @file   BloomEffect.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.12.22
 *
 * @brief  Implementation of the bloom effect.
 */

#include "BloomEffect.h"
#include "enh/ApplicationNodeBase.h"
#include "enh/gfx/gl/GLTexture.h"
#include <imgui.h>

namespace viscom::enh {

    BloomEffect::BloomEffect(const glm::ivec2 sourceSize, ApplicationNodeBase* app) :
        glareDetectProgram_(app->GetGPUProgramManager().GetResource("tm/glareDetect", std::vector<std::string>{"tm/glareDetect.cp"})),
        glareUniformIds_(glareDetectProgram_->GetUniformLocations({ "sourceTex", "targetTex", "exposure", "bloomThreshold" })),
        blurProgram_(app->GetGPUProgramManager().GetResource("tm/blurBloom", std::vector<std::string>{"tm/blurBloom.cp"})),
        blurUniformIds_(blurProgram_->GetUniformLocations({ "sourceTex", "targetTex", "dir", "bloomWidth" })),
        combineProgram_(app->GetGPUProgramManager().GetResource("tm/combineBloom_" + std::to_string(NUM_PASSES),
            std::vector<std::string>{"tm/combineBloom.cp"}, std::vector<std::string>{"NUM_PASSES " + std::to_string(NUM_PASSES)})),
        combineUniformIds_(combineProgram_->GetUniformLocations({ "sourceTex", "targetTex", "blurTex", "defocus", "bloomIntensity" })),
        sourceRTSize_(sourceSize)
    {
        params_.bloomThreshold_ = 0.63f;
        params_.bloomWidth_ = 1.0f;
        params_.defocus_ = 0.2f;
        params_.bloomIntensity_ = 1.0f;
        params_.exposure_ = 2.0f;

        Resize(sourceSize);
    }

    BloomEffect::~BloomEffect() = default;

    void BloomEffect::RenderParameterSliders()
    {
        if (ImGui::TreeNode("Bloom Parameters"))
        {
            ImGui::InputFloat("Bloom Threshold", &params_.bloomThreshold_, 0.01f);
            ImGui::InputFloat("Bloom Width", &params_.bloomWidth_, 0.1f);
            ImGui::InputFloat("Bloom Defocus", &params_.defocus_, 0.01f);
            ImGui::InputFloat("Bloom Intensity", &params_.bloomIntensity_, 0.1f);
            ImGui::TreePop();
        }
    }

    void BloomEffect::ApplyEffect(GLTexture* sourceRT, GLTexture* targetRT)
    {
        const glm::vec2 groupSize{ 32.0f, 16.0f };

        auto targetSize = glm::vec2(sourceRTSize_) / 2.0f;
        auto numGroups = glm::ivec2(glm::ceil(targetSize / groupSize));
        gl::glUseProgram(glareDetectProgram_->getProgramId());
        gl::glUniform1i(glareUniformIds_[0], 0);
        gl::glUniform1i(glareUniformIds_[1], 0);
        gl::glUniform1f(glareUniformIds_[2], params_.exposure_);
        gl::glUniform1f(glareUniformIds_[3], params_.bloomThreshold_);
        sourceRT->ActivateTexture(gl::GL_TEXTURE0);
        glaresRT_->ActivateImage(0, 0, gl::GL_WRITE_ONLY);
        gl::glDispatchCompute(numGroups.x, numGroups.y, 1);
        gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
        gl::glFinish();

        auto base = 1.0f;
        auto current = glaresRT_.get();
        for (auto& blurPassRTs : blurRTs_) {
            targetSize = glm::vec2(sourceRTSize_) / base;
            numGroups = glm::ivec2(glm::ceil(targetSize / groupSize));

            gl::glUseProgram(blurProgram_->getProgramId());
            gl::glUniform1i(blurUniformIds_[0], 0);
            gl::glUniform1i(blurUniformIds_[1], 0);
            gl::glUniform2f(blurUniformIds_[2], 1.0f, 0.0f);
            gl::glUniform1f(blurUniformIds_[3], params_.bloomWidth_);
            current->ActivateTexture(gl::GL_TEXTURE0);
            blurPassRTs[0]->ActivateImage(0, 0, gl::GL_WRITE_ONLY);
            gl::glDispatchCompute(numGroups.x, numGroups.y, 1);
            gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
            gl::glFinish();

            gl::glUniform2f(blurUniformIds_[2], 0.0f, 1.0f);
            blurPassRTs[0]->ActivateTexture(gl::GL_TEXTURE0);
            blurPassRTs[1]->ActivateImage(0, 0, gl::GL_WRITE_ONLY);
            gl::glDispatchCompute(numGroups.x, numGroups.y, 1);

            base *= 2.0f;
            current = blurPassRTs[1].get();
        }

        numGroups = glm::ivec2(glm::ceil(glm::vec2(sourceRTSize_) / groupSize));

        gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
        gl::glFinish();

        gl::glUseProgram(combineProgram_->getProgramId());
        gl::glUniform1i(combineUniformIds_[0], 0);
        gl::glUniform1i(combineUniformIds_[1], 0);
        gl::glUniform1iv(combineUniformIds_[2], static_cast<GLsizei>(blurTextureUnitIds_.size()), blurTextureUnitIds_.data());
        gl::glUniform1f(combineUniformIds_[3], params_.defocus_);
        gl::glUniform1f(combineUniformIds_[4], params_.bloomIntensity_);
        sourceRT->ActivateTexture(gl::GL_TEXTURE0);
        for (unsigned int i = 0; i < NUM_PASSES; ++i) {
            blurRTs_[i][1]->ActivateTexture(gl::GL_TEXTURE1 + i);
        }
        targetRT->ActivateImage(0, 0, gl::GL_WRITE_ONLY);
        gl::glDispatchCompute(numGroups.x, numGroups.y, 1);
        gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
        gl::glFinish();
    }

    void BloomEffect::Resize(const glm::uvec2& screenSize)
    {
        blurTextureUnitIds_.clear();
        sourceRTSize_ = screenSize;
        TextureDescriptor texDesc{ 16, gl::GL_RGBA32F, gl::GL_RGBA, gl::GL_FLOAT };
        glm::uvec2 size(screenSize.x / 2, screenSize.y / 2);
        glaresRT_ = std::make_unique<GLTexture>(size.x, size.y, texDesc, nullptr);

        unsigned int base = 1;
        auto blurTexUnit = 0;
        for (auto& blurPassRTs : blurRTs_) {
            blurTextureUnitIds_.emplace_back(++blurTexUnit);
            glm::uvec2 sizeBlurRT(glm::max(size.x / base, 1u), glm::max(size.y / base, 1u));
            blurPassRTs[0] = std::make_unique<GLTexture>(sizeBlurRT.x, sizeBlurRT.y, texDesc, nullptr);
            blurPassRTs[1] = std::make_unique<GLTexture>(sizeBlurRT.x, sizeBlurRT.y, texDesc, nullptr);
            base *= 2;
        }
    }
}
