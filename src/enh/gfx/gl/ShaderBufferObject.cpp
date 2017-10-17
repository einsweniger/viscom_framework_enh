/**
 * @file   ShaderBufferObject.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2015.06.15
 *
 * @brief  Implementation of shader buffer objects.
 */

#include "ShaderBufferObject.h"
#include "ShaderBufferBindingPoints.h"
#include "GLBuffer.h"

namespace viscom::enh {

    ShaderBufferObject::ShaderBufferObject(const std::string& name, GLBuffer* buffer, ShaderBufferBindingPoints* bindings, bool cpuAccess) :
        buffer_{ buffer, false },
        bindingPoints_(bindings),
        bindingPoint_(bindingPoints_->GetBindingPoint(name))
    {
    }

    /**
         * Constructor.
         * @param name the name of the shader buffer object.
         * @param bindings the binding points used to bind the buffer to
         */
    ShaderBufferObject::ShaderBufferObject(const std::string& name, ShaderBufferBindingPoints* bindings, bool cpuAccess) :
        buffer_{ make_owned<GLBuffer>(cpuAccess ? gl::GL_STREAM_READ : gl::GL_DYNAMIC_DRAW) },
        bindingPoints_(bindings),
        bindingPoint_(bindingPoints_->GetBindingPoint(name))
    {
        gl::glBindBuffer(gl::GL_SHADER_STORAGE_BUFFER, buffer_->GetBuffer());
        gl::glBindBuffer(gl::GL_SHADER_STORAGE_BUFFER, 0);
    }

    ShaderBufferObject::ShaderBufferObject(gl::GLuint bindingPoint, ShaderBufferBindingPoints* bindings) :
        bindingPoints_(bindings),
        bindingPoint_(bindingPoint)
    {
    }

    ShaderBufferObject::ShaderBufferObject(const ShaderBufferObject& rhs) :
        ShaderBufferObject(rhs.bindingPoint_, rhs.bindingPoints_)
    {
        if (buffer_.is_owned()) buffer_ = make_owned<GLBuffer>(*rhs.buffer_);
        else buffer_ = owned_ptr<GLBuffer>(rhs.buffer_, false);
    }

    ShaderBufferObject& ShaderBufferObject::operator=(const ShaderBufferObject& rhs)
    {
        if (this != &rhs) {
            ShaderBufferObject tmp{ rhs };
            std::swap(*this, tmp);
        }
        return *this;
    }

    ShaderBufferObject::ShaderBufferObject(ShaderBufferObject&& rhs) :
        buffer_(std::move(rhs.buffer_)),
        bindingPoints_(std::move(rhs.bindingPoints_)),
        bindingPoint_(std::move(rhs.bindingPoint_))
    {
    }

    ShaderBufferObject& ShaderBufferObject::operator=(ShaderBufferObject&& rhs)
    {
        this->~ShaderBufferObject();
        buffer_ = std::move(rhs.buffer_);
        bindingPoints_ = std::move(rhs.bindingPoints_);
        bindingPoint_ = std::move(rhs.bindingPoint_);
        return *this;
    }

    ShaderBufferObject::~ShaderBufferObject() = default;

    void ShaderBufferObject::BindBuffer() const
    {
        gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, bindingPoint_, buffer_->GetBuffer());
    }
}
