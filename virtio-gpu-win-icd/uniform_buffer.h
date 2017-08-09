#pragma once

#include <vector>

#include "win_types.h"

template <typename T>
class UniformBuffer {
private:
    std::vector<T> m_data;

    UINT32 m_vgl_ctx;

    UINT32 m_handle;
    UINT32 m_binding;
    UINT32 m_max_size;
    BOOL m_is_created;

public:
    UniformBuffer(UINT32 vgl_ctx, UINT32 handle, UINT32 m_binding);
    ~UniformBuffer();

    INT clear(VOID);
    INT push(T elt);
    INT flush(VOID);
};


template <typename T>
UniformBuffer<T>::UniformBuffer(UINT32 vgl_ctx, UINT32 handle, UINT32 binding)
    : m_vgl_ctx(vgl_ctx), m_handle(handle), m_data(9), m_binding(binding),
      m_is_created(FALSE), m_max_size(0)
{ }

template<typename T>
UniformBuffer<T>::~UniformBuffer()
{
    if (m_is_created) {
        VirGL::detachResource(m_vgl_ctx, m_handle);
        VirGL::unrefResource(m_handle);
    }
}

template <typename T>
INT UniformBuffer<T>::clear(VOID)
{
    m_data.clear();

    return STATUS_SUCCESS;
}

template <typename T>
INT UniformBuffer<T>::push(T elt)
{
    m_data.push_back(elt);

    return STATUS_SUCCESS;
}

template <typename T>
INT UniformBuffer<T>::flush(VOID)
{
    UINT32 size = (UINT32)(m_data.size() * sizeof(T));

    if (m_is_created && m_max_size < size) {
        VirGL::detachResource(m_vgl_ctx, m_handle);
        VirGL::unrefResource(m_handle);
        m_is_created = FALSE;
    }

    if (!m_is_created) {
        m_max_size = size;
        m_is_created = TRUE;

        VirGL::RESOURCE_CREATION info = { 0 };
        info.handle = m_handle;
        info.format = 0x40;
        info.bind = m_binding;
        info.width = m_max_size;
        info.height = 1;
        info.depth = 1;
        info.array_size = 1;

        VirGL::createResource3d(m_vgl_ctx, info);
        VirGL::attachResource(m_vgl_ctx, m_handle);
    }

    VirGL::INLINE_WRITE inline_info = { 0 };
    inline_info.data = m_data.data();
    inline_info.data_len = size;
    inline_info.handle = m_handle;
    inline_info.usage = size;
    inline_info.width = size;
    inline_info.height = 1;
    inline_info.depth = 1;

    VirGL::VirglCommandBuffer cmd(m_vgl_ctx);
    cmd.inlineWrite(inline_info);
    return cmd.submitCommandBuffer();
}