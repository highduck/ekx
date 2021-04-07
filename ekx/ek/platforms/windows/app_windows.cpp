#include <ek/util/detect_platform.hpp>

#if EK_UWP

#include <windows.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Graphics.Display.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Input.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.System.h>
#include <ppltasks.h>

#include <dxgi1_4.h>
#include <d3d11_3.h>
#include <DirectXMath.h>

#pragma comment (lib, "WindowsApp")
#pragma comment (lib, "dxguid")

#else // !EK_UWP
/// begin windows headers
//#include <winapifamily.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>

#pragma comment (linker, "/subsystem:windows")

#include <stdio.h>  /* freopen_s() */
#include <wchar.h>  /* wcslen() */

#pragma comment (lib, "kernel32")
#pragma comment (lib, "user32")
#pragma comment (lib, "shell32")    /* CommandLineToArgvW, DragQueryFileW, DragFinished */
#pragma comment (lib, "dxgi")
#pragma comment (lib, "d3d11")
#pragma comment (lib, "dxguid")

#ifndef D3D11_NO_HELPERS
#define D3D11_NO_HELPERS
#endif

#include <d3d11.h>
#include <dxgi.h>

#ifndef DXGI_SWAP_EFFECT_FLIP_DISCARD
// DXGI_SWAP_EFFECT_FLIP_DISCARD is only defined in newer Windows SDKs, so don't depend on it
#define DXGI_SWAP_EFFECT_FLIP_DISCARD (4)
#endif

#ifndef WM_MOUSEHWHEEL /* see https://github.com/floooh/sokol/issues/138 */
#define WM_MOUSEHWHEEL (0x020E)
#endif

#endif // #if EK_UWP

#define SAFE_RELEASE(obj) if (obj) { obj->Release(); obj=0; }

/// end windows headers

#include "impl/analytics_win.h"
#include "impl/device_win.h"
#include "impl/resources_win.h"
#include "impl/sharing_win.h"
#include "impl/user_prefs_win.h"

struct DXApp {
    ID3D11Device* device;
    ID3D11DeviceContext* device_context;
    ID3D11Texture2D* rt;
    ID3D11RenderTargetView* rtv;
    ID3D11Texture2D* msaa_rt;
    ID3D11RenderTargetView* msaa_rtv;
    ID3D11Texture2D* ds;
    ID3D11DepthStencilView* dsv;
    DXGI_SWAP_CHAIN_DESC swap_chain_desc;
    IDXGISwapChain* swap_chain;
};

#ifndef DPI_ENUMS_DECLARED

