/**
 * @file   ShaderBufferBindingPoints.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.01.26
 *
 * @brief  Contains the implementation of ShaderBufferBindingPoints.
 */

#include "ShaderBufferBindingPoints.h"

namespace viscom::enh {

    ShaderBufferBindingPoints::~ShaderBufferBindingPoints() = default;

    /**
     * Returns the binding point for a shader buffer with the given name.
     * @param name the name of the buffer.
     * @return the global binding point for this name
     */
    gl::GLuint ShaderBufferBindingPoints::GetBindingPoint(const std::string& name)
    {
        try {
            return bindingPoints_.at(name);
        }
        catch (std::out_of_range&) {
            bindingPoints_[name] = nextBindingPoint_;
            return nextBindingPoint_++;
        }
    }

    void ShaderBufferBindingPoints::BindBufferBlock(gl::GLuint program, const std::string& name)
    {
        auto uboIndex = gl::glGetUniformBlockIndex(program, name.c_str());
        gl::glUniformBlockBinding(program, uboIndex, GetBindingPoint(name.c_str()));
    }
}
