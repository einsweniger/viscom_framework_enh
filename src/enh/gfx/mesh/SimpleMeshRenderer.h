/**
 * @file   SimpleMeshRenderer.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.08.26
 *
 * @brief  Declaration of the SimpleMeshRenderer class.
 */

#pragma once

#include "enh/gfx/gl/ShaderMeshAttributes.h"

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

namespace viscom {
    class GPUProgram;
}

namespace viscom::enh {

    class ApplicationNodeBase;
    class GLBuffer;

    class SimpleMeshRenderer
    {
    public:
        explicit SimpleMeshRenderer(ApplicationNodeBase* app);
        ~SimpleMeshRenderer();

        void DrawCone(const glm::mat4& VPMatrix, const glm::mat4& modelMatrix, const glm::vec4& color) const;
        void DrawCube(const glm::mat4& VPMatrix, const glm::mat4& modelMatrix, const glm::vec4& color) const;
        void DrawCylinder(const glm::mat4& VPMatrix, const glm::mat4& modelMatrix, const glm::vec4& color) const;
        void DrawOctahedron(const glm::mat4& VPMatrix, const glm::mat4& modelMatrix, const glm::vec4& color) const;
        void DrawSphere(const glm::mat4& VPMatrix, const glm::mat4& modelMatrix, const glm::vec4& color) const;
        void DrawTorus(const glm::mat4& VPMatrix, const glm::mat4& modelMatrix, const glm::vec4& color) const;
        void DrawPoint(const glm::mat4& VPMatrix, const glm::mat4& modelMatrix, const glm::vec4& color, float pointSize) const;
        void DrawLine(const glm::mat4& VPMatrix, const glm::mat4& modelMatrix, const glm::vec4& color) const;

    private:
        void DrawSubmesh(const glm::mat4& VPMatrix, const glm::mat4& modelMatrix, const glm::vec4& color, unsigned int submeshId, float pointSize = 1.0f) const;

        using SimpleSubMesh = std::pair<unsigned int, unsigned int>;

        /** Holds the sub mesh information. */
        std::array<SimpleSubMesh, 8> submeshInfo_;
        /** Holds the simple GPU program for mesh rendering. */
        std::shared_ptr<GPUProgram> simpleProgram_;
        /** Holds the vertex buffer. */
        std::unique_ptr<GLBuffer> vBuffer_;
        /** Holds the index buffer of the mesh base. */
        std::unique_ptr<GLBuffer> iBuffer_;
        /** Holds the shader attribute bindings for the shader. */
        ShaderMeshAttributes drawAttribBinds_;
    };
}
