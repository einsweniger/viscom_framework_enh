/**
 * @file   ShaderMeshAttributes.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.12.15
 *
 * @brief  Contains information a mesh needs for rendering.
 */

#pragma once

#include <glbinding/gl/gl.h>
#include <memory>
#include <vector>

namespace viscom::enh {

    class GLVertexAttributeArray;

    using VertexAttributeBindings = std::vector<std::unique_ptr<GLVertexAttributeArray>>;

    class ShaderMeshAttributes
    {
    public:
        ShaderMeshAttributes();
        ~ShaderMeshAttributes();

        /** Returns the vertex attributes. */
        VertexAttributeBindings& GetVertexAttributes() { return vertexAttributes_; }
        /** Returns the vertex attributes. */
        const VertexAttributeBindings& GetVertexAttributes() const { return vertexAttributes_; }
        /** Returns the uniform ids. */
        std::vector<gl::GLint>& GetUniformIds() { return uniformIds_; }
        /** Returns the uniform ids. */
        const std::vector<gl::GLint>& GetUniformIds() const { return uniformIds_; }

    private:
        /** Holds the vertex attributes. */
        VertexAttributeBindings vertexAttributes_;
        /** Holds the shader uniform ids (for materials). */
        std::vector<gl::GLint> uniformIds_;
    };
}
