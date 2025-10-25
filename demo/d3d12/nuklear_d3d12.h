/*
 * Nuklear - 1.32.0 - public domain
 * no warrenty implied; use at your own risk.
 * authored from 2015-2016 by Micha Mettke
 *
 * D3D12 backend created by Ludwig Fuechsl (2022)
 */
/*
 * ==============================================================
 *
 *                              API
 *
 * ===============================================================
 */
#ifndef NK_D3D12_H_
#define NK_D3D12_H_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/*
 * USAGE:
 *    - This function will initialize a new nuklear rendering context. The context will be bound to a GLOBAL DirectX 12 rendering state.
 */
NK_API struct nk_context *nk_d3d12_init(ID3D12Device *device, int width, int height, unsigned int max_vertex_buffer, unsigned int max_index_buffer, unsigned int max_user_textures);
/*
 * USAGE:
 *    - A call to this function prepares the global nuklear d3d12 backend for receiving font information’s. Use the obtained atlas pointer to load all required fonts and do all required font setup.
 */
NK_API void nk_d3d12_font_stash_begin(struct nk_font_atlas **atlas);
/*
 * USAGE:
 *    - Call this function after a call to nk_d3d12_font_stash_begin(...) when all fonts have been loaded and configured.
 *    - This function will place commands on the supplied ID3D12GraphicsCommandList.
 *    - This function will allocate temporary data that is required until the command list has finish executing. The temporary data can be free by calling nk_d3d12_font_stash_cleanup(...)
 */
NK_API void nk_d3d12_font_stash_end(ID3D12GraphicsCommandList *command_list);
/*
 * USAGE:
 *    - This function will free temporary data that was allocated by nk_d3d12_font_stash_begin(...)
 *    - Only call this function after the command list used in the nk_d3d12_font_stash_begin(...) function call has finished executing.
 *    - It is NOT required to call this function but highly recommended.
 */
NK_API void nk_d3d12_font_stash_cleanup();
/*
 * USAGE:
 *    - This function will setup the supplied texture (ID3D12Resource) for rendering custom images using the supplied D3D12_SHADER_RESOURCE_VIEW_DESC.
 *    - This function may override any previous calls to nk_d3d12_set_user_texture(...) while using the same index.
 *    - The returned handle can be used as texture handle to render custom images.
 *    - The caller must keep track of the state of the texture when it comes to rendering with nk_d3d12_render(...).
 */
NK_API nk_bool nk_d3d12_set_user_texture(unsigned int index, ID3D12Resource* texture, const D3D12_SHADER_RESOURCE_VIEW_DESC* description, nk_handle* handle_out);
/*
 * USAGE:
 *    - This function should be called within the user window proc to allow nuklear to listen to window events
 */
