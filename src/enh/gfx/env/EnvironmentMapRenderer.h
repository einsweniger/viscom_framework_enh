/**
 * @file   EnvironmentMapRenderer.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.09.08
 *
 * @brief  Declaration of a renderer for environment maps.
 */

#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glbinding/gl/gl.h>

namespace viscom {
    class GPUProgram;
    class FullscreenQuad;
    class CameraHelper;
}

namespace viscom::enh {

    class ApplicationNodeBase;

    class EnvironmentMapRenderer
    {
    public:
        explicit EnvironmentMapRenderer(ApplicationNodeBase* app);
        ~EnvironmentMapRenderer();

        void Draw(const CameraHelper& camera, gl::GLuint tex);
        void Draw(const glm::vec3& camPos, const glm::mat4& viewproj, gl::GLuint tex);

    private:
        /** Holds the screen quad renderable. */
        std::unique_ptr<FullscreenQuad> screenQuad_;
        /** Holds the uniform bindings. */
        std::vector<gl::GLint> envMapUniformIds_;
    };
}
