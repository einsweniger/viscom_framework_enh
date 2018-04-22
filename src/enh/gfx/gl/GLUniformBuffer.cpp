/**
 * @file   GLUniformBuffer.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.01.25
 *
 * @brief  Contains the implementation of GLUniformBuffer.
 */

#include "GLUniformBuffer.h"
#include "GLBuffer.h"
#include "ShaderBufferBindingPoints.h"

#include <cassert>

namespace viscom::enh {

    GLUniformBuffer::GLUniformBuffer(const std::string& name, ShaderBufferBindingPoints* bindings) :
        buffer_{ make_owned<GLBuffer>(gl::GL_STREAM_DRAW) },
        bindingPoints_(bindings),
        bindingPoint_(bindingPoints_->GetBindingPoint(name)),
        uboName_(name)
    {
        gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, buffer_->GetBuffer());
        gl::glBindBuffer(gl::GL_UNIFORM_BUFFER, 0);
    }

    /**
     * Constructor.
     * @param name the name of the uniform buffer.
     * @param size the size of the uniform buffer
     * @param bindings the binding points used to bind the buffer to
     */
    GLUniformBuffer::GLUniformBuffer(const std::string& name, std::size_t size,
        ShaderBufferBindingPoints* bindings) :
        GLUniformBuffer(name, bindings)
    {
        buffer_->InitializeData(size, nullptr);
        BindBuffer();
    }

    /**
     *  Copy constructor.
     */
    GLUniformBuffer::GLUniformBuffer(const GLUniformBuffer& rhs) :
        GLUniformBuffer(rhs.uboName_, rhs.bindingPoints_)
    {
        std::vector<int8_t> tmp(rhs.buffer_->GetBufferSize());
        rhs.buffer_->DownloadData(tmp);
        buffer_->InitializeData(tmp);
        BindBuffer();
    }

    /**
     *  Copy assignment operator.
     */
    GLUniformBuffer& GLUniformBuffer::operator=(const GLUniformBuffer& rhs)
    {
        if (this != &rhs) {
            GLUniformBuffer tmp{ rhs };
            std::swap(*this, tmp);
        }
        return *this;
    }

    /**
     *  Move constructor.
     */
    GLUniformBuffer::GLUniformBuffer(GLUniformBuffer&& rhs) :
        buffer_(std::move(rhs.buffer_)),
        bindingPoints_(std::move(rhs.bindingPoints_)),
        bindingPoint_(std::move(rhs.bindingPoint_)),
        uboName_(std::move(rhs.uboName_))
    {
    }

    /**
     *  Move assignment operator.
     */
    GLUniformBuffer& GLUniformBuffer::operator=(GLUniformBuffer&& rhs)
    {
        if (this != &rhs) {
            this->~GLUniformBuffer();
            buffer_ = std::move(rhs.buffer_);
            bindingPoints_ = std::move(rhs.bindingPoints_);
            bindingPoint_ = std::move(rhs.bindingPoint_);
            uboName_ = std::move(rhs.uboName_);
        }
        return *this;
    }

    /** Destructor. */
    GLUniformBuffer::~GLUniformBuffer() = default;

    /**
     * Upload data to the uniform buffer.
     * @param offset the offset into the buffer to store the data
     * @param size the size of the data
     * @param data the data to store in the buffer
     */
    void GLUniformBuffer::UploadData(std::size_t offset, std::size_t size, const void* data)
    {
        assert((offset + size) <= buffer_->GetBufferSize());
        buffer_->UploadData(offset, size, data);
    }

    void GLUniformBuffer::BindBuffer() const
    {
        gl::glBindBufferRange(gl::GL_UNIFORM_BUFFER, bindingPoint_, buffer_->GetBuffer(), 0, buffer_->GetBufferSize());
    }
}