typedef enum PROCESS_DPI_AWARENESS
{
    PROCESS_DPI_UNAWARE = 0,
    PROCESS_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;

typedef enum MONITOR_DPI_TYPE {
    MDT_EFFECTIVE_DPI = 0,
    MDT_ANGULAR_DPI = 1,
    MDT_RAW_DPI = 2,
    MDT_DEFAULT = MDT_EFFECTIVE_DPI
} MONITOR_DPI_TYPE;

#endif /*DPI_ENUMS_DECLARED*/

struct WindowDpiState {
    bool aware;
    float content_scale;
    float window_scale;
    float mouse_scale;
}

struct AppWindow {
    HWND hwnd;
    HDC dc;
    UINT orig_codepage;
    LONG mouse_locked_x, mouse_locked_y;
    bool is_win10_or_greater;
    bool in_create_window;
    bool iconified;
    bool mouse_tracked;
    uint8_t mouse_capture_mask;
    WindowDpiState dpi;
    bool raw_input_mousepos_valid;
    LONG raw_input_mousepos_x;
    LONG raw_input_mousepos_y;
    uint8_t raw_input_data[256];
}

static DXApp dxApp{};
static AppWindow wnd{};

static inline HRESULT dxgi_GetBuffer(IDXGISwapChain* self, UINT Buffer, REFIID riid, void** ppSurface) {
    return self->GetBuffer(Buffer, riid, ppSurface);
}

static inline HRESULT d3d11_CreateRenderTargetView(ID3D11Device* self, ID3D11Resource *pResource, const D3D11_RENDER_TARGET_VIEW_DESC* pDesc, ID3D11RenderTargetView** ppRTView) {
    return self->CreateRenderTargetView(pResource, pDesc, ppRTView);
}

static inline HRESULT d3d11_CreateTexture2D(ID3D11Device* self, const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D) {
    return self->CreateTexture2D(pDesc, pInitialData, ppTexture2D);
}

static inline HRESULT d3d11_CreateDepthStencilView(ID3D11Device* self, ID3D11Resource* pResource, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc, ID3D11DepthStencilView** ppDepthStencilView) {
    return self->CreateDepthStencilView(pResource, pDesc, ppDepthStencilView);
}

static inline void d3d11_ResolveSubresource(ID3D11DeviceContext* self, ID3D11Resource* pDstResource, UINT DstSubresource, ID3D11Resource* pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format) {
    self->ResolveSubresource(pDstResource, DstSubresource, pSrcResource, SrcSubresource, Format);
}

static inline HRESULT dxgi_ResizeBuffers(IDXGISwapChain* self, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
    return self->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

static inline HRESULT dxgi_Present(IDXGISwapChain* self, UINT SyncInterval, UINT Flags) {
    return self->Present(SyncInterval, Flags);
}

void d3d11_create_device_and_swapchain() {
    DXGI_SWAP_CHAIN_DESC* sc_desc = &dxApp.d3d11.swap_chain_desc;
    sc_desc->BufferDesc.Width = (UINT)_sapp.framebuffer_width;
    sc_desc->BufferDesc.Height = (UINT)_sapp.framebuffer_height;
    sc_desc->BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sc_desc->BufferDesc.RefreshRate.Numerator = 60;
    sc_desc->BufferDesc.RefreshRate.Denominator = 1;
    sc_desc->OutputWindow = _sapp.win32.hwnd;
    sc_desc->Windowed = true;
    if (_sapp.win32.is_win10_or_greater) {
        sc_desc->BufferCount = 2;
        sc_desc->SwapEffect = (DXGI_SWAP_EFFECT) _SAPP_DXGI_SWAP_EFFECT_FLIP_DISCARD;
    }
    else {
        sc_desc->BufferCount = 1;
        sc_desc->SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    }
    sc_desc->SampleDesc.Count = 1;
    sc_desc->SampleDesc.Quality = 0;
    sc_desc->BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    UINT create_flags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifndef NDEBUG
    create_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL feature_level;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
            NULL,                           /* pAdapter (use default) */
            D3D_DRIVER_TYPE_HARDWARE,       /* DriverType */
            NULL,                           /* Software */
            create_flags,                   /* Flags */
            NULL,                           /* pFeatureLevels */
            0,                              /* FeatureLevels */
            D3D11_SDK_VERSION,              /* SDKVersion */
            sc_desc,                        /* pSwapChainDesc */
            &dxApp.swap_chain,        /* ppSwapChain */
            &dxApp.device,            /* ppDevice */
            &feature_level,                 /* pFeatureLevel */
            &dxApp.device_context);   /* ppImmediateContext */
    (void)hr;
    EK_ASSERT(SUCCEEDED(hr) && dxApp.swap_chain && dxApp.device && dxApp.device_context);
}

void d3d11_destroy_device_and_swapchain() {
    SAFE_RELEASE(dxApp.swap_chain);
    SAFE_RELEASE(dxApp.device_context);
    SAFE_RELEASE(dxApp.device);
}

void d3d11_create_default_render_target() {
    EK_ASSERT(0 == dxApp.rt);
    EK_ASSERT(0 == dxApp.rtv);
    EK_ASSERT(0 == dxApp.msaa_rt);
    EK_ASSERT(0 == dxApp.msaa_rtv);
    EK_ASSERT(0 == dxApp.ds);
    EK_ASSERT(0 == dxApp.dsv);

    HRESULT hr;

    /* view for the swapchain-created framebuffer */
    hr = dxgi_GetBuffer(dxApp.swap_chain, 0, IID_ID3D11Texture2D, (void**)&dxApp.rt);
    EK_ASSERT(SUCCEEDED(hr) && dxApp.rt);
    hr = d3d11_CreateRenderTargetView(dxApp.device, (ID3D11Resource*)dxApp.rt, NULL, &dxApp.rtv);
    EK_ASSERT(SUCCEEDED(hr) && dxApp.rtv);

    /* common desc for MSAA and depth-stencil texture */
    D3D11_TEXTURE2D_DESC tex_desc;
    memset(&tex_desc, 0, sizeof(tex_desc));
    tex_desc.Width = (UINT)_sapp.framebuffer_width;
    tex_desc.Height = (UINT)_sapp.framebuffer_height;
    tex_desc.MipLevels = 1;
    tex_desc.ArraySize = 1;
    tex_desc.Usage = D3D11_USAGE_DEFAULT;
    tex_desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    tex_desc.SampleDesc.Count = (UINT) _sapp.sample_count;
    tex_desc.SampleDesc.Quality = (UINT) (_sapp.sample_count > 1 ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0);

    /* create MSAA texture and view if antialiasing requested */
    if (_sapp.sample_count > 1) {
        tex_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        hr = d3d11_CreateTexture2D(dxApp.device, &tex_desc, NULL, &dxApp.msaa_rt);
        EK_ASSERT(SUCCEEDED(hr) && dxApp.msaa_rt);
        hr = d3d11_CreateRenderTargetView(dxApp.device, (ID3D11Resource*)dxApp.msaa_rt, NULL, &dxApp.msaa_rtv);
        EK_ASSERT(SUCCEEDED(hr) && dxApp.msaa_rtv);
    }

    /* texture and view for the depth-stencil-surface */
    tex_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    tex_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    hr = d3d11_CreateTexture2D(dxApp.device, &tex_desc, NULL, &dxApp.ds);
    EK_ASSERT(SUCCEEDED(hr) && dxApp.ds);
    hr = d3d11_CreateDepthStencilView(dxApp.device, (ID3D11Resource*)dxApp.ds, NULL, &dxApp.dsv);
    EK_ASSERT(SUCCEEDED(hr) && dxApp.dsv);
}

void d3d11_destroy_default_render_target(void) {
    SAFE_RELEASE(dxApp.rt);
    SAFE_RELEASE(dxApp.rtv);
    SAFE_RELEASE(dxApp.msaa_rt);
    SAFE_RELEASE(dxApp.msaa_rtv);
    SAFE_RELEASE(dxApp.ds);
    SAFE_RELEASE(dxApp.dsv);
}

void d3d11_resize_default_render_target(void) {
    if (dxApp.swap_chain) {
        d3d11_destroy_default_render_target();
        dxgi_ResizeBuffers(dxApp.swap_chain, dxApp.swap_chain_desc.BufferCount, (UINT)_sapp.framebuffer_width, (UINT)_sapp.framebuffer_height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
        d3d11_create_default_render_target();
    }
}

void d3d11_present(void) {
    /* do MSAA resolve if needed */
    if (_sapp.sample_count > 1) {
        EK_ASSERT(dxApp.rt);
        EK_ASSERT(dxApp.msaa_rt);
        d3d11_ResolveSubresource(dxApp.device_context, (ID3D11Resource*)dxApp.rt, 0, (ID3D11Resource*)dxApp.msaa_rt, 0, DXGI_FORMAT_B8G8R8A8_UNORM);
    }
    dxgi_Present(dxApp.swap_chain, (UINT)_sapp.swap_interval, 0);
}

void init_console() {
    if (_sapp.desc.win32_console_create || _sapp.desc.win32_console_attach) {
        BOOL con_valid = FALSE;
        if (_sapp.desc.win32_console_create) {
            con_valid = AllocConsole();
        }
        else if (_sapp.desc.win32_console_attach) {
            con_valid = AttachConsole(ATTACH_PARENT_PROCESS);
        }
        if (con_valid) {
            FILE* res_fp = 0;
            errno_t err;
            err = freopen_s(&res_fp, "CON", "w", stdout);
            err = freopen_s(&res_fp, "CON", "w", stderr);
            (void)err;
        }
    }
    if (_sapp.desc.win32_console_utf8) {
        _sapp.win32.orig_codepage = GetConsoleOutputCP();
        SetConsoleOutputCP(CP_UTF8);
    }
}

namespace ek {

void start_application() {

    _sapp_init_state(desc);
    init_console();
    _sapp.win32.is_win10_or_greater = _sapp_win32_is_win10_or_greater();
    _sapp_win32_uwp_init_keytable();
    _sapp_win32_uwp_utf8_to_wide(_sapp.window_title, _sapp.window_title_wide, sizeof(_sapp.window_title_wide));
    _sapp_win32_init_dpi();
    _sapp_win32_create_window();
#if defined(SOKOL_D3D11)
    _sapp_d3d11_create_device_and_swapchain();
        _sapp_d3d11_create_default_render_target();
#endif
#if defined(SOKOL_GLCORE33)
    _sapp_wgl_init();
        _sapp_wgl_load_extensions();
        _sapp_wgl_create_context();
#endif
    _sapp.valid = true;

    bool done = false;
    while (!(done || _sapp.quit_ordered)) {
        MSG msg;
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (WM_QUIT == msg.message) {
                done = true;
                continue;
            }
            else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        _sapp_frame();
#if defined(SOKOL_D3D11)
        _sapp_d3d11_present();
            if (IsIconic(_sapp.win32.hwnd)) {
                Sleep((DWORD)(16 * _sapp.swap_interval));
            }
#endif
#if defined(SOKOL_GLCORE33)
        _sapp_wgl_swap_buffers();
#endif
        /* check for window resized, this cannot happen in WM_SIZE as it explodes memory usage */
        if (_sapp_win32_update_dimensions()) {
#if defined(SOKOL_D3D11)
            _sapp_d3d11_resize_default_render_target();
#endif
            _sapp_win32_uwp_app_event(SAPP_EVENTTYPE_RESIZED);
        }
        if (_sapp.quit_requested) {
            PostMessage(_sapp.win32.hwnd, WM_CLOSE, 0, 0);
        }
    }
    _sapp_call_cleanup();

#if defined(SOKOL_D3D11)
    _sapp_d3d11_destroy_default_render_target();
        _sapp_d3d11_destroy_device_and_swapchain();
#else
    _sapp_wgl_destroy_context();
    _sapp_wgl_shutdown();
#endif
    _sapp_win32_destroy_window();
    _sapp_win32_restore_console();
    _sapp_discard_state();

    dispatch_init();
    dispatch_device_ready();
}

}

char** command_line_to_utf8_argv(LPWSTR w_command_line, int* o_argc) {
    int argc = 0;
    char** argv = 0;
    char* args;

    LPWSTR* w_argv = CommandLineToArgvW(w_command_line, &argc);
    if (w_argv == NULL) {
        EK_ERROR << "Win32: failed to parse command line";
        EK_ABORT();
    } else {
        size_t size = wcslen(w_command_line) * 4;
        argv = (char**) calloc(1, ((size_t)argc + 1) * sizeof(char*) + size);
        EK_ASSERT(argv);
        args = (char*) &argv[argc + 1];
        int n;
        for (int i = 0; i < argc; ++i) {
            n = WideCharToMultiByte(CP_UTF8, 0, w_argv[i], -1, args, (int)size, NULL, NULL);
            if (n == 0) {
                EK_ERROR << "Win32: failed to convert all arguments to utf8";
                EK_ABORT();
                break;
            }
            argv[i] = args;
            size -= (size_t)n;
            args += n;
        }
        free(w_argv);
    }
    *o_argc = argc;
    return argv;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    (void)hInstance;
    (void)(hPrevInstance);
    (void)(lpCmdLine);
    (void)(nCmdShow);
    int argc_utf8 = 0;
    char** argv_utf8 = _sapp_win32_command_line_to_utf8_argv(GetCommandLineW(), &argc_utf8);

    g_app.args = {argc_utf8, argv_utf8};
    ::ek::main();

    free(argv_utf8);
    return 0;
}