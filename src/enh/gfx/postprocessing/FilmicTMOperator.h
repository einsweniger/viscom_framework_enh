/**
 * @file   FilmicTMOperator.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.12.21
 *
 * @brief  Declaration of the filmic tone-mapping operator class.
 */

#pragma once

#include <memory>
#include <vector>
#include <glm/vec2.hpp>
#include <glbinding/gl/gl.h>

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
        void ApplyTonemapping(GLTexture* sourceRT, FrameBuffer& fbo);
        void Resize(const glm::uvec2& screenSize);

        void SetExposure(float exposure) { params_.exposure_ = exposure; }
        float GetExposure() const { return params_.exposure_; }

        void SaveParameters(std::ostream& ostr) const;
        void LoadParameters(std::istream& istr);

    private:
        static const unsigned int VERSION = 1;

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
