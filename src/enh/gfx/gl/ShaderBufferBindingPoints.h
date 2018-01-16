/**
 * @file   ShaderBufferBindingPoints.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.01.26
 *
 * @brief  Contains the definition of ShaderBufferBindingPoints.
 */

#pragma once

#include <glbinding/gl/gl.h>
#include <unordered_map>

namespace viscom::enh {

    /**
     * @brief Manages the binding points for shader buffers (uniform buffers, textures, ...).
     *
     * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
     * @date   2014.01.26
     */
    class ShaderBufferBindingPoints
    {
        /** Deleted copy constructor. */
        ShaderBufferBindingPoints(const ShaderBufferBindingPoints&) = delete;
        /** Deleted copy assignment operator. */
        ShaderBufferBindingPoints& operator=(const ShaderBufferBindingPoints&) = delete;

    public:
        ShaderBufferBindingPoints() : bindingPoints_(), nextBindingPoint_(0) {};
        ~ShaderBufferBindingPoints();

        gl::GLuint GetBindingPoint(const std::string& name);
        void BindBufferBlock(gl::GLuint program, const std::string& name);

    private:
        /** holds map that maps uniform buffer names to binding points. */
        std::unordered_map<std::string, gl::GLuint> bindingPoints_;
        /** holds the next free binding point. */
        unsigned int nextBindingPoint_;

    };
}
