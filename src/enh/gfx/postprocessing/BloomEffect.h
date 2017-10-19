/**
 * @file   BloomEffect.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.12.22
 *
 * @brief  Declaration of the bloom effect.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glbinding/gl/gl.h>
#include <cereal/cereal.hpp>
#include <cereal/access.hpp>

namespace viscom {

    class GPUProgram;
}

namespace viscom::enh {

    class ApplicationNodeBase;
    class GLTexture;

    struct BloomParams
    {
        float exposure_;
        float bloomThreshold_;
        float bloomWidth_;
        float defocus_;
        float bloomIntensity_;

        template<class Archive> void serialize(Archive& ar, const std::uint32_t) {
            ar(cereal::make_nvp("exposure", exposure_),
                cereal::make_nvp("bloomThreshold", bloomThreshold_),
                cereal::make_nvp("bloomWidth", bloomWidth_),
                cereal::make_nvp("defocus", defocus_),
                cereal::make_nvp("bloomIntensity", bloomIntensity_));
        }
    };

    class BloomEffect
    {
    public:
        explicit BloomEffect(const glm::ivec2 sourceSize, ApplicationNodeBase* app);
        ~BloomEffect();

        void RenderParameterSliders();
        void ApplyEffect(GLTexture* sourceRT, GLTexture* targetRT);
        void Resize(const glm::uvec2& screenSize);

        void SetExposure(float exposure) { params_.exposure_ = exposure; }
        float GetExposure() const { return params_.exposure_; }

        template<class Archive> void SaveParameters(Archive& ar, const std::uint32_t) const {
            ar(cereal::make_nvp("params", params_));
        }

        template<class Archive> void LoadParameters(Archive& ar, const std::uint32_t) {
            ar(cereal::make_nvp("params", params_));
        }

    private:
        using BlurPassTargets = std::array<std::unique_ptr<GLTexture>, 2>;
        static constexpr unsigned int NUM_PASSES = 6;

        /** Holds the render target for HDR rendering. */
        std::unique_ptr<GLTexture> glaresRT_;
        /** Holds the temporary results of the blurring. */
        std::array<BlurPassTargets, NUM_PASSES> blurRTs_;
        /** Holds the bloom parameters. */
        BloomParams params_;

        /** Holds the GPU program used for glare detection. */
        std::shared_ptr<GPUProgram> glareDetectProgram_;
        /** Holds the glare program uniform ids. */
        std::vector<gl::GLint> glareUniformIds_;
        /** Holds the GPU program used for blurring. */
        std::shared_ptr<GPUProgram> blurProgram_;
        /** Holds the blur program uniform ids. */
        std::vector<gl::GLint> blurUniformIds_;
        /** Holds the GPU program used for combining the final image. */
        std::shared_ptr<GPUProgram> combineProgram_;
        /** Holds the combine program uniform ids. */
        std::vector<gl::GLint> combineUniformIds_;
        /** Holds the number of compute groups. */
        glm::ivec2 sourceRTSize_;
        /** Holds the texture unit ids to bind the blur textures to. */
        std::vector<int> blurTextureUnitIds_;
    };
}

CEREAL_CLASS_VERSION(viscom::enh::BloomParams, 1)
