/**
 * @file   FilmicTMOperator.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.12.21
 *
 * @brief  Declaration of the filmic tone-mapping operator class.
 */

#pragma once

#include "core/gfx/FullscreenQuad.h"
#include <memory>
#include <vector>
#include <glm/vec2.hpp>
#include <glbinding/gl/gl.h>
#include <cereal/cereal.hpp>
#include <cereal/access.hpp>

namespace viscom {
    class GPUProgram;
    class FullscreenQuad;
    class FrameBuffer;
}

namespace viscom::enh {

    class ApplicationNodeBase;
    class GLUniformBuffer;
    class GLTexture;


    struct FilmicTMParameters
    {
        float sStrength_;
        float linStrength_;
        float linAngle_;
        float toeStrength_;
        float toeNumerator_;
        float toeDenominator_;
        float white_;
        float exposure_;

        template<class Archive> void serialize(Archive& ar, const std::uint32_t) {
            ar(cereal::make_nvp("sStrength", sStrength_),
                cereal::make_nvp("linStrength", linStrength_),
                cereal::make_nvp("linAngle", linAngle_),
                cereal::make_nvp("toeStrength", toeStrength_),
                cereal::make_nvp("toeNumerator", toeNumerator_),
                cereal::make_nvp("toeDenominator", toeDenominator_),
                cereal::make_nvp("white", white_),
                cereal::make_nvp("exposure", exposure_));
        }
    };

    /**
     *  Filmic tone-mapping operator.
     *  @see http://filmicgames.com/archives/75
     */
    class FilmicTMOperator
    {
    public:
        explicit FilmicTMOperator(ApplicationNodeBase* app);
        ~FilmicTMOperator();

        void RenderParameterSliders();
        void ApplyTonemapping(GLuint sourceTex, const FrameBuffer* fbo, std::size_t drawBufferIndex);
        void ApplyTonemapping(GLuint sourceTex, const FrameBuffer* fbo);

        void SetExposure(float exposure) { params_.exposure_ = exposure; }
        float GetExposure() const { return params_.exposure_; }

        template<class Archive> void SaveParameters(Archive& ar, const std::uint32_t) const {
            ar(cereal::make_nvp("params", params_));
        }

        template<class Archive> void LoadParameters(Archive& ar, const std::uint32_t) {
            ar(cereal::make_nvp("params", params_));
        }

    private:
        void ApplyTonemappingInternal(GLuint sourceTex);

        /** Holds the screen renderable for the tone-mapping. */
        std::unique_ptr<FullscreenQuad> renderable_;
        /** Holds the shader uniform ids. */
        std::vector<gl::GLint> uniformIds_;
        /** Holds the parameters for the tone-mapping. */
        FilmicTMParameters params_;
        /** Holds the filmic uniform buffer. */
        std::unique_ptr<GLUniformBuffer> filmicUBO_;
    };
}

CEREAL_CLASS_VERSION(viscom::enh::FilmicTMParameters, 1)
