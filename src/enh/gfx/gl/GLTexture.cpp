/**
 * @file   GLTexture.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2014.02.08
 *
 * @brief  Contains the implementation of GLTexture.
 */

#include "GLTexture.h"
#include "core/main.h"
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace viscom::enh {

    /**
     * Constructor.
     * Creates a 2d array texture.
     * @param w the textures width
     * @param h the textures height
     * @param desc the textures format
     * @param arraySize the array size
     */
    GLTexture::GLTexture(unsigned int w, unsigned int h, unsigned int arraySize, const TextureDescriptor& desc) :
        id_{ gl::GL_TEXTURE_2D_ARRAY },
        descriptor_(desc),
        width_(w),
        height_(h),
        depth_(arraySize),
        mipMapLevels_(1)
    {
        gl::glBindTexture(gl::GL_TEXTURE_2D_ARRAY, id_.textureId);
        gl::glTexStorage3D(gl::GL_TEXTURE_2D_ARRAY, mipMapLevels_, descriptor_.internalFormat_, width_, height_, depth_);
        gl::glBindTexture(gl::GL_TEXTURE_2D_ARRAY, 0);
        InitSampling();
    }

    GLTexture::GLTexture(unsigned int size, const TextureDescriptor& desc) :
        id_{ gl::GL_TEXTURE_1D },
        descriptor_(desc),
        width_(size),
        height_(1),
        depth_(1),
        mipMapLevels_(1)
    {
        gl::glBindTexture(id_.textureType, id_.textureId);
        gl::glTexStorage1D(id_.textureType, mipMapLevels_, descriptor_.internalFormat_, width_);
        gl::glBindTexture(id_.textureType, 0);
        InitSampling();
    }

    /**
     * Constructor.
     * Creates a 2d texture.
     * @param w the textures width
     * @param h the textures height
     * @param desc the textures format
     * @param data the textures data
     */
    GLTexture::GLTexture(unsigned int w, unsigned int h, const TextureDescriptor& desc, const void* data) :
        id_{ gl::GL_TEXTURE_2D },
        descriptor_(desc),
        width_(w),
        height_(h),
        depth_(1),
        mipMapLevels_(1)
    {
        gl::glBindTexture(id_.textureType, id_.textureId);
        gl::glTexStorage2D(id_.textureType, mipMapLevels_, descriptor_.internalFormat_, width_, height_);
        if (data) {
            gl::glTexSubImage2D(id_.textureType, 0, 0, 0, width_, depth_, descriptor_.format_,
                descriptor_.type_, data);
        }
        gl::glBindTexture(id_.textureType, 0);
        InitSampling();
    }

    /**
    * Constructor.
    * Creates a 3d texture.
    * @param w the textures width
    * @param h the textures height
    * @param d the textures depth
    * @param numMipLevels the number of MipMap levels to create.
    * @param desc the textures format
    * @param data the textures data
    */
    GLTexture::GLTexture(unsigned int w, unsigned int h, unsigned int d, unsigned int numMipLevels, const TextureDescriptor& desc, const void* data) :
        id_{ gl::GL_TEXTURE_3D },
        descriptor_(desc),
        width_(w),
        height_(h),
        depth_(d),
        mipMapLevels_(numMipLevels)
    {
        gl::GLint maxSizeSystem;
        gl::glGetIntegerv(gl::GL_MAX_3D_TEXTURE_SIZE, &maxSizeSystem);
        auto maxSize = glm::max(glm::max(width_, height_), depth_);

        if (maxSize > static_cast<unsigned int>(maxSizeSystem))
            throw std::runtime_error("Texture size is too big.");

        mipMapLevels_ = glm::min(mipMapLevels_, glm::max(1U, static_cast<unsigned int>(glm::log2(static_cast<float>(maxSize))) + 1));
        gl::glBindTexture(id_.textureType, id_.textureId);
        gl::glTexStorage3D(id_.textureType, mipMapLevels_, descriptor_.internalFormat_, width_, height_, depth_);
        if (data) {
            gl::glTexSubImage3D(id_.textureType, 0, 0, 0, 0, width_, height_, depth_,
                descriptor_.format_, descriptor_.type_, data);
        }
        gl::glBindTexture(id_.textureType, 0);
        InitSampling();
    }

    /**
     * Manages a pre-created texture.
     * @param texID the texture id
     * @param texType the textures type
     */
    GLTexture::GLTexture(TextureRAII texID, gl::GLenum texType, const TextureDescriptor& desc) :
        id_{ std::move(texID), texType },
        descriptor_(desc),
        width_(0),
        height_(0),
        depth_(0),
        mipMapLevels_(1)
    {
        gl::glBindTexture(id_.textureType, id_.textureId);
        gl::GLint qResult;
        auto queryType = id_.textureType;
        if (id_.textureType == gl::GL_TEXTURE_CUBE_MAP) queryType = gl::GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        gl::glGetTexLevelParameteriv(queryType, 0, gl::GL_TEXTURE_WIDTH, &qResult);
        width_ = static_cast<unsigned int>(qResult);
        gl::glGetTexLevelParameteriv(queryType, 0, gl::GL_TEXTURE_HEIGHT, &qResult);
        height_ = static_cast<unsigned int>(qResult);
        gl::glGetTexLevelParameteriv(queryType, 0, gl::GL_TEXTURE_DEPTH, &qResult);
        depth_ = static_cast<unsigned int>(qResult);
        InitSampling();
    }

    /** Destructor. */
    GLTexture::~GLTexture() = default;

    /** Initializes the sampler. */
    void GLTexture::InitSampling() const
    {
        SampleLinear();
        SampleWrapClamp();
    }

    /**
     * Activate the texture for rendering
     * @param textureUnit the texture unit to set the texture to
     */
    void GLTexture::ActivateTexture(gl::GLenum textureUnit) const
    {
        gl::glActiveTexture(textureUnit);
        gl::glBindTexture(id_.textureType, id_.textureId);
    }

    /**
     *  Activate the texture as an image.
     *  @param imageUnitIndex the index of the image unit to activate the texture for
     *  @param mipLevel the MipMap level to bind
     *  @param accessType the access type needed
     */
    void GLTexture::ActivateImage(gl::GLuint imageUnitIndex, gl::GLint mipLevel, gl::GLenum accessType) const
    {
        gl::glBindImageTexture(imageUnitIndex, id_.textureId, mipLevel, gl::GL_TRUE, 0, accessType, descriptor_.internalFormat_);
    }

    /**
     * Add a 2d bitmap to a texture array.
     * @param file the file to add
     * @param slice the array slice to add the files content to
     */
    void GLTexture::AddTextureToArray(const std::string& file, unsigned int slice) const
    {
        stbi_set_flip_vertically_on_load(1);
        auto channelsNeeded = 0;
        switch (descriptor_.format_) {
        case gl::GL_RED: channelsNeeded = 1; break;
        case gl::GL_RG: channelsNeeded = 2; break;
        case gl::GL_RGB: channelsNeeded = 3; break;
        case gl::GL_RGBA: channelsNeeded = 4; break; //-V112
        default:
            LOG(FATAL) << "Unknown texture format (" << descriptor_.format_ << ").";
            throw std::runtime_error("Unknown texture format.");
        }

        auto imgWidth = 0, imgHeight = 0, imgChannels = 0;
        auto image = stbi_load(file.c_str(), &imgWidth, &imgHeight, &imgChannels, channelsNeeded);
        if (!image) {
            LOG(FATAL) << R"(Could not load texture ")" << file.c_str() << R"(".)";
            throw std::runtime_error(R"(Could not load texture ")" + file + R"(".)");
        }

        if (width_ != imgWidth || height_ != imgHeight) {
            LOG(FATAL) << R"(Texture ")" << file.c_str() << R"(" has the wrong format!)";
            stbi_image_free(image);
            throw std::runtime_error(R"(Texture ")" + file + R"(" has the wrong format.)");
        }

        gl::glBindTexture(id_.textureType, id_.textureId);
        gl::glTexSubImage3D(id_.textureType, 0, 0, 0, slice, width_, height_, 1,
            descriptor_.format_, descriptor_.type_, image);
        gl::glBindTexture(id_.textureType, 0);

        stbi_image_free(image);
    }

    /**
     *  Sets the textures data.
     *  @param data the data to set.
     */
    void GLTexture::SetData(const void* data) const
    {
        glBindTexture(id_.textureType, id_.textureId);
        switch (id_.textureType)
        {
        case gl::GL_TEXTURE_1D:
            gl::glTexSubImage1D(id_.textureType, 0, 0, width_, descriptor_.format_, descriptor_.type_, data);
            break;
        case gl::GL_TEXTURE_2D:
            gl::glTexSubImage2D(id_.textureType, 0, 0, 0, width_, height_, descriptor_.format_, descriptor_.type_, data);
            break;
        case gl::GL_TEXTURE_3D:
            gl::glTexSubImage3D(id_.textureType, 0, 0, 0, 0, width_, height_, depth_, descriptor_.format_, descriptor_.type_, data);
            break;
        default:
            throw std::runtime_error("Texture format not supported for upload.");
        }        
        gl::glBindTexture(id_.textureType, 0);
    }

    /**
     *  Downloads the textures data to a vector.
     *  @param data the vector to contain the data.
     */
    void GLTexture::DownloadData(std::vector<uint8_t>& data, size_t offset, size_t size) const
    {
        if (size == 0) size = static_cast<std::size_t>(width_ * height_ * depth_ * descriptor_.bytesPP_);
        data.resize(size);
        assert(data.size() != 0);

        // TODO: create external PBOs for real asynchronous up-/download [8/19/2015 Sebastian Maisch]
        BufferRAII pbo;
        gl::glBindBuffer(gl::GL_PIXEL_PACK_BUFFER, pbo);
        gl::glBufferData(gl::GL_PIXEL_PACK_BUFFER, width_ * height_ * depth_ * descriptor_.bytesPP_, nullptr, gl::GL_STREAM_READ);

        gl::glBindTexture(id_.textureType, id_.textureId);
        gl::glGetTexImage(id_.textureType, 0, descriptor_.format_, descriptor_.type_, nullptr);

        gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
        auto gpuMem = gl::glMapBuffer(gl::GL_PIXEL_PACK_BUFFER, gl::GL_READ_ONLY);
        if (gpuMem) {
            memcpy(data.data() + offset, gpuMem, size);
            gl::glUnmapBuffer(gl::GL_PIXEL_PACK_BUFFER);
        }

        gl::glBindTexture(id_.textureType, 0);
        gl::glBindBuffer(gl::GL_PIXEL_PACK_BUFFER, 0);
    }

    void GLTexture::DownloadData8Bit(std::vector<uint8_t>& data) const
    {
        DownloadData8Bit(id_.textureId, descriptor_, id_.textureType, glm::uvec3(width_, height_, depth_), data);
    }

    void GLTexture::DownloadData8Bit(gl::GLuint texture, const TextureDescriptor& descriptor,
        gl::GLenum textureType, const glm::uvec3& size, std::vector<uint8_t>& data)
    {
        auto comp = 0;
        if (descriptor.format_ == gl::GL_RED) comp = 1;
        else if (descriptor.format_ == gl::GL_RG) comp = 2;
        else if (descriptor.format_ == gl::GL_RGB) comp = 3;
        else if (descriptor.format_ == gl::GL_RGBA) comp = 4; //-V112
        else {
            LOG(WARNING) << "Texture format not supported for downloading.";
            return;
        }

        data.resize(static_cast<std::size_t>(size.x * size.y * size.z * comp));
        assert(data.size() != 0);

        // TODO: create external PBOs for real asynchronous up-/download [8/19/2015 Sebastian Maisch]
        BufferRAII pbo;
        gl::glBindBuffer(gl::GL_PIXEL_PACK_BUFFER, pbo);
        gl::glBufferData(gl::GL_PIXEL_PACK_BUFFER, data.size(), nullptr, gl::GL_STREAM_READ);

        gl::glBindTexture(textureType, texture);
        gl::glGetTexImage(textureType, 0, descriptor.format_, gl::GL_UNSIGNED_BYTE, nullptr);

        gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
        auto gpuMem = gl::glMapBuffer(gl::GL_PIXEL_PACK_BUFFER, gl::GL_READ_ONLY);
        if (gpuMem) {
            memcpy(data.data(), gpuMem, data.size());
            glUnmapBuffer(gl::GL_PIXEL_PACK_BUFFER);
        }

        gl::glBindTexture(textureType, 0);
        gl::glBindBuffer(gl::GL_PIXEL_PACK_BUFFER, 0);
    }

    void GLTexture::SaveTextureToFile(const std::string& filename) const
    {
        assert(id_.textureType == gl::GL_TEXTURE_2D);
        SaveTextureToFile(id_.textureId, descriptor_, glm::uvec3(width_, height_, depth_), filename);
    }

    void GLTexture::SaveTextureToFile(gl::GLuint texture, const TextureDescriptor& descriptor,
        const glm::uvec3& size, const std::string& filename)
    {
        auto comp = 0;
        if (descriptor.format_ == gl::GL_RED) comp = 1;
        else if (descriptor.format_ == gl::GL_RG) comp = 2;
        else if (descriptor.format_ == gl::GL_RGB) comp = 3;
        else if (descriptor.format_ == gl::GL_RGBA) comp = 4; //-V112
        else {
            LOG(WARNING) << "Texture format not supported for saving.";
            return;
        }
        std::vector<uint8_t> screenData;
        DownloadData8Bit(texture, descriptor, gl::GL_TEXTURE_2D, size, screenData);

        auto stride = static_cast<int>(size.x * comp);
        for (unsigned i = 0; i < size.y / 2; ++i) {
            auto first = i * stride;
            auto last = ((i + 1) * stride) - 1;
            std::swap_ranges(screenData.begin() + first, screenData.begin() + last, screenData.end() - last - 1);
        }
        stbi_write_png(filename.c_str(), size.x, size.y, comp, screenData.data(), stride * static_cast<int>(sizeof(uint8_t)));
    }

    /**
     *  Uploads data to the texture from a vector.
     *  @param data the vector that contains the data.
     */
    void GLTexture::UploadData(std::vector<uint8_t>& data) const
    {
        assert(data.size() != 0);

        // TODO: create external PBOs for real asynchronous up-/download [8/19/2015 Sebastian Maisch]
        BufferRAII pbo;
        gl::glBindBuffer(gl::GL_PIXEL_UNPACK_BUFFER, pbo);
        gl::glBufferData(gl::GL_PIXEL_UNPACK_BUFFER, data.size(), nullptr, gl::GL_STREAM_DRAW);

        auto gpuMem = gl::glMapBuffer(gl::GL_PIXEL_UNPACK_BUFFER, gl::GL_WRITE_ONLY);
        if (gpuMem) {
            memcpy(gpuMem, data.data(), data.size());
            gl::glUnmapBuffer(gl::GL_PIXEL_UNPACK_BUFFER);
        }

        gl::glBindTexture(id_.textureType, id_.textureId);
        if (id_.textureType == gl::GL_TEXTURE_3D || id_.textureType == gl::GL_TEXTURE_2D_ARRAY) {
            glTexSubImage3D(id_.textureType, 0, 0, 0, 0, width_, height_, depth_, descriptor_.format_, descriptor_.type_, nullptr);
        } else if (id_.textureType == gl::GL_TEXTURE_2D || id_.textureType == gl::GL_TEXTURE_1D_ARRAY) {
            glTexSubImage2D(id_.textureType, 0, 0, 0, width_, height_, descriptor_.format_, descriptor_.type_, nullptr);
        } else {
            glTexSubImage1D(id_.textureType, 0, 0, width_, descriptor_.format_, descriptor_.type_, nullptr);
        }

        glBindTexture(id_.textureType, 0);
        glBindBuffer(gl::GL_PIXEL_UNPACK_BUFFER, 0);
    }

    /**
     *  Generates MipMaps for the texture.
     */
    void GLTexture::GenerateMipMaps() const
    {
        gl::glBindTexture(id_.textureType, id_.textureId);
        gl::glGenerateMipmap(id_.textureType);
        gl::glBindTexture(id_.textureType, 0);
    }

    /**
     *  Clears a texture.
     *  @param mipLevel the MipMap level to clear.
     *  @param data the clear color.
     */
    void GLTexture::ClearTexture(unsigned int mipLevel, const glm::vec4& data) const
    {
        assert(mipLevel < mipMapLevels_);
        gl::glClearTexImage(id_.textureId, mipLevel, descriptor_.format_, gl::GL_FLOAT, &data);
    }

    /**
     *  Returns the dimensions of a mip map level.
     */
    glm::uvec3 GLTexture::GetLevelDimensions(int level) const
    {
        gl::GLint w, h, d;
        gl::glBindTexture(id_.textureType, id_.textureId);
        gl::glGetTexLevelParameteriv(id_.textureType, level, gl::GL_TEXTURE_WIDTH, &w);
        gl::glGetTexLevelParameteriv(id_.textureType, level, gl::GL_TEXTURE_HEIGHT, &h);
        gl::glGetTexLevelParameteriv(id_.textureType, level, gl::GL_TEXTURE_DEPTH, &d);
        gl::glBindTexture(id_.textureType, 0);
        return glm::uvec3(static_cast<unsigned int>(w), static_cast<unsigned int>(h), static_cast<unsigned int>(d));
    }

    /**
     *  Sets the sampler parameters for mirroring.
     */
    void GLTexture::SampleWrapMirror() const
    {
        SetSampleWrap(gl::GL_MIRRORED_REPEAT);
    }

    /**
     *  Sets the sampler parameters for clamping.
     */
    void GLTexture::SampleWrapClamp() const
    {
        SetSampleWrap(gl::GL_CLAMP_TO_EDGE);
    }

    void GLTexture::SampleWrapRepeat() const
    {
        SetSampleWrap(gl::GL_REPEAT);
    }

    void GLTexture::SampleWrapMirrorClamp() const
    {
        SetSampleWrap(gl::GL_MIRROR_CLAMP_TO_EDGE);
    }

    void GLTexture::SampleWrapBorderColor(const glm::vec4& color) const
    {
        if (id_.textureType == gl::GL_TEXTURE_2D_MULTISAMPLE) return;
        SetSampleWrap(gl::GL_CLAMP_TO_BORDER);
        gl::glBindTexture(id_.textureType, id_.textureId);
        gl::glTexParameterfv(id_.textureType, gl::GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));
        gl::glBindTexture(id_.textureType, 0);
    }

    /**
     *  Sets the sampler parameters for texture wrapping mode.
     *  @param param the wrapping parameter
     */
    void GLTexture::SetSampleWrap(gl::GLenum param) const
    {
        if (id_.textureType == gl::GL_TEXTURE_2D_MULTISAMPLE) return;
        gl::glBindTexture(id_.textureType, id_.textureId);
        gl::glTexParameteri(id_.textureType, gl::GL_TEXTURE_WRAP_S, param);
        if (id_.textureType == gl::GL_TEXTURE_2D || id_.textureType == gl::GL_TEXTURE_3D) {
            gl::glTexParameteri(id_.textureType, gl::GL_TEXTURE_WRAP_T, param);
        }
        if (id_.textureType == gl::GL_TEXTURE_3D) {
            gl::glTexParameteri(id_.textureType, gl::GL_TEXTURE_WRAP_R, param);
        }
        gl::glBindTexture(id_.textureType, 0);
    }

    /**
     *  Sets the sampler parameters for linear filtering.
     */
    void GLTexture::SampleLinear() const
    {
        if (id_.textureType == gl::GL_TEXTURE_2D_MULTISAMPLE) return;
        gl::glBindTexture(id_.textureType, id_.textureId);
        if (mipMapLevels_ > 1) {
            gl::glTexParameteri(id_.textureType, gl::GL_TEXTURE_MAG_FILTER, gl::GL_LINEAR);
            gl::glTexParameteri(id_.textureType, gl::GL_TEXTURE_MIN_FILTER, gl::GL_LINEAR_MIPMAP_LINEAR);
            gl::glTexParameteri(id_.textureType, gl::GL_TEXTURE_BASE_LEVEL, 0);
        } else {
            gl::glTexParameteri(id_.textureType, gl::GL_TEXTURE_MAG_FILTER, gl::GL_LINEAR);
            gl::glTexParameteri(id_.textureType, gl::GL_TEXTURE_MIN_FILTER, gl::GL_LINEAR);
        }
        gl::glBindTexture(id_.textureType, 0);
    }

    /**
     *  Sets the sampler parameters for box filtering.
     */
    void GLTexture::SampleNearest() const
    {
        if (id_.textureType == gl::GL_TEXTURE_2D_MULTISAMPLE) return;
        glBindTexture(id_.textureType, id_.textureId);
        if (mipMapLevels_ > 1) {
            gl::glTexParameteri(id_.textureType, gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
            gl::glTexParameteri(id_.textureType, gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST_MIPMAP_NEAREST);
            gl::glTexParameteri(id_.textureType, gl::GL_TEXTURE_BASE_LEVEL, 0);
        } else {
            gl::glTexParameteri(id_.textureType, gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
            gl::glTexParameteri(id_.textureType, gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
        }
        gl::glBindTexture(id_.textureType, 0);
    }

    void GLTexture::ActivateShadowMapComparison() const
    {
        if (id_.textureType == gl::GL_TEXTURE_2D_MULTISAMPLE) return;
        gl::glBindTexture(id_.textureType, id_.textureId);
        gl::glTexParameteri(id_.textureType, gl::GL_TEXTURE_COMPARE_MODE, gl::GL_COMPARE_REF_TO_TEXTURE);
        gl::glTexParameteri(id_.textureType, gl::GL_TEXTURE_COMPARE_FUNC, gl::GL_LESS);
        gl::glBindTexture(id_.textureType, 0);
    }
}
