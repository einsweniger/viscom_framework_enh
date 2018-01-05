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

    namespace dof {
        struct DoFPassParams;
    }

    struct DOFParams
    {
        float focusZ_;
        float imageDistance_;
        float fStops_;
        float fStopsMin_;
        float fStopsMax_;
        int bokehShape_;
        float rotateBokehMax_;

        template<class Archive> void serialize(Archive& ar, const std::uint32_t) {
            ar(cereal::make_nvp("focusZ", focusZ_),
                cereal::make_nvp("imageDistance", imageDistance_),
                cereal::make_nvp("fStops", fStops_),
                cereal::make_nvp("fStopsMin", fStopsMin_),
                cereal::make_nvp("fStopsMax", fStopsMax_),
                cereal::make_nvp("bokehShape", bokehShape_),
                cereal::make_nvp("rotateBokehMax", rotateBokehMax_));
        }
    };

    class DepthOfField
    {
    public:
        explicit DepthOfField(const glm::ivec2 sourceSize, ApplicationNodeBase* app);
        ~DepthOfField();

        void RenderParameterSliders();
        void ApplyEffect(const CameraHelper& cam, GLuint colorTex, GLuint depthTex, const GLTexture* targetRT);
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

        void RecalcBokeh();
        void CoCPass(const dof::DoFPassParams& passParams);
        void DownsamplePass(const dof::DoFPassParams& passParams);
        void TileMinMaxPass(std::size_t pass, std::size_t sourceTex);
        void NearCoCBlurPass(std::size_t pass, std::size_t sourceTex);
        void ComputeDoFPass();
        void FillPass();
        void CompositePass();

        // float CalculateFocalLength(const CameraHelper& cam) const;
        // float CalculateCoCRadius(const CameraHelper& cam, float z) const;
        // glm::vec2 CalculateMaxCoCRadius(const CameraHelper& cam) const;

        /** Holds the frame buffer with the full resolution render targets. */
        std::unique_ptr<FrameBuffer> fullResRT_;
        /** Holds the frame buffer with the low resolution render targets. */
        std::unique_ptr<FrameBuffer> lowResRT_;

        /** Holds the bloom parameters. */
        DOFParams params_;
        /** Holds the filter taps for the bokeh shape. */
        std::array<glm::vec2, 48> bokehTaps_;
        /** Holds whether the bokeh taps need recalculation. */
        bool recalcBokeh_ = true;

        /** Holds the quad for calculating the CoC. */
        FullscreenQuad cocQuad_;
        /** Holds the CoC program uniform ids. */
        std::vector<gl::GLint> cocUniformIds_;
        /** Holds the quad for down sampling the textures. */
        FullscreenQuad downsampleQuad_;
        /** Holds the down sampling program uniform ids. */
        std::vector<gl::GLint> downsampleUniformIds_;
        /** Holds the quad for calculating the tile horizontal and vertical min/max CoC. */
        std::array<FullscreenQuad, 2> tileMinMaxCoCQuad_;
        /** Holds the tile horizontal and vertical min/max CoC program uniform ids. */
        std::array<std::vector<gl::GLint>, 2> tileMinMaxCoCUniformIds_;
        /** Holds the quad for blurring the near field CoC. */
        std::array<FullscreenQuad, 2> nearCoCBlurQuad_;
        /** Holds the near CoC blur program uniform ids. */
        std::array<std::vector<gl::GLint>, 2> nearCoCBlurUniformIds_;
        /** Holds the quad for calculating DoF effect. */
        FullscreenQuad dofQuad_;
        /** Holds the computation program uniform ids. */
        std::vector<gl::GLint> dofUniformIds_;
        /** Holds the quad for calculating the CoC. */
        FullscreenQuad fillQuad_;
        /** Holds the fill program uniform ids. */
        std::vector<gl::GLint> fillUniformIds_;
        /** Holds the quad for combining near and far field again. */
        FullscreenQuad compositeQuad_;
        /** Holds the composite program uniform ids. */
        std::vector<gl::GLint> compositeUniformIds_;

        /** The draw buffers used in the down sample pass. */
        std::vector<std::size_t> downsamplePassDrawBuffers_;
        /** The draw buffers used in the tile passes. */
        std::array<std::vector<std::size_t>, 2> tilePassDrawBuffers_;
        /** The draw buffers used in the compute dof pass. */
        std::vector<std::size_t> dofPassDrawBuffers_;
        /** The draw buffers used in the fill dof pass. */
        std::vector<std::size_t> fillPassDrawBuffers_;
        /** The draw buffers used in the combine dof pass. */
        std::vector<std::size_t> combinePassDrawBuffers_;

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
