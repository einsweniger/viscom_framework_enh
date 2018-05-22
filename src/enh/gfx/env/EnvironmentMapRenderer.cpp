/**
 * @file   EnvironmentMapRenderer.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.09.08
 *
 * @brief  Implementation of a renderer for environment maps.
 */

#include "EnvironmentMapRenderer.h"
#include "enh/ApplicationNodeBase.h"
#include "core/gfx/FullscreenQuad.h"
#include <glm/gtc/type_ptr.hpp>

namespace viscom::enh {

    EnvironmentMapRenderer::EnvironmentMapRenderer(ApplicationNodeBase* app) :
        screenQuad_(app->CreateFullscreenQuad("envmap/drawEnvMap.frag")),
        envMapUniformIds_(screenQuad_->GetGPUProgram()->GetUniformLocations({ "envMapTex", "vpInv", "camPos" }))
    {
    }

    EnvironmentMapRenderer::~EnvironmentMapRenderer() = default;

    void EnvironmentMapRenderer::Draw(const CameraHelper& camera, gl::GLuint tex)
    {
        Draw(camera.GetPosition(), camera.GetViewPerspectiveMatrix(), tex);
    }

    void EnvironmentMapRenderer::Draw(const glm::vec3& camPos, const glm::mat4& viewproj, gl::GLuint tex)
    {
        gl::glUseProgram(screenQuad_->GetGPUProgram()->getProgramId());

        gl::glActiveTexture(gl::GL_TEXTURE0 + 0);
        gl::glBindTexture(gl::GL_TEXTURE_2D, tex);
        gl::glUniform1i(envMapUniformIds_[0], 0);

        auto invMatrix = glm::inverse(viewproj);
        gl::glUniformMatrix4fv(envMapUniformIds_[1], 1, gl::GL_FALSE, glm::value_ptr(invMatrix));
        gl::glUniform3fv(envMapUniformIds_[2], 1, glm::value_ptr(camPos));
        screenQuad_->Draw();
    }
}
