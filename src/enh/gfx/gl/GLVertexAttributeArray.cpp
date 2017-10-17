/**
 * @file   GLVertexAttributeArray.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.02.16
 *
 * @brief  Contains the implementation of GLVertexAttributeArray.
 */

#include "GLVertexAttributeArray.h"

namespace viscom::enh {

    /**
     * Constructor.
     * @param vertexBuffer the vertex buffer used to create the attributes.
     * @param indexBuffer the index buffer to bind
     */
    GLVertexAttributeArray::GLVertexAttributeArray(gl::GLuint vertexBuffer, gl::GLuint indexBuffer) :
        iBuffer_(indexBuffer),
        vBuffer_(vertexBuffer),
        vDesc_()
    {
    }

    /** Move constructor. */
    GLVertexAttributeArray::GLVertexAttributeArray(GLVertexAttributeArray&& orig) :
        vao_(std::move(orig.vao_)),
        iBuffer_(orig.iBuffer_),
        vBuffer_(orig.vBuffer_),
        vDesc_(std::move(orig.vDesc_))
    {
        orig.iBuffer_ = 0;
        orig.vBuffer_ = 0;
    }

    /** Move assignment operator. */
    GLVertexAttributeArray& GLVertexAttributeArray::operator =(GLVertexAttributeArray&& orig)
    {
        if (this != &orig) {
            this->~GLVertexAttributeArray();
            vao_ = std::move(orig.vao_);
            iBuffer_ = orig.iBuffer_;
            vBuffer_ = orig.vBuffer_;
            vDesc_ = std::move(orig.vDesc_);
            orig.iBuffer_ = 0;
            orig.vBuffer_ = 0;
        }
        return *this;
    }

    GLVertexAttributeArray::~GLVertexAttributeArray() = default;

    /** Disables all vertex attributes in the array. */
    void GLVertexAttributeArray::DisableAttributes()
    {
        gl::glBindVertexArray(vao_);
        gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, 0);

        for (const auto& desc : vDesc_) {
            if (desc.location_ > 0) {
                gl::glDisableVertexAttribArray(desc.location_);
            }
        }

        gl::glBindVertexArray(0);
    }

    /** Initializes the vertex attribute setup. */
    void GLVertexAttributeArray::StartAttributeSetup() const
    {
        gl::glBindVertexArray(vao_);
        gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    /** Ends the vertex attribute setup. */
    void GLVertexAttributeArray::EndAttributeSetup() const
    {
        gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, iBuffer_);
        gl::glBindVertexArray(0);
        gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    /** Enables the vertex attribute array. */
    void GLVertexAttributeArray::EnableVertexAttributeArray() const
    {
        gl::glBindVertexArray(vao_);
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    /** Disables the vertex attribute array. */
    void GLVertexAttributeArray::DisableVertexAttributeArray() const
    {
        gl::glBindVertexArray(0);
    }

    /**
     * Updates all vertex attributes.
     * This is used when BindingLocation changes after a shader recompile.
     */
    void GLVertexAttributeArray::UpdateVertexAttributes()
    {
        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, vBuffer_);
        gl::glBindVertexArray(vao_);
        gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, iBuffer_);

        for (const auto& desc : vDesc_) {
            if (desc.location_ > 0) {
                gl::glEnableVertexAttribArray(desc.location_);
                switch (desc.shaderType_)
                {
                case VAShaderType::FLOAT:
                    gl::glVertexAttribPointer(desc.location_, desc.size_,
                        desc.type_, desc.normalized_, desc.stride_,
                        (static_cast<char*> (nullptr)) + desc.offset_);
                    break;
                case VAShaderType::INTEGER:
                    gl::glVertexAttribIPointer(desc.location_, desc.size_,
                        desc.type_, desc.stride_, (static_cast<char*> (nullptr)) + desc.offset_);
                    break;
                case VAShaderType::DOUBLE:
                    gl::glVertexAttribLPointer(desc.location_, desc.size_,
                        desc.type_, desc.stride_, (static_cast<char*> (nullptr)) + desc.offset_);
                    break;
                }
            }
        }

        gl::glBindVertexArray(0);
        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, 0);
    }

    /**
     * Adds a new floating point attribute to the vertex.
     * @param location the binding location
     * @param size the number of components in the attribute
     * @param type the type in which the data is stored in the vertex buffer
     * @param normalized whether the attributes values will be normalized to (0..1)
     * @param stride the difference in bytes between of this attributes
     * @param offset the offset into the buffer to the first of this attributes
     */
    void GLVertexAttributeArray::AddVertexAttribute(gl::GLint location, int size, gl::GLenum type,
        gl::GLboolean normalized, gl::GLsizei stride, std::size_t offset)
    {
        if (location == -1)  return;
        vertex_attribute_desc desc;
        desc.shaderType_ = VAShaderType::FLOAT;
        desc.location_ = location;
        desc.size_ = size;
        desc.type_ = type;
        desc.normalized_ = normalized;
        desc.stride_ = stride;
        desc.offset_ = static_cast<unsigned int>(offset);
        vDesc_.push_back(std::move(desc));
        gl::glEnableVertexAttribArray(location);
        gl::glVertexAttribPointer(location, size, type, normalized, stride,
            (static_cast<char*> (nullptr)) + offset);
    }

    /**
     * Adds a new integer attribute to the vertex.
     * @param location the binding location
     * @param size the number of components in the attribute
     * @param type the type in which the data is stored in the vertex buffer
     * @param stride the difference in bytes between of this attributes
     * @param offset the offset into the buffer to the first of this attributes
     */
    void GLVertexAttributeArray::AddVertexAttributeI(gl::GLint location, int size, gl::GLenum type,
        gl::GLsizei stride, std::size_t offset)
    {
        if (location == -1)  return;
        vertex_attribute_desc desc;
        desc.shaderType_ = VAShaderType::INTEGER;
        desc.location_ = location;
        desc.size_ = size;
        desc.type_ = type;
        desc.stride_ = stride;
        desc.offset_ = static_cast<unsigned int>(offset);
        vDesc_.push_back(std::move(desc));
        gl::glEnableVertexAttribArray(location);
        gl::glVertexAttribIPointer(location, size, type, stride,
            (static_cast<char*> (nullptr)) + offset);
    }

    /**
     * Adds a new double attribute to the vertex.
     * @param location the binding location
     * @param size the number of components in the attribute
     * @param type the type in which the data is stored in the vertex buffer
     * @param stride the difference in bytes between of this attributes
     * @param offset the offset into the buffer to the first of this attributes
     */
    void GLVertexAttributeArray::AddVertexAttributeL(gl::GLint location, int size, gl::GLenum type,
        gl::GLsizei stride, std::size_t offset)
    {
        if (location == -1)  return;
        vertex_attribute_desc desc;
        desc.shaderType_ = VAShaderType::DOUBLE;
        desc.location_ = location;
        desc.size_ = size;
        desc.type_ = type;
        desc.stride_ = stride;
        desc.offset_ = static_cast<unsigned int>(offset);
        vDesc_.push_back(std::move(desc));
        gl::glEnableVertexAttribArray(location);
        gl::glVertexAttribLPointer(location, size, type, stride,
            (static_cast<char*> (nullptr)) + offset);
    }
}