NK_API int nk_d3d12_handle_event(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
/*
 * USAGE:
 *    - A call to this function renders any previous placed nuklear draw calls and will flush all nuklear buffers for the next frame
 *    - This function will place commands on the supplied ID3D12GraphicsCommandList.
 *    - When using custom images for rendering make sure they are in the correct resource state (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) when calling this function.
 *    - This function will upload data to the gpu (64 + max_vertex_buffer + max_index_buffer BYTES).
 */
NK_API void nk_d3d12_render(ID3D12GraphicsCommandList *command_list, enum nk_anti_aliasing AA);
/*
 * USAGE:
 *    - This function will notify nuklear that the framebuffer dimensions have changed.
 */
NK_API void nk_d3d12_resize(int width, int height);
/*
 * USAGE:
 *    - This function will free the global d3d12 rendering state.
 */
NK_API void nk_d3d12_shutdown(void);

#endif
/*
 * ==============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================
 */
#ifdef NK_D3D12_IMPLEMENTATION

#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#include <d3d12.h>

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <float.h>
#include <assert.h>

#include "nuklear_d3d12_vertex_shader.h"
#include "nuklear_d3d12_pixel_shader.h"

struct nk_d3d12_vertex
{
    float position[2];
    float uv[2];
    nk_byte col[4];
};

static struct
{
    struct nk_context ctx;
    struct nk_font_atlas atlas;
    struct nk_buffer cmds;

    struct nk_draw_null_texture tex_null;
    unsigned int max_vertex_buffer;
    unsigned int max_index_buffer;
    unsigned int max_user_textures;

    D3D12_HEAP_PROPERTIES heap_prop_default;
    D3D12_HEAP_PROPERTIES heap_prop_upload;

    UINT cbv_srv_uav_desc_increment;

    D3D12_VIEWPORT viewport;
    ID3D12Device *device;
    ID3D12RootSignature *root_signature;
    ID3D12PipelineState *pipeline_state;
    ID3D12DescriptorHeap *desc_heap;
    ID3D12Resource *font_texture;
    ID3D12Resource *font_upload_buffer;
    ID3D12Resource *upload_buffer;
    ID3D12Resource *const_buffer;
    ID3D12Resource *index_buffer;
    ID3D12Resource *vertex_buffer;

    D3D12_CPU_DESCRIPTOR_HANDLE cpu_descriptor_handle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpu_descriptor_handle;
    D3D12_GPU_VIRTUAL_ADDRESS gpu_vertex_buffer_address;
    D3D12_GPU_VIRTUAL_ADDRESS gpu_index_buffer_address;
} d3d12;

NK_API void
nk_d3d12_render(ID3D12GraphicsCommandList *command_list, enum nk_anti_aliasing AA)
{
    HRESULT hr;
#ifdef NK_UINT_DRAW_INDEX
    DXGI_FORMAT index_buffer_format = DXGI_FORMAT_R32_UINT;
#else
    DXGI_FORMAT index_buffer_format = DXGI_FORMAT_R16_UINT;
#endif
    const UINT stride = sizeof(struct nk_d3d12_vertex);
    const struct nk_draw_command *cmd;
    UINT offset = 0;
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
    D3D12_INDEX_BUFFER_VIEW index_buffer_view;
    unsigned char* ptr_data;
    D3D12_RANGE map_range;
    D3D12_RESOURCE_BARRIER resource_barriers[3];

    /* Activate D3D12 pipeline state and config root signature */
    ID3D12GraphicsCommandList_SetPipelineState(command_list, d3d12.pipeline_state);
    ID3D12GraphicsCommandList_SetGraphicsRootSignature(command_list, d3d12.root_signature);
    ID3D12GraphicsCommandList_SetDescriptorHeaps(command_list, 1, &d3d12.desc_heap);
    ID3D12GraphicsCommandList_SetGraphicsRootDescriptorTable(command_list, 0, d3d12.gpu_descriptor_handle);

    /* Configure rendering pipeline */
    ID3D12GraphicsCommandList_IASetPrimitiveTopology(command_list, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    vertex_buffer_view.BufferLocation = d3d12.gpu_vertex_buffer_address;
    vertex_buffer_view.SizeInBytes = d3d12.max_vertex_buffer;
    vertex_buffer_view.StrideInBytes = stride;
    ID3D12GraphicsCommandList_IASetVertexBuffers(command_list, 0, 1, &vertex_buffer_view);
    index_buffer_view.BufferLocation = d3d12.gpu_index_buffer_address;
    index_buffer_view.Format = index_buffer_format;
    index_buffer_view.SizeInBytes = d3d12.max_index_buffer;
    ID3D12GraphicsCommandList_IASetIndexBuffer(command_list, &index_buffer_view);
    ID3D12GraphicsCommandList_RSSetViewports(command_list, 1, &d3d12.viewport);

    /* Map upload buffer to cpu accessible pointer */
    map_range.Begin = sizeof(float) * 4 * 4;
    map_range.End = map_range.Begin + d3d12.max_vertex_buffer + d3d12.max_index_buffer;
    hr = ID3D12Resource_Map(d3d12.upload_buffer, 0, &map_range, &ptr_data);
    NK_ASSERT(SUCCEEDED(hr));

    /* Nuklear convert and copy to upload buffer */
    {
    struct nk_convert_config config;
    NK_STORAGE const struct nk_draw_vertex_layout_element vertex_layout[] = {
        {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_d3d12_vertex, position)},
        {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_d3d12_vertex, uv)},
        {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct nk_d3d12_vertex, col)},
        {NK_VERTEX_LAYOUT_END}
    };
    memset(&config, 0, sizeof(config));
    config.vertex_layout = vertex_layout;
    config.vertex_size = sizeof(struct nk_d3d12_vertex);
    config.vertex_alignment = NK_ALIGNOF(struct nk_d3d12_vertex);
    config.global_alpha = 1.0f;
    config.shape_AA = AA;
    config.line_AA = AA;
    config.circle_segment_count = 22;
    config.curve_segment_count = 22;
    config.arc_segment_count = 22;
    config.tex_null = d3d12.tex_null;

    struct nk_buffer vbuf, ibuf;
    nk_buffer_init_fixed(&vbuf, &ptr_data[sizeof(float) * 4 * 4], (size_t)d3d12.max_vertex_buffer);
    nk_buffer_init_fixed(&ibuf, &ptr_data[sizeof(float) * 4 * 4 + d3d12.max_vertex_buffer], (size_t)d3d12.max_index_buffer);
    nk_convert(&d3d12.ctx, &d3d12.cmds, &vbuf, &ibuf, &config);
    }

    /* Close mapping range */
    ID3D12Resource_Unmap(d3d12.upload_buffer, 0, &map_range);

    /* Issue GPU resource change for copying */
    resource_barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    resource_barriers[0].Transition.pResource = d3d12.const_buffer;
    resource_barriers[0].Transition.Subresource = 0;
    resource_barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    resource_barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
    resource_barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    resource_barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    resource_barriers[1].Transition.pResource = d3d12.vertex_buffer;
    resource_barriers[1].Transition.Subresource = 0;
    resource_barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    resource_barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
    resource_barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    resource_barriers[2].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    resource_barriers[2].Transition.pResource = d3d12.index_buffer;
    resource_barriers[2].Transition.Subresource = 0;
    resource_barriers[2].Transition.StateBefore = D3D12_RESOURCE_STATE_INDEX_BUFFER;
    resource_barriers[2].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
    resource_barriers[2].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    ID3D12GraphicsCommandList_ResourceBarrier(command_list, 3, resource_barriers);

    /* Copy from upload buffer to gpu buffers */
    ID3D12GraphicsCommandList_CopyBufferRegion(command_list, d3d12.const_buffer, 0, d3d12.upload_buffer, 0, sizeof(float) * 4 * 4);
    ID3D12GraphicsCommandList_CopyBufferRegion(command_list, d3d12.vertex_buffer, 0, d3d12.upload_buffer, sizeof(float) * 4 * 4, d3d12.max_vertex_buffer);
    ID3D12GraphicsCommandList_CopyBufferRegion(command_list, d3d12.index_buffer, 0, d3d12.upload_buffer, sizeof(float) * 4 * 4 + d3d12.max_vertex_buffer, d3d12.max_index_buffer);

    /* Issue GPU resource change for rendering */
    resource_barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    resource_barriers[0].Transition.pResource = d3d12.const_buffer;
    resource_barriers[0].Transition.Subresource = 0;
    resource_barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    resource_barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    resource_barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    resource_barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    resource_barriers[1].Transition.pResource = d3d12.vertex_buffer;
    resource_barriers[1].Transition.Subresource = 0;
    resource_barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    resource_barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    resource_barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    resource_barriers[2].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    resource_barriers[2].Transition.pResource = d3d12.index_buffer;
    resource_barriers[2].Transition.Subresource = 0;
    resource_barriers[2].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    resource_barriers[2].Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
    resource_barriers[2].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    ID3D12GraphicsCommandList_ResourceBarrier(command_list, 3, resource_barriers);

    /* Issue draw commands */
    nk_draw_foreach(cmd, &d3d12.ctx, &d3d12.cmds)
    {
        D3D12_RECT scissor;
        UINT32 texture_id;

        /* Only place a drawcall in case the command contains drawable data */
        if(cmd->elem_count)
        {
            /* Setup scissor rect */
            scissor.left = (LONG)cmd->clip_rect.x;
            scissor.right = (LONG)(cmd->clip_rect.x + cmd->clip_rect.w);
            scissor.top = (LONG)cmd->clip_rect.y;
            scissor.bottom = (LONG)(cmd->clip_rect.y + cmd->clip_rect.h);
            ID3D12GraphicsCommandList_RSSetScissorRects(command_list, 1, &scissor);

            /* Setup texture (index to descriptor heap table) to use for draw call */
            texture_id = (UINT32)cmd->texture.id;
            ID3D12GraphicsCommandList_SetGraphicsRoot32BitConstants(command_list, 1, 1, &texture_id, 0);

            /* Dispatch draw call */
            ID3D12GraphicsCommandList_DrawIndexedInstanced(command_list, (UINT)cmd->elem_count, 1, offset, 0, 0);
            offset += cmd->elem_count;
        }
    }

    /* Default nuklear context and command buffer clear */
    nk_clear(&d3d12.ctx);
    nk_buffer_clear(&d3d12.cmds);
}

