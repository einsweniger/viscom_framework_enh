/**
 * @file   BloomEffect.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.12.22
 *
 * @brief  Declaration of the bloom effect.
 */

#pragma once

#include "core/gfx/FullscreenQuad.h"
#include <memory>
#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glbinding/gl/gl.h>
#include <cereal/cereal.hpp>
#include <cereal/access.hpp>

namespace viscom {
    class GPUProgram;
    class FrameBuffer;
}

namespace viscom::enh {

    class ApplicationNodeBase;
    class GLTexture;


    namespace bloom {
        struct BloomPassParams;
    }

    struct BloomParams
    {
        float bloomWidth_;
        float bloomIntensity_;

        template<class Archive> void serialize(Archive& ar, const std::uint32_t) {
            ar(cereal::make_nvp("bloomWidth", bloomWidth_),
                cereal::make_nvp("bloomIntensity", bloomIntensity_));
        }
    };

    class BloomEffect
    {
    public:
        explicit BloomEffect(ApplicationNodeBase* app);
        ~BloomEffect();

        void RenderParameterSliders();
        void ApplyEffect(GLuint sourceTex, const FrameBuffer* targetFBO, std::size_t drawBufferIndex);
        void ApplyEffect(GLuint sourceTex, const FrameBuffer* targetFBO);
        void Resize();

        template<class Archive> void SaveParameters(Archive& ar, const std::uint32_t) const {
            ar(cereal::make_nvp("params", params_));
        }

        template<class Archive> void LoadParameters(Archive& ar, const std::uint32_t) {
            ar(cereal::make_nvp("params", params_));
        }

    private:
        void ApplyEffectInternal(bloom::BloomPassParams& passParams, GLuint sourceTex);
        void GlareDetectPass(const bloom::BloomPassParams& passParams);
        void DownsamplePass(const bloom::BloomPassParams& passParams);
        void BlurPass(const FrameBuffer* fbo, const std::array<std::vector<std::size_t>, 2>& drawBuffers, std::size_t pass, std::size_t sourceTex);
        void CombinePass(const bloom::BloomPassParams& passParams);

        /** Holds the base application object. */
        ApplicationNodeBase* app_;

        /** Holds the frame buffers for glare detection. */
        std::vector<FrameBuffer> glaresHalfRTs_;
        /** Holds the frame buffers for glare detection. */
        std::vector<FrameBuffer> glaresFourthRTs_;

        /** Holds the bloom parameters. */
        BloomParams params_;

        /** Holds the full screen quad used for glare detection. */
        FullscreenQuad glareDetectQuad_;
        /** Holds the glare program uniform ids. */
        std::vector<gl::GLint> glareUniformIds_;
        /** Holds the full screen quad used for down sampling. */
        FullscreenQuad downsampleQuad_;
        /** Holds the down sampling program uniform ids. */
        std::vector<gl::GLint> downsampleUniformIds_;
        /** Holds the full screen quad used for blurring. */
        std::array<FullscreenQuad, 2> blurQuads_;
        /** Holds the blur program uniform ids. */
        std::array<std::vector<gl::GLint>, 2> blurUniformIds_;
        /** Holds the full screen quad used for combining. */
        FullscreenQuad combineQuad_;
        /** Holds the combining program uniform ids. */
        std::vector<gl::GLint> combineUniformIds_;

        /** The draw buffers used in the glare pass. */
        std::vector<std::size_t> glarePassDrawBuffers_;
        /** The draw buffers used in the blur half resolution pass. */
        std::array<std::vector<std::size_t>, 2> blurHalfPassDrawBuffers_;
        /** The draw buffers used in the down sample pass. */
        std::vector<std::size_t> dsPassDrawBuffers_;
        /** The draw buffers used in the first blur fourth resolution pass. */
        std::array<std::vector<std::size_t>, 2> blur1FourthPassDrawBuffers_;
        /** The draw buffers used in the second blur fourth resolution pass. */
        std::array<std::vector<std::size_t>, 2> blur2FourthPassDrawBuffers_;
    };
}

CEREAL_CLASS_VERSION(viscom::enh::BloomParams, 1)
