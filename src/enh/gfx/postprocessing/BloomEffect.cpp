/**
 * @file   BloomEffect.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.12.22
 *
 * @brief  Implementation of the bloom effect.
 */

#include "BloomEffect.h"
#include "core/gfx/FrameBuffer.h"
#include "enh/ApplicationNodeBase.h"
#include "enh/gfx/gl/GLTexture.h"
#include <imgui.h>

namespace viscom::enh {

    namespace bloom {
        struct BloomPassParams {
            GLuint colorTex_;
            const FrameBuffer* halfResRT_;
            const FrameBuffer* fourthResRT_;
        };
    }

    BloomEffect::BloomEffect(ApplicationNodeBase* app) :
        app_{ app },
        glareDetectQuad_("tm/glareDetect.frag", app),
        glareUniformIds_(glareDetectQuad_.GetGPUProgram()->GetUniformLocations({ "sourceTex" })),
        downsampleQuad_("tm/downsampleBloom.frag", app),
        downsampleUniformIds_(downsampleQuad_.GetGPUProgram()->GetUniformLocations({ "sourceTex" })),
        blurQuads_{ FullscreenQuad{ "tm/blurBloomX.frag", "tm/blurBloom.frag", std::vector<std::string>{ "HORIZONTAL" }, app },
            FullscreenQuad{ "tm/blurBloomY.frag", "tm/blurBloom.frag", std::vector<std::string>{ "VERTICAL" }, app } },
        blurUniformIds_{ blurQuads_[0].GetGPUProgram()->GetUniformLocations({ "sourceTex", "bloomWidth" }), blurQuads_[1].GetGPUProgram()->GetUniformLocations({ "sourceTex", "bloomWidth" }) },
        combineQuad_("tm/combineBloom.frag", app),
        combineUniformIds_(combineQuad_.GetGPUProgram()->GetUniformLocations({ "sourceTex", "blurTex", "bloomIntensity" }))
    {
        params_.bloomWidth_ = 1.0f;
        params_.bloomIntensity_ = 0.4f;

        Resize();
    }

    BloomEffect::~BloomEffect() = default;

    void BloomEffect::RenderParameterSliders()
    {
        if (ImGui::TreeNode("Bloom Parameters"))
        {
            ImGui::SliderFloat("Bloom Width", &params_.bloomWidth_, 0.2f, 1.8f);
            ImGui::InputFloat("Bloom Intensity", &params_.bloomIntensity_, 0.1f);
            ImGui::TreePop();
        }
    }

    void BloomEffect::ApplyEffect(GLuint sourceTex, const FrameBuffer* targetFBO, std::size_t drawBufferIndex)
    {
        bloom::BloomPassParams passParams;
        ApplyEffectInternal(passParams, sourceTex);

        targetFBO->DrawToFBO(std::vector<std::size_t>{drawBufferIndex}, [this, &passParams]() { CombinePass(passParams); });
    }

    void BloomEffect::ApplyEffect(GLuint sourceTex, const FrameBuffer* targetFBO)
    {
        bloom::BloomPassParams passParams;
        ApplyEffectInternal(passParams, sourceTex);

        targetFBO->DrawToFBO([this, &passParams]() { CombinePass(passParams); });
    }

    void BloomEffect::ApplyEffectInternal(bloom::BloomPassParams& passParams, GLuint sourceTex)
    {
        passParams.colorTex_ = sourceTex;
        passParams.halfResRT_ = app_->SelectOffscreenBuffer(glaresHalfRTs_);
        passParams.fourthResRT_ = app_->SelectOffscreenBuffer(glaresFourthRTs_);

        GlareDetectPass(passParams);
        DownsamplePass(passParams);

        // blur half
        BlurPass(passParams.halfResRT_, blurHalfPassDrawBuffers_, 0, 0);
        BlurPass(passParams.halfResRT_, blurHalfPassDrawBuffers_, 1, 1);

        // blur fourth
        BlurPass(passParams.fourthResRT_, blur1FourthPassDrawBuffers_, 0, 0);
        BlurPass(passParams.fourthResRT_, blur1FourthPassDrawBuffers_, 1, 2);

        BlurPass(passParams.fourthResRT_, blur2FourthPassDrawBuffers_, 0, 0);
        BlurPass(passParams.fourthResRT_, blur2FourthPassDrawBuffers_, 1, 2);
    }