static void
nk_d3d12_get_projection_matrix(int width, int height, float *result)
{
    const float L = 0.0f;
    const float R = (float)width;
    const float T = 0.0f;
    const float B = (float)height;
    float matrix[4][4] =
    {
        { 0.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.5f, 0.0f },
        { 0.0f, 0.0f, 0.5f, 1.0f },
    };
    matrix[0][0] = 2.0f / (R - L);
    matrix[1][1] = 2.0f / (T - B);
    matrix[3][0] = (R + L) / (L - R);
    matrix[3][1] = (T + B) / (B - T);
    memcpy(result, matrix, sizeof(matrix));
}

NK_API void
nk_d3d12_resize(int width, int height)
{
    D3D12_RANGE map_range;
    void* ptr_data;

    /* Describe area to be mapped (the upload buffer region where the constant buffer / projection matrix) lives */
    map_range.Begin = 0;
    map_range.End = sizeof(float) * 4 * 4;

    /* Map area to cpu accassible pointer (from upload buffer) */
    if (SUCCEEDED(ID3D12Resource_Map(d3d12.upload_buffer, 0, &map_range, &ptr_data)))
    {
        /* Compute projection matrix into upload buffer */
        nk_d3d12_get_projection_matrix(width, height, (float*)ptr_data);
        ID3D12Resource_Unmap(d3d12.upload_buffer, 0, &map_range);

        /* Update internal viewport state to relect resize changes */
        d3d12.viewport.Width = (float)width;
        d3d12.viewport.Height = (float)height;
    }

    /*
        NOTE:
        When mapping and copying succeeds, the data will still be in CPU sided memory
        copying to the GPU is done in the nk_d3d12_render function
    */
}

