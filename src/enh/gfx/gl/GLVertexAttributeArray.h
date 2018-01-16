/**
 * @file   GLVertexAttributeArray.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.02.16
 *
 * @brief  Contains the definition of GLVertexAttributeArray.
 */

#pragma once

#include "enh/main.h"
#include <glbinding/gl/gl.h>
#include <vector>

namespace viscom::enh {

    /** The type of the vertex attribute inside a shader. */
    enum class VAShaderType
    {
        /** Integer type. */
        INTEGER,
        /** Single precision float type. */
        FLOAT,
        /** Double precision float type. */
        DOUBLE
    };

    /** Description of a vertex attribute. */
    struct vertex_attribute_desc
    {
        /** Holds the attribute type inside the shader. */
        VAShaderType shaderType_;
        /** Holds the attribute binding location. */
        gl::GLint location_;
        /** Holds the number of components in the attribute. */
        int size_;
        /** Holds the attributes type in the vertex buffer. */
        gl::GLenum type_;
        /** Holds whether the attribute should be normalized. */
        gl::GLboolean normalized_;
        /** The distance between 2 attributes of this type in bytes. */
        int stride_;
        /** The offset into the vertex to the beginning of this attribute in bytes. */
        unsigned int offset_;
    };

    /**
     * @brief
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2014.02.16
     */
    class GLVertexAttributeArray
    {
    public:
        GLVertexAttributeArray(gl::GLuint vertexBuffer, gl::GLuint indexBuffer);
        GLVertexAttributeArray(const GLVertexAttributeArray&) = delete;
        GLVertexAttributeArray& operator=(const GLVertexAttributeArray&) = delete;
        GLVertexAttributeArray(GLVertexAttributeArray&& orig);
        GLVertexAttributeArray& operator=(GLVertexAttributeArray&& orig);
        ~GLVertexAttributeArray();

        void StartAttributeSetup() const;
        void EndAttributeSetup() const;
        void AddVertexAttribute(gl::GLint location, int size, gl::GLenum type, gl::GLboolean normalized,
            gl::GLsizei stride, std::size_t offset);
        void AddVertexAttributeI(gl::GLint location, int size, gl::GLenum type, gl::GLsizei stride,
            std::size_t offset);
        void AddVertexAttributeL(gl::GLint location, int size, gl::GLenum type, gl::GLsizei stride,
            std::size_t offset);
        void UpdateVertexAttributes();
        void DisableAttributes();
        void EnableVertexAttributeArray() const;
        void DisableVertexAttributeArray() const;

    private:
        VertexArrayRAII vao_;
        gl::GLuint iBuffer_;
        gl::GLuint vBuffer_;
        std::vector<vertex_attribute_desc> vDesc_;

    };
}
