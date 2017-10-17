/**
 * @file   GLBuffer.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.05.08
 *
 * @brief  Implementation of an OpenGL buffer.
 */

#include "GLBuffer.h"

namespace viscom::enh {

    GLBuffer::GLBuffer(gl::GLenum usage) :
        bufferSize_{ 0 },
        usage_{ usage }
    {
    }


    GLBuffer::~GLBuffer() = default;

    GLBuffer::GLBuffer(const GLBuffer& rhs) :
        bufferSize_{ rhs.bufferSize_ },
        usage_{ rhs.usage_ }
    {
        std::vector<int8_t> tmp(bufferSize_);
        rhs.DownloadData(tmp);
        InitializeData(tmp);
    }

    GLBuffer& GLBuffer::operator=(const GLBuffer& rhs)
    {
        if (this != &rhs) {
            GLBuffer tmp{ rhs };
            std::swap(*this, tmp);
        }
        return *this;
    }

    GLBuffer::GLBuffer(GLBuffer&& rhs) :
        buffer_{ std::move(rhs.buffer_) },
        bufferSize_{ rhs.bufferSize_ },
        usage_{ rhs.usage_ }
    {
        rhs.bufferSize_ = 0;
    }

    GLBuffer& GLBuffer::operator=(GLBuffer&& rhs)
    {
        this->~GLBuffer();
        buffer_ = std::move(rhs.buffer_);
        bufferSize_ = rhs.bufferSize_;
        usage_ = rhs.usage_;
        rhs.bufferSize_ = 0;
        return *this;
    }

    void GLBuffer::InitializeData(unsigned int size, const void* data)
    {
        bufferSize_ = size;
        gl::glNamedBufferData(buffer_, size, data, usage_);
    }

    void GLBuffer::UploadData(unsigned int offset, unsigned int size, const void* data)
    {
        if (offset + size > bufferSize_) {
            std::vector<int8_t> tmp(offset);
            bufferSize_ = offset + size;
            gl::glGetNamedBufferSubData(buffer_, 0, offset, tmp.data());
            gl::glNamedBufferData(buffer_, bufferSize_, nullptr, usage_);
            gl::glNamedBufferSubData(buffer_, 0, offset, tmp.data());
        }

        gl::glNamedBufferSubData(buffer_, offset, size, data);
    }

    void GLBuffer::DownloadData(unsigned int size, void* data) const
    {
        gl::glGetNamedBufferSubData(buffer_, 0, size, data);
    }
}