NK_API int
nk_d3d12_handle_event(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    {
        int down = !((lparam >> 31) & 1);
        int ctrl = GetKeyState(VK_CONTROL) & (1 << 15);

        switch (wparam)
        {
        case VK_SHIFT:
        case VK_LSHIFT:
        case VK_RSHIFT:
            nk_input_key(&d3d12.ctx, NK_KEY_SHIFT, down);
            return 1;

        case VK_DELETE:
            nk_input_key(&d3d12.ctx, NK_KEY_DEL, down);
            return 1;

        case VK_RETURN:
        case VK_SEPARATOR:
            nk_input_key(&d3d12.ctx, NK_KEY_ENTER, down);
            return 1;

        case VK_TAB:
            nk_input_key(&d3d12.ctx, NK_KEY_TAB, down);
            return 1;

        case VK_LEFT:
            if (ctrl)
                nk_input_key(&d3d12.ctx, NK_KEY_TEXT_WORD_LEFT, down);
            else
                nk_input_key(&d3d12.ctx, NK_KEY_LEFT, down);
            return 1;

        case VK_RIGHT:
            if (ctrl)
                nk_input_key(&d3d12.ctx, NK_KEY_TEXT_WORD_RIGHT, down);
            else
                nk_input_key(&d3d12.ctx, NK_KEY_RIGHT, down);
            return 1;

        case VK_BACK:
            nk_input_key(&d3d12.ctx, NK_KEY_BACKSPACE, down);
            return 1;

        case VK_HOME:
            nk_input_key(&d3d12.ctx, NK_KEY_TEXT_START, down);
            nk_input_key(&d3d12.ctx, NK_KEY_SCROLL_START, down);
            return 1;

        case VK_END:
            nk_input_key(&d3d12.ctx, NK_KEY_TEXT_END, down);
            nk_input_key(&d3d12.ctx, NK_KEY_SCROLL_END, down);
            return 1;

        case VK_NEXT:
            nk_input_key(&d3d12.ctx, NK_KEY_SCROLL_DOWN, down);
            return 1;

        case VK_PRIOR:
            nk_input_key(&d3d12.ctx, NK_KEY_SCROLL_UP, down);
            return 1;

        case 'C':
            if (ctrl) {
                nk_input_key(&d3d12.ctx, NK_KEY_COPY, down);
                return 1;
            }
            break;

        case 'V':
            if (ctrl) {
                nk_input_key(&d3d12.ctx, NK_KEY_PASTE, down);
                return 1;
            }
            break;

        case 'X':
            if (ctrl) {
                nk_input_key(&d3d12.ctx, NK_KEY_CUT, down);
                return 1;
            }
            break;

        case 'Z':
            if (ctrl) {
                nk_input_key(&d3d12.ctx, NK_KEY_TEXT_UNDO, down);
                return 1;
            }
            break;

        case 'R':
            if (ctrl) {
                nk_input_key(&d3d12.ctx, NK_KEY_TEXT_REDO, down);
                return 1;
            }
            break;
        }
        return 0;
    }

    case WM_CHAR:
        if (wparam >= 32)
        {
            nk_input_unicode(&d3d12.ctx, (nk_rune)wparam);
            return 1;
        }
        break;

    case WM_LBUTTONDOWN:
        nk_input_button(&d3d12.ctx, NK_BUTTON_LEFT, (short)LOWORD(lparam), (short)HIWORD(lparam), 1);
        SetCapture(wnd);
        return 1;

    case WM_LBUTTONUP:
        nk_input_button(&d3d12.ctx, NK_BUTTON_DOUBLE, (short)LOWORD(lparam), (short)HIWORD(lparam), 0);
        nk_input_button(&d3d12.ctx, NK_BUTTON_LEFT, (short)LOWORD(lparam), (short)HIWORD(lparam), 0);
        ReleaseCapture();
        return 1;

    case WM_RBUTTONDOWN:
        nk_input_button(&d3d12.ctx, NK_BUTTON_RIGHT, (short)LOWORD(lparam), (short)HIWORD(lparam), 1);
        SetCapture(wnd);
        return 1;

    case WM_RBUTTONUP:
        nk_input_button(&d3d12.ctx, NK_BUTTON_RIGHT, (short)LOWORD(lparam), (short)HIWORD(lparam), 0);
        ReleaseCapture();
        return 1;

    case WM_MBUTTONDOWN:
        nk_input_button(&d3d12.ctx, NK_BUTTON_MIDDLE, (short)LOWORD(lparam), (short)HIWORD(lparam), 1);
        SetCapture(wnd);
        return 1;

    case WM_MBUTTONUP:
        nk_input_button(&d3d12.ctx, NK_BUTTON_MIDDLE, (short)LOWORD(lparam), (short)HIWORD(lparam), 0);
        ReleaseCapture();
        return 1;

    case WM_MOUSEWHEEL:
        nk_input_scroll(&d3d12.ctx, nk_vec2(0,(float)(short)HIWORD(wparam) / WHEEL_DELTA));
        return 1;

    case WM_MOUSEMOVE:
        nk_input_motion(&d3d12.ctx, (short)LOWORD(lparam), (short)HIWORD(lparam));
        return 1;

    case WM_LBUTTONDBLCLK:
        nk_input_button(&d3d12.ctx, NK_BUTTON_DOUBLE, (short)LOWORD(lparam), (short)HIWORD(lparam), 1);
        return 1;
    }

    return 0;
}

