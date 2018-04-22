/**
 * @file   GLBuffer.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.05.08
 *
 * @brief  Declaration of an OpenGL buffer.
 */

#pragma once

#include "enh/core/type_traits.h"
#include "enh/main.h"
#include <glbinding/gl/gl.h>

namespace viscom::enh {

    class GLBuffer final
    {
    public:
        explicit GLBuffer(gl::GLenum usage);
        ~GLBuffer();
        GLBuffer(const GLBuffer&);
        GLBuffer& operator=(const GLBuffer&);
        GLBuffer(GLBuffer&&);
        GLBuffer& operator=(GLBuffer&&);

        void InitializeData(std::size_t size, const void* data);
        void UploadData(std::size_t offset, std::size_t size, const void* data);
        void DownloadData(std::size_t size, void* data) const;

        std::size_t GetBufferSize() const { return bufferSize_; }
        const BufferRAII& GetBuffer() const { return buffer_; }

        template<class T> std::enable_if_t<has_contiguous_memory<T>::value> InitializeData(const T& data);
        template<class T> std::enable_if_t<has_contiguous_memory<T>::value> UploadData(std::size_t offset, const T& data);
        template<class T> std::enable_if_t<has_contiguous_memory<T>::value>  DownloadData(T& data) const;

    private:
        /** Holds the buffer object name. */
        BufferRAII buffer_;
        /** Holds the current size of the buffer in bytes. */
        std::size_t bufferSize_;
        /** Holds the buffer usage. */
        gl::GLenum usage_;
    };

    template <class T> std::enable_if_t<has_contiguous_memory<T>::value> GLBuffer::InitializeData(const T& data)
    {
        InitializeData(sizeof(typename T::value_type) * data.size(), data.data());
    }

    template <class T> std::enable_if_t<has_contiguous_memory<T>::value> GLBuffer::UploadData(std::size_t offset, const T& data)
    {
        UploadData(offset, sizeof(typename T::value_type) * data.size(), data.data());
    }

    template <class T> std::enable_if_t<has_contiguous_memory<T>::value> GLBuffer::DownloadData(T& data) const
    {
        DownloadData(sizeof(typename T::value_type) * data.size(), data.data());
    }
}
