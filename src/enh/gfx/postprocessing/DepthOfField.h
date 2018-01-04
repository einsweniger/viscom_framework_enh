/**
 * @file   DepthOfField.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.02.11
 *
 * @brief  Declaration of the depth of field effect.
 */

#pragma once

#include "core/gfx/FullscreenQuad.h"
#include <array>
#include <cereal/access.hpp>
#include <cereal/cereal.hpp>
#include <glbinding/gl/gl.h>
#include <glm/vec2.hpp>
#include <memory>

namespace viscom {
    class GPUProgram;
    class CameraHelper;
    class FrameBuffer;
}

namespace viscom::enh {

    class ApplicationNodeBase;
    class GLTexture;

    struct DOFParams
    {
        float focusZ_;
        float apertureRadius_;
        float fStops_;

        template<class Archive> void serialize(Archive& ar, const std::uint32_t) {
            ar(cereal::make_nvp("focusZ", focusZ_),
                cereal::make_nvp("apertureRadius", apertureRadius_),
                cereal::make_nvp("fStops", fStops_));
        }
    };

    class DepthOfField
    {
    public:
        explicit DepthOfField(const glm::ivec2 sourceSize, ApplicationNodeBase* app);
        ~DepthOfField();

        void RenderParameterSliders();
        void ApplyEffect(const CameraHelper& cam, const GLuint color, const GLuint depth, const GLTexture* targetRT);
        void Resize(const glm::uvec2& screenSize);

        template<class Archive> void SaveParameters(Archive& ar, const std::uint32_t) const {
            ar(cereal::make_nvp("params", params_));
        }

        template<class Archive> void LoadParameters(Archive& ar, const std::uint32_t) {
            ar(cereal::make_nvp("params", params_));
        }

    private:
        static constexpr unsigned int RT_SIZE_FACTOR = 1;
        using FrontBackTargets = std::array<std::unique_ptr<GLTexture>, 2>;

        float CalculateFocalLength(const CameraHelper& cam) const;
        float CalculateCoCRadius(const CameraHelper& cam, float z) const;
        float CalculateMaxCoCRadius(const CameraHelper& cam) const;

        /** Holds the frame buffer with the full resolution render targets. */
        std::unique_ptr<FrameBuffer> fullResRT_;
        /** Holds the frame buffer with the low resolution render targets. */
        std::unique_ptr<FrameBuffer> lowResRT_;

        /** Holds the bloom parameters. */
        DOFParams params_;

        /** Holds the quad for calculating the CoC. */
        FullscreenQuad cocQuad_;
        /** Holds the CoC program uniform ids. */
        std::vector<gl::GLint> cocUniformIds_;
        /** Holds the quad for calculating the CoC. */
        FullscreenQuad downsampleQuad_;
        /** Holds the down sampling program uniform ids. */
        std::vector<gl::GLint> downsampleUniformIds_;
        /** Holds the quad for calculating the tile horizontal min/max CoC. */
        FullscreenQuad tileMinMaxXCoCQuad_;
        /** Holds the tile horizontal min/max CoC program uniform ids. */
        std::vector<gl::GLint> tileMinMaxXCoCUniformIds_;
        /** Holds the quad for calculating the tile vertical min/max CoC. */
        FullscreenQuad tileMinMaxYCoCQuad_;
        /** Holds the tile vertical min/max CoC program uniform ids. */
        std::vector<gl::GLint> tileMinMaxYCoCUniformIds_;

        /** Holds the quad for calculating the CoC. */
        FullscreenQuad nearCoCBlurQuad_;
        /** Holds the near CoC blur program uniform ids. */
        std::vector<gl::GLint> nearCoCBlurUniformIds_;
        /** Holds the quad for calculating the CoC. */
        FullscreenQuad computationQuad_;
        /** Holds the computation program uniform ids. */
        std::vector<gl::GLint> computationUniformIds_;
        /** Holds the quad for calculating the CoC. */
        FullscreenQuad fillQuad_;
        /** Holds the fill program uniform ids. */
        std::vector<gl::GLint> fillUniformIds_;
        /** Holds the quad for calculating the CoC. */
        FullscreenQuad combineQuad_;
        /** Holds the combine program uniform ids. */
        std::vector<gl::GLint> combineUniformIds_;

        /** The draw buffers used in the down sample pass. */
        std::vector<std::size_t> downsamplePassDrawBuffers_;
        /** The draw buffers used in the tileX pass. */
        std::vector<std::size_t> tileXPassDrawBuffers_;
        /** The draw buffers used in the tileY pass. */
        std::vector<std::size_t> tileYPassDrawBuffers_;

        /** Holds the render target for storing color information with circle of confusion. */
        std::unique_ptr<GLTexture> cocRT_;
        /** Holds the temporary results of the blurring. */
        std::array<FrontBackTargets, 2> blurRTs_;

        /** Holds the GPU program used for glare detection. */
        std::shared_ptr<GPUProgram> cocProgram_;
        /** Holds the glare program uniform ids. */
        // std::vector<gl::GLint> cocUniformIds_;
        /** Holds the GPU program used for blurring. */
        std::shared_ptr<GPUProgram> hBlurProgram_;
        /** Holds the blur program uniform ids. */
        std::vector<gl::GLint> hBlurUniformIds_;
        /** Holds the GPU program used for blurring. */
        std::shared_ptr<GPUProgram> vBlurProgram_;
        /** Holds the blur program uniform ids. */
        std::vector<gl::GLint> vBlurUniformIds_;
        /** Holds the GPU program used for combining the final image. */
        std::shared_ptr<GPUProgram> combineProgram_;
        /** Holds the combine program uniform ids. */
        // std::vector<gl::GLint> combineUniformIds_;

        /** Holds the size of the source textures. */
        glm::ivec2 sourceRTSize_;
    };
}

CEREAL_CLASS_VERSION(viscom::enh::DOFParams, 1)