static void
nk_d3d12_clipboard_paste(nk_handle usr, struct nk_text_edit *edit)
{
    (void)usr;
    if (IsClipboardFormatAvailable(CF_UNICODETEXT) && OpenClipboard(NULL))
    {
        HGLOBAL mem = GetClipboardData(CF_UNICODETEXT);
        if (mem)
        {
            SIZE_T size = GlobalSize(mem) - 1;
            if (size)
            {
                LPCWSTR wstr = (LPCWSTR)GlobalLock(mem);
                if (wstr)
                {
                    int utf8size = WideCharToMultiByte(CP_UTF8, 0, wstr, size / sizeof(wchar_t), NULL, 0, NULL, NULL);
                    if (utf8size)
                    {
                        char* utf8 = (char*)malloc(utf8size);
                        if (utf8)
                        {
                            WideCharToMultiByte(CP_UTF8, 0, wstr, size / sizeof(wchar_t), utf8, utf8size, NULL, NULL);
                            nk_textedit_paste(edit, utf8, utf8size);
                            free(utf8);
                        }
                    }
                    GlobalUnlock(mem);
                }
            }
        }
        CloseClipboard();
    }
}

static void
nk_d3d12_clipboard_copy(nk_handle usr, const char *text, int len)
{
    (void)usr;
    if (OpenClipboard(NULL))
    {
        int wsize = MultiByteToWideChar(CP_UTF8, 0, text, len, NULL, 0);
        if (wsize)
        {
            HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, (wsize + 1) * sizeof(wchar_t));
            if (mem)
            {
                wchar_t* wstr = (wchar_t*)GlobalLock(mem);
                if (wstr)
                {
                    MultiByteToWideChar(CP_UTF8, 0, text, len, wstr, wsize);
                    wstr[wsize] = 0;
                    GlobalUnlock(mem);
                    SetClipboardData(CF_UNICODETEXT, mem);
                }
            }
        }
        CloseClipboard();
    }
}