    void BloomEffect::GlareDetectPass(const bloom::BloomPassParams& passParams)
    {
        passParams.halfResRT_->DrawToFBO(glarePassDrawBuffers_, [this, &passParams] {
            gl::glUseProgram(glareDetectQuad_.GetGPUProgram()->getProgramId());
            gl::glUniform1i(glareUniformIds_[0], 0);
            gl::glActiveTexture(gl::GL_TEXTURE0);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.colorTex_);
            glareDetectQuad_.Draw();
        });
    }

    void BloomEffect::DownsamplePass(const bloom::BloomPassParams& passParams)
    {
        passParams.fourthResRT_->DrawToFBO(dsPassDrawBuffers_, [this, &passParams] {
            gl::glUseProgram(downsampleQuad_.GetGPUProgram()->getProgramId());
            gl::glUniform1i(downsampleUniformIds_[0], 0);
            gl::glActiveTexture(gl::GL_TEXTURE0);
            gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.halfResRT_->GetTextures()[0]);
            downsampleQuad_.Draw();
        });
    }

    void BloomEffect::BlurPass(const FrameBuffer* fbo, const std::array<std::vector<std::size_t>, 2>& drawBuffers, std::size_t pass, std::size_t sourceTex)
    {
       fbo->DrawToFBO(drawBuffers[pass], [this, fbo, pass, sourceTex] {
            gl::glUseProgram(blurQuads_[pass].GetGPUProgram()->getProgramId());

            gl::glActiveTexture(gl::GL_TEXTURE0);
            gl::glBindTexture(gl::GL_TEXTURE_2D, fbo->GetTextures()[sourceTex]);

            gl::glUniform1i(blurUniformIds_[pass][0], 0);
            gl::glUniform1f(blurUniformIds_[pass][1], params_.bloomWidth_);
            blurQuads_[pass].Draw();
        });
    }

    void BloomEffect::CombinePass(const bloom::BloomPassParams& passParams)
    {
        gl::glUseProgram(combineQuad_.GetGPUProgram()->getProgramId());
        gl::glActiveTexture(gl::GL_TEXTURE0);
        gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.colorTex_);
        gl::glActiveTexture(gl::GL_TEXTURE0 + 1);
        gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.halfResRT_->GetTextures()[0]);
        gl::glActiveTexture(gl::GL_TEXTURE0 + 2);
        gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.fourthResRT_->GetTextures()[0]);
        gl::glActiveTexture(gl::GL_TEXTURE0 + 3);
        gl::glBindTexture(gl::GL_TEXTURE_2D, passParams.fourthResRT_->GetTextures()[1]);

        std::array<int, 3> blurTextureUnitIds{ 1, 2, 3 };
        gl::glUniform1i(combineUniformIds_[0], 0);
        gl::glUniform1iv(combineUniformIds_[1], static_cast<GLsizei>(blurTextureUnitIds.size()), blurTextureUnitIds.data());
        gl::glUniform1f(combineUniformIds_[2], params_.bloomIntensity_);

        combineQuad_.Draw();
    }

    void BloomEffect::Resize()
    {
        FrameBufferDescriptor glareDetectHalfRTDesc{ { 
                FrameBufferTextureDescriptor{ static_cast<GLenum>(gl::GL_RGBA32F) }, // 0: glare half, blur pong
                FrameBufferTextureDescriptor{ static_cast<GLenum>(gl::GL_RGBA32F) } // 0: glare half, blur ping
            },{} };
        glaresHalfRTs_ = app_->CreateOffscreenBuffers(glareDetectHalfRTDesc, 2);

        FrameBufferDescriptor glareDetectFourthRTDesc{ {
                FrameBufferTextureDescriptor{ static_cast<GLenum>(gl::GL_RGBA32F) }, // 0: glare fourth, blur pong
                FrameBufferTextureDescriptor{ static_cast<GLenum>(gl::GL_RGBA32F) }, // 0: glare fourth, blur 2 pong
                FrameBufferTextureDescriptor{ static_cast<GLenum>(gl::GL_RGBA32F) } // 0: glare fourth, blur ping
            },{} };
        glaresFourthRTs_ = app_->CreateOffscreenBuffers(glareDetectFourthRTDesc, 4);

        glarePassDrawBuffers_ = { 0 };
        blurHalfPassDrawBuffers_[0] = { 1 };
        blurHalfPassDrawBuffers_[1] = { 0 };
        dsPassDrawBuffers_ = { 0 };
        blur1FourthPassDrawBuffers_[0] = { 2 };
        blur1FourthPassDrawBuffers_[1] = { 0 };
        blur2FourthPassDrawBuffers_[0] = { 2 };
        blur2FourthPassDrawBuffers_[1] = { 1 };
    }

}
