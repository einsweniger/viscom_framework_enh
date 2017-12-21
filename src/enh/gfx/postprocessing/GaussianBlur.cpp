/**
 * @file   GaussianBlur.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.05.05
 *
 * @brief  Implementation of a Gaussian blur filter.
 */

#define GLM_FORCE_SWIZZLE
#include "GaussianBlur.h"
#include "enh/ApplicationNodeBase.h"
#include "enh/gfx/gl/GLTexture.h"

namespace viscom::enh {

    GaussianBlur::GaussianBlur(const GLTexture* source, const std::string& texFormat, const std::string& blurColorType, const std::string& blurSwizzle, ApplicationNodeBase* app) :
        source_{ source },
        size_{ 0 },
        gaussianProgram_(app->GetGPUProgramManager().GetResource("gaussianFilter_" + texFormat + "_" + blurColorType + "_" + blurSwizzle,
            std::initializer_list<std::string>{"gaussianFilter.cp"},
            std::vector<std::string>{"TEX_FORMAT " + texFormat, "BTYPE " + blurColorType, "COMP_SWIZZLE " + blurSwizzle})),
        gaussianUniformIds_(gaussianProgram_->GetUniformLocations({ "sourceTex", "targetTex", "dir", "bloomWidth" }))
    {
        Resize();
    }


    GaussianBlur::~GaussianBlur() = default;

    void GaussianBlur::UpdateResizedTarget(const GLTexture* source)
    {
        source_ = source;
    }

    void GaussianBlur::ApplyBlur(float width)
    {
        const glm::vec2 groupSize{ 32.0f, 16.0f };
        auto numGroups = glm::ivec2(glm::ceil(glm::vec2(size_) / groupSize));

        gl::glUseProgram(gaussianProgram_->getProgramId());
        gl::glUniform2f(gaussianUniformIds_[2], 1.0f, 0.0f);
        gl::glUniform1f(gaussianUniformIds_[3], 1.0f);

        /*for (auto i = 0; i < blurredShadowMap_.size(); ++i) {
            shadowMapRT_->GetTextures()[i]->ActivateTexture(GL_TEXTURE0 + i);
            blurredShadowMap_[i]->ActivateImage(i, 0, GL_WRITE_ONLY);
            textureStages.push_back(i);
        }
        gaussianProgram_->SetUniform(gaussianUniformIds_[0], textureStages);
        gaussianProgram_->SetUniform(gaussianUniformIds_[1], textureStages);*/
        source_->ActivateTexture(gl::GL_TEXTURE0 + 0);
        tmp_->ActivateImage(0, 0, gl::GL_WRITE_ONLY);
        gl::glUniform1i(gaussianUniformIds_[0], 0);
        gl::glUniform1i(gaussianUniformIds_[1], 0);
        gl::glDispatchCompute(numGroups.x, numGroups.y, 1);
        gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
        gl::glFinish();

        gl::glUniform2f(gaussianUniformIds_[2], 0.0f, 1.0f);
        /*for (auto i = 0; i < blurredShadowMap_.size(); ++i) {
            shadowMapRT_->GetTextures()[i]->ActivateImage(i, 0, GL_WRITE_ONLY);
            blurredShadowMap_[i]->ActivateTexture(GL_TEXTURE0 + i);
            }*/
        source_->ActivateImage(0, 0, gl::GL_WRITE_ONLY);
        tmp_->ActivateTexture(gl::GL_TEXTURE0 + 0);
        gl::glDispatchCompute(numGroups.x, numGroups.y, 1);
        gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
        gl::glFinish();
    }

    void GaussianBlur::Resize()
    {
        auto texDim = source_->GetDimensions();
        size_.x = texDim.x;
        size_.y = texDim.y;
        tmp_ = std::make_unique<GLTexture>(texDim.x, texDim.y, source_->GetDescriptor(), nullptr);
    }
}