NK_API struct nk_context*
nk_d3d12_init(ID3D12Device *device, int width, int height, unsigned int max_vertex_buffer, unsigned int max_index_buffer, unsigned int max_user_textures)
{
    HRESULT hr;
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv;
    D3D12_CPU_DESCRIPTOR_HANDLE cbv_handle;

    /* Do plain object / ref copys */
    d3d12.max_vertex_buffer = max_vertex_buffer;
    d3d12.max_index_buffer = max_index_buffer;
    d3d12.max_user_textures = max_user_textures;
    d3d12.device = device;
    ID3D12Device_AddRef(device);
    d3d12.font_texture = NULL;
    d3d12.font_upload_buffer = NULL;

    /* Init nuklear context */
    nk_init_default(&d3d12.ctx, 0);
    d3d12.ctx.clip.copy = nk_d3d12_clipboard_copy;
    d3d12.ctx.clip.paste = nk_d3d12_clipboard_paste;
    d3d12.ctx.clip.userdata = nk_handle_ptr(0);

    /* Init nuklear buffer */
    nk_buffer_init_default(&d3d12.cmds);

    /* Define Heap properties */
    d3d12.heap_prop_default.Type = D3D12_HEAP_TYPE_DEFAULT;
    d3d12.heap_prop_default.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    d3d12.heap_prop_default.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    d3d12.heap_prop_default.CreationNodeMask = 0;
    d3d12.heap_prop_default.VisibleNodeMask = 0;
    d3d12.heap_prop_upload.Type = D3D12_HEAP_TYPE_UPLOAD;
    d3d12.heap_prop_upload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    d3d12.heap_prop_upload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    d3d12.heap_prop_upload.CreationNodeMask = 0;
    d3d12.heap_prop_upload.VisibleNodeMask = 0;

    /* Create data objects */
    /* Create upload buffer */
    {
    D3D12_RESOURCE_DESC desc;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    desc.Width = (sizeof(float) * 4 * 4) + max_vertex_buffer + max_index_buffer; /* Needs to hold matrix + vertices + indicies */
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;
    hr = ID3D12Device_CreateCommittedResource(device, &d3d12.heap_prop_upload, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_SOURCE, NULL, &IID_ID3D12Resource, &d3d12.upload_buffer);
    NK_ASSERT(SUCCEEDED(hr));
    }
    /* Create constant buffer */
    {
    D3D12_RESOURCE_DESC desc;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    desc.Width = 256; /* Should be sizeof(float) * 4 * 4 - but this does not match how d3d12 works (min CBV size of 256) */
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;
    hr = ID3D12Device_CreateCommittedResource(device, &d3d12.heap_prop_default, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, NULL, &IID_ID3D12Resource, &d3d12.const_buffer);
    NK_ASSERT(SUCCEEDED(hr));
    }
    /* Create vertex buffer */
    {
    D3D12_RESOURCE_DESC desc;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    desc.Width = max_vertex_buffer;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;
    hr = ID3D12Device_CreateCommittedResource(device, &d3d12.heap_prop_default, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, NULL, &IID_ID3D12Resource, &d3d12.vertex_buffer);
    NK_ASSERT(SUCCEEDED(hr));
    }
    /* Create index buffer */
    {
    D3D12_RESOURCE_DESC desc;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    desc.Width = max_index_buffer;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;
    hr = ID3D12Device_CreateCommittedResource(device, &d3d12.heap_prop_default, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, NULL, &IID_ID3D12Resource, &d3d12.index_buffer);
    NK_ASSERT(SUCCEEDED(hr));
    }

    /* Create descriptor heap for shader root signature */
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 2 + max_user_textures;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask = 0;
        ID3D12Device_CreateDescriptorHeap(device, &desc, &IID_ID3D12DescriptorHeap, &d3d12.desc_heap);
    }

    /* Get address of first handle (CPU and GPU) */
    ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart(d3d12.desc_heap, &d3d12.cpu_descriptor_handle);
    ID3D12DescriptorHeap_GetGPUDescriptorHandleForHeapStart(d3d12.desc_heap, &d3d12.gpu_descriptor_handle);

    /* Get addresses of vertex & index buffers */
    d3d12.gpu_vertex_buffer_address = ID3D12Resource_GetGPUVirtualAddress(d3d12.vertex_buffer);
    d3d12.gpu_index_buffer_address = ID3D12Resource_GetGPUVirtualAddress(d3d12.index_buffer);

    /* Get handle increment */
    d3d12.cbv_srv_uav_desc_increment = ID3D12Device_GetDescriptorHandleIncrementSize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    /* Create view to constant buffer */
    cbv.BufferLocation = ID3D12Resource_GetGPUVirtualAddress(d3d12.const_buffer);
    cbv.SizeInBytes = 256;
    cbv_handle = d3d12.cpu_descriptor_handle;
    ID3D12Device_CreateConstantBufferView(device, &cbv, cbv_handle);

    /* Create root signature */
    hr = ID3D12Device_CreateRootSignature(device, 0, nk_d3d12_vertex_shader, sizeof(nk_d3d12_vertex_shader), &IID_ID3D12RootSignature, &d3d12.root_signature);
    NK_ASSERT(SUCCEEDED(hr));

    /* Create pipeline state */
    {
    /* Describe input layout */
    const D3D12_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, NK_OFFSETOF(struct nk_d3d12_vertex, position),  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, NK_OFFSETOF(struct nk_d3d12_vertex, uv),        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, NK_OFFSETOF(struct nk_d3d12_vertex, col),       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    /* Describe pipeline state */
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.pRootSignature = d3d12.root_signature;
    desc.VS.pShaderBytecode = nk_d3d12_vertex_shader;
    desc.VS.BytecodeLength = sizeof(nk_d3d12_vertex_shader);
    desc.PS.pShaderBytecode = nk_d3d12_pixel_shader;
    desc.PS.BytecodeLength = sizeof(nk_d3d12_pixel_shader);
    desc.BlendState.RenderTarget[0].BlendEnable = TRUE;
    desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    desc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
    desc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    desc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    desc.SampleMask = UINT_MAX;
    desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    desc.RasterizerState.CullMode= D3D12_CULL_MODE_NONE;
    desc.RasterizerState.DepthClipEnable = TRUE;
    desc.InputLayout.NumElements = _countof(layout);
    desc.InputLayout.pInputElementDescs = layout;
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc.NumRenderTargets = 1;
    desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; /* NOTE: When using HDR rendering you might have a different framebuffer format */
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.NodeMask = 0;

    /* Create PSO */
    hr = ID3D12Device_CreateGraphicsPipelineState(device, &desc, &IID_ID3D12PipelineState, &d3d12.pipeline_state);
    NK_ASSERT(SUCCEEDED(hr));
    }

    /* First time const buffer init */
    nk_d3d12_resize(width, height);

    /* viewport */
    d3d12.viewport.TopLeftX = 0.0f;
    d3d12.viewport.TopLeftY = 0.0f;
    d3d12.viewport.Width = (float)width;
    d3d12.viewport.Height = (float)height;
    d3d12.viewport.MinDepth = 0.0f;
    d3d12.viewport.MaxDepth = 1.0f;

    return &d3d12.ctx;
}

