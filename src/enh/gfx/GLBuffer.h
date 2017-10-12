/**
 * @file   GLBuffer.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.05.08
 *
 * @brief  Declaration of an OpenGL buffer.
 */

#pragma once

#include "enh/main.h"
#include "enh/core/type_traits.h"
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

        void InitializeData(unsigned int size, const void* data);
        void UploadData(unsigned int offset, unsigned int size, const void* data);
        void DownloadData(unsigned int size, void* data) const;

        unsigned int GetBufferSize() const { return bufferSize_; }
        const BufferRAII& GetBuffer() const { return buffer_; }

        template<class T> std::enable_if_t<has_contiguous_memory<T>::value> InitializeData(const T& data);
        template<class T> std::enable_if_t<has_contiguous_memory<T>::value> UploadData(unsigned offset, const T& data);
        template<class T> std::enable_if_t<has_contiguous_memory<T>::value>  DownloadData(T& data) const;

    private:
        /** Holds the buffer object name. */
        BufferRAII buffer_;
        /** Holds the current size of the buffer in bytes. */
        unsigned int bufferSize_;
        /** Holds the buffer usage. */
        gl::GLenum usage_;
    };

    template <class T> std::enable_if_t<has_contiguous_memory<T>::value> GLBuffer::InitializeData(const T& data)
    {
        InitializeData(static_cast<unsigned int>(sizeof(T::value_type) * data.size()), data.data());
    }

    template <class T> std::enable_if_t<has_contiguous_memory<T>::value> GLBuffer::UploadData(unsigned offset, const T& data)
    {
        UploadData(offset, static_cast<unsigned int>(sizeof(T::value_type) * data.size()), data.data());
    }

    template <class T> std::enable_if_t<has_contiguous_memory<T>::value> GLBuffer::DownloadData(T& data) const
    {
        DownloadData(static_cast<unsigned int>(sizeof(T::value_type) * data.size()), data.data());
    }
}
