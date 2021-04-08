#include <ek/util/detect_platform.hpp>
#include <ek/app/app.hpp>

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

//#pragma comment (lib, "WindowsApp")
//#pragma comment (lib, "dxguid")

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

//#pragma comment (lib, "kernel32")
//#pragma comment (lib, "user32")
//#pragma comment (lib, "shell32")    /* CommandLineToArgvW, DragQueryFileW, DragFinished */
//#pragma comment (lib, "dxgi")
//#pragma comment (lib, "d3d11")
//#pragma comment (lib, "dxguid")

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

typedef enum PROCESS_DPI_AWARENESS {
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

static DXApp
dxApp{
};
static AppWindow wnd{};

static int cfgSamplesCount = 1;
static bool cfgAllowHighDPI = true;

using namespace ek;
using namespace ek::app;

static inline HRESULT dxgi_GetBuffer(IDXGISwapChain* self, UINT Buffer, REFIID riid, void** ppSurface) {
    return self->GetBuffer(Buffer, riid, ppSurface);
}

static inline HRESULT
d3d11_CreateRenderTargetView(ID3D11Device* self, ID3D11Resource* pResource, const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
                             ID3D11RenderTargetView** ppRTView) {
    return self->CreateRenderTargetView(pResource, pDesc, ppRTView);
}

static inline HRESULT
d3d11_CreateTexture2D(ID3D11Device* self, const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData,
                      ID3D11Texture2D** ppTexture2D) {
    return self->CreateTexture2D(pDesc, pInitialData, ppTexture2D);
}

static inline HRESULT
d3d11_CreateDepthStencilView(ID3D11Device* self, ID3D11Resource* pResource, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc,
                             ID3D11DepthStencilView** ppDepthStencilView) {
    return self->CreateDepthStencilView(pResource, pDesc, ppDepthStencilView);
}

static inline void
d3d11_ResolveSubresource(ID3D11DeviceContext* self, ID3D11Resource* pDstResource, UINT DstSubresource,
                         ID3D11Resource* pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format) {
    self->ResolveSubresource(pDstResource, DstSubresource, pSrcResource, SrcSubresource, Format);
}

static inline HRESULT
dxgi_ResizeBuffers(IDXGISwapChain* self, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat,
                   UINT SwapChainFlags) {
    return self->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

static inline HRESULT dxgi_Present(IDXGISwapChain* self, UINT SyncInterval, UINT Flags) {
    return self->Present(SyncInterval, Flags);
}

void d3d11_create_device_and_swapchain() {
    DXGI_SWAP_CHAIN_DESC* sc_desc = &dxApp.d3d11.swap_chain_desc;
    sc_desc->BufferDesc.Width = (UINT) g_app.drawable_size.x;
    sc_desc->BufferDesc.Height = (UINT) g_app.drawable_size.y;
    sc_desc->BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sc_desc->BufferDesc.RefreshRate.Numerator = 60;
    sc_desc->BufferDesc.RefreshRate.Denominator = 1;
    sc_desc->OutputWindow = wnd.hwnd;
    sc_desc->Windowed = true;
    if (wnd.is_win10_or_greater) {
        sc_desc->BufferCount = 2;
        sc_desc->SwapEffect = (DXGI_SWAP_EFFECT) _SAPP_DXGI_SWAP_EFFECT_FLIP_DISCARD;
    } else {
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
    (void) hr;
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
    hr = dxgi_GetBuffer(dxApp.swap_chain, 0, IID_ID3D11Texture2D, (void**) &dxApp.rt);
    EK_ASSERT(SUCCEEDED(hr) && dxApp.rt);
    hr = d3d11_CreateRenderTargetView(dxApp.device, (ID3D11Resource*) dxApp.rt, NULL, &dxApp.rtv);
    EK_ASSERT(SUCCEEDED(hr) && dxApp.rtv);

    /* common desc for MSAA and depth-stencil texture */
    D3D11_TEXTURE2D_DESC tex_desc;
    memset(&tex_desc, 0, sizeof(tex_desc));
    tex_desc.Width = (UINT) g_app.drawable_size.x;
    tex_desc.Height = (UINT) g_app.drawable_size.y;
    tex_desc.MipLevels = 1;
    tex_desc.ArraySize = 1;
    tex_desc.Usage = D3D11_USAGE_DEFAULT;
    tex_desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    tex_desc.SampleDesc.Count = (UINT) cfgSamplesCount;
    tex_desc.SampleDesc.Quality = (UINT)(cfgSamplesCount > 1 ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0);

    /* create MSAA texture and view if antialiasing requested */
    if (cfgSamplesCount > 1) {
        tex_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        hr = d3d11_CreateTexture2D(dxApp.device, &tex_desc, NULL, &dxApp.msaa_rt);
        EK_ASSERT(SUCCEEDED(hr) && dxApp.msaa_rt);
        hr = d3d11_CreateRenderTargetView(dxApp.device, (ID3D11Resource*) dxApp.msaa_rt, NULL, &dxApp.msaa_rtv);
        EK_ASSERT(SUCCEEDED(hr) && dxApp.msaa_rtv);
    }

    /* texture and view for the depth-stencil-surface */
    tex_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    tex_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    hr = d3d11_CreateTexture2D(dxApp.device, &tex_desc, NULL, &dxApp.ds);
    EK_ASSERT(SUCCEEDED(hr) && dxApp.ds);
    hr = d3d11_CreateDepthStencilView(dxApp.device, (ID3D11Resource*) dxApp.ds, NULL, &dxApp.dsv);
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
        dxgi_ResizeBuffers(dxApp.swap_chain,
                           dxApp.swap_chain_desc.BufferCount,
                           (UINT) g_app.drawable_size.x,
                           (UINT) g_app.drawable_size.y,
                           DXGI_FORMAT_B8G8R8A8_UNORM, 0);
        d3d11_create_default_render_target();
    }
}

void d3d11_present(void) {
    /* do MSAA resolve if needed */
    if (cfgSamplesCount > 1) {
        EK_ASSERT(dxApp.rt);
        EK_ASSERT(dxApp.msaa_rt);
        d3d11_ResolveSubresource(dxApp.device_context, (ID3D11Resource*) dxApp.rt, 0, (ID3D11Resource*) dxApp.msaa_rt,
                                 0, DXGI_FORMAT_B8G8R8A8_UNORM);
    }
    dxgi_Present(dxApp.swap_chain, (UINT)/*swap_interval */ 1, 0);
}

void init_console() {
    // config
    bool win32_console_create = true;
    bool win32_console_attach = true;
    bool win32_console_utf8 = true;

    if (win32_console_create || win32_console_attach) {
        BOOL con_valid = FALSE;
        if (win32_console_create) {
            con_valid = AllocConsole();
        } else if (win32_console_attach) {
            con_valid = AttachConsole(ATTACH_PARENT_PROCESS);
        }
        if (con_valid) {
            FILE* res_fp = 0;
            errno_t err;
            err = freopen_s(&res_fp, "CON", "w", stdout);
            err = freopen_s(&res_fp, "CON", "w", stderr);
            (void) err;
        }
    }
    if (win32_console_utf8) {
        wnd.orig_codepage = GetConsoleOutputCP();
        SetConsoleOutputCP(CP_UTF8);
    }
}

bool win32_is_win10_or_greater() {
    HMODULE h = GetModuleHandleW(L"kernel32.dll");
    if (NULL != h) {
        return (NULL != GetProcAddress(h, "GetSystemCpuSetInformation"));
    } else {
        return false;
    }
}

bool win32_uwp_utf8_to_wide(const char* src, wchar_t* dst, int dst_num_bytes) {
    EK_ASSERT(src && dst && (dst_num_bytes > 1));
    memset(dst, 0, (size_t)dst_num_bytes);
    const int dst_chars = dst_num_bytes / (int)sizeof(wchar_t);
    const int dst_needed = MultiByteToWideChar(CP_UTF8, 0, src, -1, 0, 0);
    if ((dst_needed > 0) && (dst_needed < dst_chars)) {
        MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, dst_chars);
        return true;
    }
    /* input string doesn't fit into destination buffer */
    return false;
}

void win32_uwp_app_event(event_type type) {
    event_t ev{type};
    dispatch_event(ev);
}

void win32_uwp_init_keytable() {
    /* same as GLFW */
//    keycodes[0x00B] = KEYCODE_0;
//    keycodes[0x002] = KEYCODE_1;
//    keycodes[0x003] = KEYCODE_2;
//    keycodes[0x004] = KEYCODE_3;
//    keycodes[0x005] = KEYCODE_4;
//    keycodes[0x006] = KEYCODE_5;
//    keycodes[0x007] = KEYCODE_6;
//    keycodes[0x008] = KEYCODE_7;
//    keycodes[0x009] = KEYCODE_8;
//    keycodes[0x00A] = KEYCODE_9;
//    keycodes[0x01E] = KEYCODE_A;
//    keycodes[0x030] = KEYCODE_B;
//    keycodes[0x02E] = KEYCODE_C;
//    keycodes[0x020] = KEYCODE_D;
//    keycodes[0x012] = KEYCODE_E;
//    keycodes[0x021] = KEYCODE_F;
//    keycodes[0x022] = KEYCODE_G;
//    keycodes[0x023] = KEYCODE_H;
//    keycodes[0x017] = KEYCODE_I;
//    keycodes[0x024] = KEYCODE_J;
//    keycodes[0x025] = KEYCODE_K;
//    keycodes[0x026] = KEYCODE_L;
//    keycodes[0x032] = KEYCODE_M;
//    keycodes[0x031] = KEYCODE_N;
//    keycodes[0x018] = KEYCODE_O;
//    keycodes[0x019] = KEYCODE_P;
//    keycodes[0x010] = KEYCODE_Q;
//    keycodes[0x013] = KEYCODE_R;
//    keycodes[0x01F] = KEYCODE_S;
//    keycodes[0x014] = KEYCODE_T;
//    keycodes[0x016] = KEYCODE_U;
//    keycodes[0x02F] = KEYCODE_V;
//    keycodes[0x011] = KEYCODE_W;
//    keycodes[0x02D] = KEYCODE_X;
//    keycodes[0x015] = KEYCODE_Y;
//    keycodes[0x02C] = KEYCODE_Z;
//    keycodes[0x028] = KEYCODE_APOSTROPHE;
//    keycodes[0x02B] = KEYCODE_BACKSLASH;
//    keycodes[0x033] = KEYCODE_COMMA;
//    keycodes[0x00D] = KEYCODE_EQUAL;
//    keycodes[0x029] = KEYCODE_GRAVE_ACCENT;
//    keycodes[0x01A] = KEYCODE_LEFT_BRACKET;
//    keycodes[0x00C] = KEYCODE_MINUS;
//    keycodes[0x034] = KEYCODE_PERIOD;
//    keycodes[0x01B] = KEYCODE_RIGHT_BRACKET;
//    keycodes[0x027] = KEYCODE_SEMICOLON;
//    keycodes[0x035] = KEYCODE_SLASH;
//    keycodes[0x056] = KEYCODE_WORLD_2;
//    keycodes[0x00E] = KEYCODE_BACKSPACE;
//    keycodes[0x153] = KEYCODE_DELETE;
//    keycodes[0x14F] = KEYCODE_END;
//    keycodes[0x01C] = KEYCODE_ENTER;
//    keycodes[0x001] = KEYCODE_ESCAPE;
//    keycodes[0x147] = KEYCODE_HOME;
//    keycodes[0x152] = KEYCODE_INSERT;
//    keycodes[0x15D] = KEYCODE_MENU;
//    keycodes[0x151] = KEYCODE_PAGE_DOWN;
//    keycodes[0x149] = KEYCODE_PAGE_UP;
//    keycodes[0x045] = KEYCODE_PAUSE;
//    keycodes[0x146] = KEYCODE_PAUSE;
//    keycodes[0x039] = KEYCODE_SPACE;
//    keycodes[0x00F] = KEYCODE_TAB;
//    keycodes[0x03A] = KEYCODE_CAPS_LOCK;
//    keycodes[0x145] = KEYCODE_NUM_LOCK;
//    keycodes[0x046] = KEYCODE_SCROLL_LOCK;
//    keycodes[0x03B] = KEYCODE_F1;
//    keycodes[0x03C] = KEYCODE_F2;
//    keycodes[0x03D] = KEYCODE_F3;
//    keycodes[0x03E] = KEYCODE_F4;
//    keycodes[0x03F] = KEYCODE_F5;
//    keycodes[0x040] = KEYCODE_F6;
//    keycodes[0x041] = KEYCODE_F7;
//    keycodes[0x042] = KEYCODE_F8;
//    keycodes[0x043] = KEYCODE_F9;
//    keycodes[0x044] = KEYCODE_F10;
//    keycodes[0x057] = KEYCODE_F11;
//    keycodes[0x058] = KEYCODE_F12;
//    keycodes[0x064] = KEYCODE_F13;
//    keycodes[0x065] = KEYCODE_F14;
//    keycodes[0x066] = KEYCODE_F15;
//    keycodes[0x067] = KEYCODE_F16;
//    keycodes[0x068] = KEYCODE_F17;
//    keycodes[0x069] = KEYCODE_F18;
//    keycodes[0x06A] = KEYCODE_F19;
//    keycodes[0x06B] = KEYCODE_F20;
//    keycodes[0x06C] = KEYCODE_F21;
//    keycodes[0x06D] = KEYCODE_F22;
//    keycodes[0x06E] = KEYCODE_F23;
//    keycodes[0x076] = KEYCODE_F24;
//    keycodes[0x038] = KEYCODE_LEFT_ALT;
//    keycodes[0x01D] = KEYCODE_LEFT_CONTROL;
//    keycodes[0x02A] = KEYCODE_LEFT_SHIFT;
//    keycodes[0x15B] = KEYCODE_LEFT_SUPER;
//    keycodes[0x137] = KEYCODE_PRINT_SCREEN;
//    keycodes[0x138] = KEYCODE_RIGHT_ALT;
//    keycodes[0x11D] = KEYCODE_RIGHT_CONTROL;
//    keycodes[0x036] = KEYCODE_RIGHT_SHIFT;
//    keycodes[0x15C] = KEYCODE_RIGHT_SUPER;
//    keycodes[0x150] = KEYCODE_DOWN;
//    keycodes[0x14B] = KEYCODE_LEFT;
//    keycodes[0x14D] = KEYCODE_RIGHT;
//    keycodes[0x148] = KEYCODE_UP;
//    keycodes[0x052] = KEYCODE_KP_0;
//    keycodes[0x04F] = KEYCODE_KP_1;
//    keycodes[0x050] = KEYCODE_KP_2;
//    keycodes[0x051] = KEYCODE_KP_3;
//    keycodes[0x04B] = KEYCODE_KP_4;
//    keycodes[0x04C] = KEYCODE_KP_5;
//    keycodes[0x04D] = KEYCODE_KP_6;
//    keycodes[0x047] = KEYCODE_KP_7;
//    keycodes[0x048] = KEYCODE_KP_8;
//    keycodes[0x049] = KEYCODE_KP_9;
//    keycodes[0x04E] = KEYCODE_KP_ADD;
//    keycodes[0x053] = KEYCODE_KP_DECIMAL;
//    keycodes[0x135] = KEYCODE_KP_DIVIDE;
//    keycodes[0x11C] = KEYCODE_KP_ENTER;
//    keycodes[0x037] = KEYCODE_KP_MULTIPLY;
//    keycodes[0x04A] = KEYCODE_KP_SUBTRACT;
}

void win32_restore_console() {
    bool win32_console_utf8 = true;
    if (win32_console_utf8) {
        SetConsoleOutputCP(wnd.orig_codepage);
    }
}

void win32_init_dpi() {

    typedef BOOL(WINAPI
    *SETPROCESSDPIAWARE_T)(void);
    typedef HRESULT(WINAPI
    *SETPROCESSDPIAWARENESS_T)(PROCESS_DPI_AWARENESS);
    typedef HRESULT(WINAPI
    *GETDPIFORMONITOR_T)(HMONITOR, MONITOR_DPI_TYPE, UINT *, UINT *);

    SETPROCESSDPIAWARE_T fn_setprocessdpiaware = 0;
    SETPROCESSDPIAWARENESS_T fn_setprocessdpiawareness = 0;
    GETDPIFORMONITOR_T fn_getdpiformonitor = 0;
    HINSTANCE user32 = LoadLibraryA("user32.dll");
    if (user32) {
        fn_setprocessdpiaware = (SETPROCESSDPIAWARE_T)(void * )
        GetProcAddress(user32, "SetProcessDPIAware");
    }
    HINSTANCE shcore = LoadLibraryA("shcore.dll");
    if (shcore) {
        fn_setprocessdpiawareness = (SETPROCESSDPIAWARENESS_T)(void * )
        GetProcAddress(shcore, "SetProcessDpiAwareness");
        fn_getdpiformonitor = (GETDPIFORMONITOR_T)(void * )
        GetProcAddress(shcore, "GetDpiForMonitor");
    }
    if (fn_setprocessdpiawareness) {
        /* if the app didn't request HighDPI rendering, let Windows do the upscaling */
        PROCESS_DPI_AWARENESS process_dpi_awareness = PROCESS_SYSTEM_DPI_AWARE;
        wnd.dpi.aware = true;
        if (!cfgAllowHighDPI) {
            process_dpi_awareness = PROCESS_DPI_UNAWARE;
            wnd.dpi.aware = false;
        }
        fn_setprocessdpiawareness(process_dpi_awareness);
    } else if (fn_setprocessdpiaware) {
        fn_setprocessdpiaware();
        wnd.dpi.aware = true;
    }
    /* get dpi scale factor for main monitor */
    if (fn_getdpiformonitor && wnd.dpi.aware) {
        POINT pt = {1, 1};
        HMONITOR hm = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
        UINT dpix, dpiy;
        HRESULT hr = fn_getdpiformonitor(hm, MDT_EFFECTIVE_DPI, &dpix, &dpiy);
        (void) (hr);
        EK_ASSERT(SUCCEEDED(hr));
        /* clamp window scale to an integer factor */
        wnd.dpi.window_scale = (float) dpix / 96.0f;
    } else {
        wnd.dpi.window_scale = 1.0f;
    }
    if (cfgAllowHighDPI) {
        wnd.dpi.content_scale = wnd.dpi.window_scale;
        wnd.dpi.mouse_scale = 1.0f;
    } else {
        wnd.dpi.content_scale = 1.0f;
        wnd.dpi.mouse_scale = 1.0f / wnd.dpi.window_scale;
    }
    g_app.content_scale = wnd.dpi.content_scale;
    if (user32) {
        FreeLibrary(user32);
    }
    if (shcore) {
        FreeLibrary(shcore);
    }
}

/* updates current window and framebuffer size from the window's client rect, returns true if size has changed */
bool win32_update_dimensions() {
    RECT rect;
    if (GetClientRect(wnd.hwnd, &rect)) {
        g_app.window_size.x = (int) ((float) (rect.right - rect.left) / wnd.dpi.window_scale);
        g_app.window_size.y = (int) ((float) (rect.bottom - rect.top) / wnd.dpi.window_scale);
        const int fb_width = (int) ((float) g_app.window_size.x * wnd.dpi.content_scale);
        const int fb_height = (int) ((float) g_app.window_size.y * wnd.dpi.content_scale);
        if ((fb_width != (int) g_app.drawable_size.x) || (fb_height != (int) g_app.drawable_size.y)) {
            g_app.drawable_size.x = fb_width;
            g_app.drawable_size.y = fb_height;
            /* prevent a framebuffer size of 0 when window is minimized */
            if (g_app.drawable_size.x < 1) {
                g_app.drawable_size.x = 1;
            }
            if (g_app.drawable_size.y < 0) {
                g_app.drawable_size.y = 1;
            }
            return true;
        }
    } else {
        g_app.window_size.x = 1;
        g_app.window_size.y = 1;
        g_app.drawable_size.x = 1;
        g_app.drawable_size.y = 1;
    }
    return false;
}

LRESULT CALLBACK win32_wndproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (!wnd.in_create_window) {
        switch (uMsg) {
            case WM_CLOSE:
                // only give user a chance to intervene when sapp_quit() wasn't already called
                if (!g_app.require_exit) {
                    // if window should be closed and event handling is enabled, give user code
                    // a change to intervene via sapp_cancel_quit()
                    g_app.require_exit = true;
                    win32_uwp_app_event(event_type::app_close);
                    // if user code hasn't intervened, quit the app
                    if (g_app.require_exit) {
                        // exit ordered
                        g_app.require_exit = true;
                    }
                }
                if (g_app.require_exit /* exit ordered */) {
                    PostQuitMessage(0);
                }
                return 0;
            case WM_SYSCOMMAND:
                switch (wParam & 0xFFF0) {
                    case SC_SCREENSAVE:
                    case SC_MONITORPOWER:
                        // TODO:
//if (g_app.fullscreen) {
///* disable screen saver and blanking in fullscreen mode */
//return 0;
//}
                        break;
                    case SC_KEYMENU:
/* user trying to access menu via ALT */
                        return 0;
                }
                break;
            case WM_ERASEBKGND:
                return 1;
            case WM_SIZE: {
                const bool iconified = wParam == SIZE_MINIMIZED;
                if (iconified != wnd.iconified) {
                    wnd.iconified = iconified;
                    if (iconified) {
//win32_uwp_app_event(SAPP_EVENTTYPE_ICONIFIED);
                    } else {
//win32_uwp_app_event(SAPP_EVENTTYPE_RESTORED);
                    }
                }
            }
                break;
            case WM_SETCURSOR:
//                if (_sapp.desc.user_cursor) {
//                    if (LOWORD(lParam) == HTCLIENT) {
//win32_uwp_app_event(SAPP_EVENTTYPE_UPDATE_CURSOR);
//                        return 1;
//                    }
//                }
                break;
            case WM_LBUTTONDOWN:
//_sapp_win32_mouse_event(SAPP_EVENTTYPE_MOUSE_DOWN, SAPP_MOUSEBUTTON_LEFT);
//_sapp_win32_capture_mouse(1<<SAPP_MOUSEBUTTON_LEFT);
                break;
            case WM_RBUTTONDOWN:
//_sapp_win32_mouse_event(SAPP_EVENTTYPE_MOUSE_DOWN, SAPP_MOUSEBUTTON_RIGHT);
//_sapp_win32_capture_mouse(1<<SAPP_MOUSEBUTTON_RIGHT);
                break;
            case WM_MBUTTONDOWN:
//_sapp_win32_mouse_event(SAPP_EVENTTYPE_MOUSE_DOWN, SAPP_MOUSEBUTTON_MIDDLE);
//_sapp_win32_capture_mouse(1<<SAPP_MOUSEBUTTON_MIDDLE);
                break;
            case WM_LBUTTONUP:
//_sapp_win32_mouse_event(SAPP_EVENTTYPE_MOUSE_UP, SAPP_MOUSEBUTTON_LEFT);
//_sapp_win32_release_mouse(1<<SAPP_MOUSEBUTTON_LEFT);
                break;
            case WM_RBUTTONUP:
//_sapp_win32_mouse_event(SAPP_EVENTTYPE_MOUSE_UP, SAPP_MOUSEBUTTON_RIGHT);
//_sapp_win32_release_mouse(1<<SAPP_MOUSEBUTTON_RIGHT);
                break;
            case WM_MBUTTONUP:
//_sapp_win32_mouse_event(SAPP_EVENTTYPE_MOUSE_UP, SAPP_MOUSEBUTTON_MIDDLE);
//_sapp_win32_release_mouse(1<<SAPP_MOUSEBUTTON_MIDDLE);
                break;
            case WM_MOUSEMOVE:
//if (!_sapp.mouse.locked) {
//const float new_x  = (float)GET_X_LPARAM(lParam) * _sapp.win32.dpi.mouse_scale;
//const float new_y = (float)GET_Y_LPARAM(lParam) * _sapp.win32.dpi.mouse_scale;
///* don't update dx/dy in the very first event */
//if (_sapp.mouse.pos_valid) {
//_sapp.mouse.dx = new_x - _sapp.mouse.x;
//_sapp.mouse.dy = new_y - _sapp.mouse.y;
//}
//_sapp.mouse.x = new_x;
//_sapp.mouse.y = new_y;
//_sapp.mouse.pos_valid = true;
//if (!_sapp.win32.mouse_tracked) {
//_sapp.win32.mouse_tracked = true;
//TRACKMOUSEEVENT tme;
//memset(&tme, 0, sizeof(tme));
//tme.cbSize = sizeof(tme);
//tme.dwFlags = TME_LEAVE;
//tme.hwndTrack = _sapp.win32.hwnd;
//TrackMouseEvent(&tme);
//_sapp_win32_mouse_event(SAPP_EVENTTYPE_MOUSE_ENTER, SAPP_MOUSEBUTTON_INVALID);
//}
//_sapp_win32_mouse_event(SAPP_EVENTTYPE_MOUSE_MOVE, SAPP_MOUSEBUTTON_INVALID);
//}
                break;
            case WM_INPUT:
/* raw mouse input during mouse-lock */
//                if (_sapp.mouse.locked) {
//                    HRAWINPUT ri = (HRAWINPUT) lParam;
//                    UINT size = sizeof(_sapp.win32.raw_input_data);
// see: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getrawinputdata
//                    if ((UINT) - 1 ==
//                        GetRawInputData(ri, RID_INPUT, &_sapp.win32.raw_input_data, &size, sizeof(RAWINPUTHEADER))) {
//                        SOKOL_LOG("GetRawInputData() failed\n");
//                        break;
//                    }
//                    const RAWINPUT* raw_mouse_data = (const RAWINPUT*) &_sapp.win32.raw_input_data;
//                    if (raw_mouse_data->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) {
/* mouse only reports absolute position
   NOTE: THIS IS UNTESTED, it's unclear from reading the
   Win32 RawInput docs under which circumstances absolute
   positions are sent.
//*/
//                        if (_sapp.win32.raw_input_mousepos_valid) {
//                            LONG new_x = raw_mouse_data->data.mouse.lLastX;
//                            LONG new_y = raw_mouse_data->data.mouse.lLastY;
//                            _sapp.mouse.dx = (float) (new_x - _sapp.win32.raw_input_mousepos_x);
//                            _sapp.mouse.dy = (float) (new_y - _sapp.win32.raw_input_mousepos_y);
//                            _sapp.win32.raw_input_mousepos_x = new_x;
//                            _sapp.win32.raw_input_mousepos_y = new_y;
//                            _sapp.win32.raw_input_mousepos_valid = true;
//                        }
//                    } else {
/* mouse reports movement delta (this seems to be the common case) */
//                        _sapp.mouse.dx = (float) raw_mouse_data->data.mouse.lLastX;
//                        _sapp.mouse.dy = (float) raw_mouse_data->data.mouse.lLastY;
//                    }
//                    _sapp_win32_mouse_event(SAPP_EVENTTYPE_MOUSE_MOVE, SAPP_MOUSEBUTTON_INVALID);
                }
                break;

            case WM_MOUSELEAVE:
//                if (!_sapp.mouse.locked) {
//                    _sapp.win32.mouse_tracked = false;
//                    _sapp_win32_mouse_event(SAPP_EVENTTYPE_MOUSE_LEAVE, SAPP_MOUSEBUTTON_INVALID);
//                }
                break;
            case WM_MOUSEWHEEL:
//                _sapp_win32_scroll_event(0.0f, (float) ((SHORT) HIWORD(wParam)));
                break;
            case WM_MOUSEHWHEEL:
                //_sapp_win32_scroll_event((float) ((SHORT) HIWORD(wParam)), 0.0f);
                break;
            case WM_CHAR:
                //_sapp_win32_char_event((uint32_t) wParam, !!(lParam & 0x40000000));
                break;
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
                //_sapp_win32_key_event(SAPP_EVENTTYPE_KEY_DOWN, (int) (HIWORD(lParam) & 0x1FF), !!(lParam & 0x40000000));
                break;
            case WM_KEYUP:
            case WM_SYSKEYUP:
                //_sapp_win32_key_event(SAPP_EVENTTYPE_KEY_UP, (int) (HIWORD(lParam) & 0x1FF), false);
                break;
            case WM_ENTERSIZEMOVE:
                SetTimer(wnd.hwnd, 1, USER_TIMER_MINIMUM, NULL);
                break;
            case WM_EXITSIZEMOVE:
                KillTimer(wnd.hwnd, 1);
                break;
            case WM_TIMER:
                dispatch_draw_frame();
                d3d11_present();
/* NOTE: resizing the swap-chain during resize leads to a substantial
   memory spike (hundreds of megabytes for a few seconds).
if (_sapp_win32_update_dimensions()) {
    #if defined(SOKOL_D3D11)
    _sapp_d3d11_resize_default_render_target();
    #endif
    _sapp_win32_uwp_app_event(SAPP_EVENTTYPE_RESIZED);
}
*/
                break;
            case WM_DROPFILES:
                //win32_files_dropped((HDROP) wParam);
                break;
            default:
                break;
        }
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

void win32_create_window() {
    WNDCLASSW wndclassw;
    memset(&wndclassw, 0, sizeof(wndclassw));
    wndclassw.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclassw.lpfnWndProc = (WNDPROC) win32_wndproc;
    wndclassw.hInstance = GetModuleHandleW(NULL);
    wndclassw.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclassw.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wndclassw.lpszClassName = L"EKAPP";
    RegisterClassW(&wndclassw);

    DWORD win_style;
    const DWORD win_ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    RECT rect = {0, 0, 0, 0};
//    if (_sapp.fullscreen) {
//        win_style = WS_POPUP | WS_SYSMENU | WS_VISIBLE;
//        rect.right = GetSystemMetrics(SM_CXSCREEN);
//        rect.bottom = GetSystemMetrics(SM_CYSCREEN);
//    } else {
        win_style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX |
                    WS_SIZEBOX;
        rect.right = (int) ((float) g_app.window_cfg.size.x * wnd.dpi.window_scale);
        rect.bottom = (int) ((float) g_app.window_cfg.size.x * wnd.dpi.window_scale);
//    }
    AdjustWindowRectEx(&rect, win_style, FALSE, win_ex_style);
    const int win_width = rect.right - rect.left;
    const int win_height = rect.bottom - rect.top;
    wnd.in_create_window = true;

    static wchar_t wide_title[128];
    win32_uwp_utf8_to_wide(g_app.window_cfg.title.c_str(), wide_title, sizeof(wide_title));

    wnd.hwnd = CreateWindowExW(
            win_ex_style,               /* dwExStyle */
            L"EKAPP",                   /* lpClassName */
            wide_title,                 /* lpWindowName */
            win_style,                  /* dwStyle */
            CW_USEDEFAULT,              /* X */
            CW_USEDEFAULT,              /* Y */
            win_width,                  /* nWidth */
            win_height,                 /* nHeight */
            NULL,                       /* hWndParent */
            NULL,                       /* hMenu */
            GetModuleHandle(NULL),      /* hInstance */
            NULL);                      /* lParam */
    ShowWindow(wnd.hwnd, SW_SHOW);
    wnd.in_create_window = false;
    wnd.dc = GetDC(wnd.hwnd);
    EK_ASSERT(wnd.dc);
    win32_update_dimensions();

    DragAcceptFiles(wnd.hwnd, 1);
}

void win32_destroy_window() {
    DestroyWindow(wnd.hwnd);
    wnd.hwnd = 0;
    UnregisterClassW(L"SOKOLAPP", GetModuleHandleW(NULL));
}

void win32_create() {
    init_console();
    wnd.is_win10_or_greater = win32_is_win10_or_greater();
    win32_uwp_init_keytable();
    win32_init_dpi();
    win32_create_window();
    d3d11_create_device_and_swapchain();
    d3d11_create_default_render_target();
}

void win32_run_loop() {
    bool done = false;
    while (!(done || g_app.require_exit)) {
        MSG msg;
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (WM_QUIT == msg.message) {
                done = true;
                continue;
            } else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        dispatch_draw_frame();
        d3d11_present();
        if (IsIconic(wnd.hwnd)) {
            Sleep((DWORD)(16 * 1/* swap_interval */));
        }
        /* check for window resized, this cannot happen in WM_SIZE as it explodes memory usage */
        if (win32_update_dimensions()) {
            d3d11_resize_default_render_target();
            win32_uwp_app_event(event_type::app_resize);
        }
        if (g_app.require_exit) {
            PostMessage(wnd.hwnd, WM_CLOSE, 0, 0);
        }
    }
}

void win32_shutdown() {
    d3d11_destroy_default_render_target();
    d3d11_destroy_device_and_swapchain();
    win32_destroy_window();
    win32_restore_console();
}

namespace ek {

void start_application() {
    win32_create();

    dispatch_init();
    dispatch_device_ready();

    win32_run_loop();
    win32_shutdown();
}

}

char** command_line_to_utf8_argv(LPWSTR w_command_line, int* o_argc) {
    int argc = 0;
    char** argv = 0;
    char* args;

    LPWSTR* w_argv = CommandLineToArgvW(w_command_line, &argc);
    if (w_argv == NULL) {
        EK_ERROR << "Win32: failed to parse command line";
        abort();
    } else {
        size_t size = wcslen(w_command_line) * 4;
        argv = (char**) calloc(1, ((size_t) argc + 1) * sizeof(char*) + size);
        EK_ASSERT(argv);
        args = (char*) &argv[argc + 1];
        int n;
        for (int i = 0; i < argc; ++i) {
            n = WideCharToMultiByte(CP_UTF8, 0, w_argv[i], -1, args, (int) size, NULL, NULL);
            if (n == 0) {
                EK_ERROR << "Win32: failed to convert all arguments to utf8";
                abort();
                break;
            }
            argv[i] = args;
            size -= (size_t) n;
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
char** argv_utf8 = win32_command_line_to_utf8_argv(GetCommandLineW(), &argc_utf8);

g_app.args = {argc_utf8, argv_utf8};
::ek::main();

free(argv_utf8);
return 0;
}