NK_API void
nk_d3d12_font_stash_begin(struct nk_font_atlas **atlas)
{
    /* Default nuklear font stash */
    nk_font_atlas_init_default(&d3d12.atlas);
    nk_font_atlas_begin(&d3d12.atlas);
    *atlas = &d3d12.atlas;
}

NK_API void
nk_d3d12_font_stash_end(ID3D12GraphicsCommandList *command_list)
{
    HRESULT hr;
    D3D12_TEXTURE_COPY_LOCATION  cpy_src, cpy_dest;
    D3D12_BOX cpy_box;
    D3D12_RESOURCE_BARRIER resource_barrier;
    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc;
    D3D12_CPU_DESCRIPTOR_HANDLE srv_handle;
    const void *image;
    void* ptr_data;
    int w, h;

    /* Bake nuklear font atlas */
    image = nk_font_atlas_bake(&d3d12.atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    NK_ASSERT(image);

    /* Create font texture */
    {
    D3D12_RESOURCE_DESC desc;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    desc.Width = w;
    desc.Height = h;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;
    hr = ID3D12Device_CreateCommittedResource(d3d12.device, &d3d12.heap_prop_default, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, NULL, &IID_ID3D12Resource, &d3d12.font_texture);
    NK_ASSERT(SUCCEEDED(hr));
    }

    /* Create font upload buffer */
    {
    D3D12_RESOURCE_DESC desc;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    desc.Width = w * h * 4;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;
    hr = ID3D12Device_CreateCommittedResource(d3d12.device, &d3d12.heap_prop_upload, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_SOURCE, NULL, &IID_ID3D12Resource, &d3d12.font_upload_buffer);
    NK_ASSERT(SUCCEEDED(hr));
    }

    /* Copy image data to upload buffer */
    hr = ID3D12Resource_Map(d3d12.font_upload_buffer, 0, NULL, &ptr_data);
    NK_ASSERT(SUCCEEDED(hr));
    memcpy(ptr_data, image, w * h * 4);
    ID3D12Resource_Unmap(d3d12.font_upload_buffer, 0, NULL);

    /* Execute copy operation */
    cpy_src.pResource = d3d12.font_upload_buffer;
    cpy_src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    cpy_src.PlacedFootprint.Offset = 0;
    cpy_src.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    cpy_src.PlacedFootprint.Footprint.Width = w;
    cpy_src.PlacedFootprint.Footprint.Height = h;
    cpy_src.PlacedFootprint.Footprint.Depth = 1;
    cpy_src.PlacedFootprint.Footprint.RowPitch = w * 4;
    cpy_dest.pResource = d3d12.font_texture;
    cpy_dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    cpy_dest.SubresourceIndex = 0;
    cpy_box.top = 0;
    cpy_box.left = 0;
    cpy_box.back = 1;
    cpy_box.bottom = h;
    cpy_box.right = w;
    cpy_box.front = 0;
    ID3D12GraphicsCommandList_CopyTextureRegion(command_list, &cpy_dest, 0, 0, 0, &cpy_src, &cpy_box);

    /* Bring texture in the right state for rendering */
    resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    resource_barrier.Transition.pResource = d3d12.font_texture;
    resource_barrier.Transition.Subresource = 0;
    resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    ID3D12GraphicsCommandList_ResourceBarrier(command_list, 1, &resource_barrier);

    /* Create the SRV for the font texture */
    srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv_desc.Texture2D.MipLevels = 1;
    srv_desc.Texture2D.MostDetailedMip = 0;
    srv_desc.Texture2D.PlaneSlice = 0;
    srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;
    srv_handle.ptr = d3d12.cpu_descriptor_handle.ptr + d3d12.cbv_srv_uav_desc_increment;
    ID3D12Device_CreateShaderResourceView(d3d12.device, d3d12.font_texture, &srv_desc, srv_handle);

    /* Done with nk atlas data. Atlas will be served with texture id 0 */
    nk_font_atlas_end(&d3d12.atlas, nk_handle_id(0), &d3d12.tex_null);

    /* Setup default font */
    if (d3d12.atlas.default_font)
        nk_style_set_font(&d3d12.ctx, &d3d12.atlas.default_font->handle);
}

NK_API
void nk_d3d12_font_stash_cleanup()
{
    if(d3d12.font_upload_buffer)
    {
        ID3D12Resource_Release(d3d12.font_upload_buffer);
        d3d12.font_upload_buffer = NULL;
    }
}

NK_API
nk_bool nk_d3d12_set_user_texture(unsigned int index, ID3D12Resource* texture, const D3D12_SHADER_RESOURCE_VIEW_DESC* description, nk_handle* handle_out)
{
    nk_bool result = nk_false;
    if(index < d3d12.max_user_textures)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE srv_handle;

        /* Get handle to texture (0 - Const Buffer; 1 - Font Texture; 2 - First user texture) */
        srv_handle.ptr = d3d12.cpu_descriptor_handle.ptr + ((2 + index) * d3d12.cbv_srv_uav_desc_increment);

        /* Create SRV */
        ID3D12Device_CreateShaderResourceView(d3d12.device, texture, description, srv_handle);

        /* Set nk handle (0 - Font Texture; 1 - First user texture) */
        *handle_out = nk_handle_id(1 + index);
        result = nk_true;
    }

    return result;
}

NK_API
void nk_d3d12_shutdown(void)
{
    /* Nuklear cleanup */
    nk_font_atlas_clear(&d3d12.atlas);
    nk_buffer_free(&d3d12.cmds);
    nk_free(&d3d12.ctx);

    /* DirectX 12 cleanup */
    ID3D12Device_Release(d3d12.device);
    ID3D12PipelineState_Release(d3d12.pipeline_state);
    ID3D12RootSignature_Release(d3d12.root_signature);
    ID3D12DescriptorHeap_Release(d3d12.desc_heap);
    ID3D12Resource_Release(d3d12.upload_buffer);
    ID3D12Resource_Release(d3d12.const_buffer);
    ID3D12Resource_Release(d3d12.index_buffer);
    ID3D12Resource_Release(d3d12.vertex_buffer);
    if(d3d12.font_texture)
        ID3D12Resource_Release(d3d12.font_texture);
    if(d3d12.font_upload_buffer)
        ID3D12Resource_Release(d3d12.font_upload_buffer);
}

#endif
