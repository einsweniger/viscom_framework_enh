/**
 * @file   GLTexture.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.02.08
 *
 * @brief  Contains the definition of GLTexture.
 */

#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "OpenGLRAIIWrapper.h"

namespace viscom::enh {

    class GLTexture;

    /** Describes the format of a texture. */
    struct TextureDescriptor
    {
        TextureDescriptor(unsigned int btsPP, gl::GLenum intFmt, gl::GLenum fmt, gl::GLenum tp) : bytesPP_(btsPP), internalFormat_(intFmt), format_(fmt), type_(tp) {};

        /** Holds the bytes per pixel of the format. */
        unsigned int bytesPP_;
        /** Holds the internal format. */
        gl::GLenum internalFormat_;
        /** Holds the format. */
        gl::GLenum format_;
        /** Holds the type. */
        gl::GLenum type_;
    };

    class TextureGLIdentifierAccessor
    {
        friend class GLTexture;
        friend class FrameBuffer;
        friend class ShadowMap;

        TextureGLIdentifierAccessor(TextureRAII id, gl::GLenum type) : textureId(std::move(id)), textureType(type) {};
        explicit TextureGLIdentifierAccessor(gl::GLenum type) : textureType(type) {};
        TextureRAII textureId;
        gl::GLenum textureType;
    };

    /**
    * @brief  General representation of an OpenGL texture.
    *
    * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
    * @date   2014.02.08
    */
    class GLTexture
    {
        /** Deleted copy constructor. */
        GLTexture(const GLTexture&) = delete;
        /** Deleted copy assignment operator. */
        GLTexture& operator=(const GLTexture&)  = delete;

    public:
        GLTexture(TextureRAII texID, gl::GLenum texType, const TextureDescriptor& desc);
        GLTexture(unsigned int size, const TextureDescriptor& desc);
        GLTexture(unsigned int width, unsigned int height, unsigned int arraySize, const TextureDescriptor& desc);
        GLTexture(unsigned int width, unsigned int height, const TextureDescriptor& desc, const void* data);
        GLTexture(unsigned int width, unsigned int height, unsigned int depth, unsigned int numMipLevels, const TextureDescriptor& desc, const void* data);
        virtual ~GLTexture();

        void ActivateTexture(gl::GLenum textureUnit) const;
        void ActivateImage(gl::GLuint imageUnitIndex, gl::GLint mipLevel, gl::GLenum accessType) const;
        void AddTextureToArray(const std::string& file, unsigned int slice) const;
        void SetData(const void* data) const;
        void DownloadData(std::vector<uint8_t>& data, size_t offset = 0, size_t size = 0) const;
        void DownloadData8Bit(std::vector<uint8_t>& data) const;
        void SaveTextureToFile(const std::string& filename) const;
        void UploadData(std::vector<uint8_t>& data) const;
        void GenerateMipMaps() const;
        void ClearTexture(unsigned int mipLevel, const glm::vec4& data) const;
        glm::uvec3 GetDimensions() const { return glm::uvec3(width_, height_, depth_); }
        glm::uvec3 GetLevelDimensions(int level) const;
        const TextureDescriptor& GetDescriptor() const { return descriptor_; }

        void SampleWrapMirror() const;
        void SampleWrapClamp() const;
        void SampleWrapRepeat() const;
        void SampleWrapMirrorClamp() const;
        void SampleWrapBorderColor(const glm::vec4& color) const;
        void SampleLinear() const;
        void SampleNearest() const;

        void ActivateShadowMapComparison() const;

        const TextureGLIdentifierAccessor& GetGLIdentifier() const { return id_; };
        gl::GLuint GetId() const {return id_.textureId;}
    private:
        void SetSampleWrap(gl::GLenum param) const;

        /** Holds the OpenGL texture id. */
        TextureGLIdentifierAccessor id_;
        /** Holds the texture descriptor. */
        TextureDescriptor descriptor_;

        /** Holds the width. */
        unsigned int width_;
        /** Holds the height. */
        unsigned int height_;
        /** Holds the depth or number of array slices. */
        unsigned int depth_;
        /** Holds the number of MipMap levels the texture has. */
        unsigned int mipMapLevels_;

        void InitSampling() const;
    };
}
