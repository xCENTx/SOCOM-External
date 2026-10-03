// https://github.com/NightFyre/DX11-ImGui-External-Base/tree/main
#include "DxWindow.h"
#include <dwmapi.h>

DxWindow::DxWindow() { }

DxWindow::~DxWindow() { }

MARGINS gMargin;
void DxWindow::Init()
{
    m_szScreen = ImVec2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    m_wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"WC NightFyre Dx11 External Base", nullptr };
    ::RegisterClassEx(&m_wc);
    //  m_hwnd = ::CreateWindowW(m_wc.lpszClassName, L"NightFyre Dx11 External Base", WS_EX_TOPMOST | WS_POPUP, static_cast<int>(m_posScreen.x), static_cast<int>(m_posScreen.y), static_cast<int>(m_szScreen.x), static_cast<int>(m_szScreen.y), nullptr, nullptr, m_wc.hInstance, nullptr);
    m_hwnd = ::CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED,
        m_wc.lpszClassName,
        L"NightFyre Dx11 External Base",
        WS_POPUP,
        static_cast<int>(m_posScreen.x),
        static_cast<int>(m_posScreen.y),
        static_cast<int>(m_szScreen.x),
        static_cast<int>(m_szScreen.y),
        nullptr,
        nullptr,
        m_wc.hInstance,
        this
    );
    SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);
    gMargin = { 0, 0, static_cast<int>(m_szScreen.x), static_cast<int>(m_szScreen.y) };
    DwmExtendFrameIntoClientArea(m_hwnd, &gMargin);

    if (!CreateDeviceD3D(m_hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
        return;
    }

    ::ShowWindow(m_hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(m_hwnd);
    SetWindowLong(m_hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;        // Enable Gamepad Controls
    io.IniFilename = NULL;                                      // Disable Ini File

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowTitleAlign = ImVec2(.5f, .5f);                  // Center Align Window Title

    ImGui_ImplWin32_Init(m_hwnd);
    ImGui_ImplDX11_Init(m_pd3dDevice, m_pd3dDeviceContext);
}

void DxWindow::Shutdown()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    DestroyWindow(m_hwnd);
    UnregisterClass(m_wc.lpszClassName, m_wc.hInstance);
}

bool DxWindow::CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice, &featureLevel, &m_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();

    return true;
}

void DxWindow::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_mainRenderTargetView);
    pBackBuffer->Release();
}

void DxWindow::CleanupDeviceD3D()
{
    CleanupRenderTarget();

    if (m_pSwapChain)
    {
        m_pSwapChain->Release();
        m_pSwapChain = nullptr;
    }

    if (m_pd3dDeviceContext)
    {
        m_pd3dDeviceContext->Release();
        m_pd3dDeviceContext = nullptr;
    }

    if (m_pd3dDevice)
    {
        m_pd3dDevice->Release();
        m_pd3dDevice = nullptr;
    }
}

void DxWindow::CleanupRenderTarget()
{
    if (m_mainRenderTargetView)
    {
        m_mainRenderTargetView->Release();
        m_mainRenderTargetView = nullptr;
    }
}

void DxWindow::SetWindowStyle(LONG flags) { SetWindowLong(m_hwnd, GWL_EXSTYLE, flags); }

void DxWindow::ClickThrough(bool bIsClickThrough)
{
    LONG flags = WS_EX_LAYERED;

    if (bIsClickThrough)
        flags = WS_EX_LAYERED | WS_EX_TRANSPARENT;

    SetWindowStyle(flags);
}

void DxWindow::SetWindowFocus(HWND window)
{
    SetForegroundWindow(window);
    SetActiveWindow(window);
}

ImVec2 DxWindow::GetScreenSize() { return m_szScreen; }

ImVec2 DxWindow::GetCloneWindowSize() { return m_szClone; }

ImVec2 DxWindow::GetCloneWindowPos() { return m_posClone; }

HWND DxWindow::GetWindowHandle() { return m_hwnd; }

ID3D11Device* DxWindow::GetD3DDevice() { return m_pd3dDevice; }

IDXGISwapChain* DxWindow::GetSwapChain() { return m_pSwapChain; }

ID3D11DeviceContext* DxWindow::GetDeviceContext() { return m_pd3dDeviceContext; }

ID3D11RenderTargetView* DxWindow::GetRTV() { return m_mainRenderTargetView; }

void DxWindow::Update(SOverlay bind)
{
    static float clearColor[4] = { 0.0f,0.0f,0.0f,0.0f };

    MSG msg;
    while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    if (m_pendingWidth != 0 && m_pendingHeight != 0 && m_pSwapChain)
    {
        CleanupRenderTarget();

        m_pSwapChain->ResizeBuffers(
            0,
            m_pendingWidth,
            m_pendingHeight,
            DXGI_FORMAT_UNKNOWN,
            0
        );

        m_pendingWidth = 0;
        m_pendingHeight = 0;

        CreateRenderTarget();
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (bind.bIsShown)
    {
        bind.Shroud();
        bind.Menu();
    }
    else
        bind.Hud();

    ClickThrough(!bind.bIsShown);

    ImGui::Render();
    m_pd3dDeviceContext->OMSetRenderTargets(1, &m_mainRenderTargetView, NULL);
    m_pd3dDeviceContext->ClearRenderTargetView(m_mainRenderTargetView, (float*)clearColor);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    m_pSwapChain->Present(0, 0);
}

void DxWindow::CloneUpdate(HWND window)
{
    if (!window || !IsWindow(window))
    {
		// clear cloned window position and size
		m_posClone = ImVec2(0.0f, 0.0f);
		m_szClone = ImVec2(0.0f, 0.0f);
        bValidClone &= false;
        return;
    }
		
    RECT clientRect;
    if (!GetClientRect(window, &clientRect))
    {
        bValidClone = false;
        return;
    }

    POINT clientPos
    {
        clientRect.left,
        clientRect.top
    };

    if (!ClientToScreen(window, &clientPos))
    {
        bValidClone = false;
        return;
    }

    const int width = clientRect.right - clientRect.left;
    const int height = clientRect.bottom - clientRect.top;

    if (width <= 0 || height <= 0)
    {
        bValidClone = false;
        return;
    }

    const ImVec2 newPos
    {
        static_cast<float>(clientPos.x),
        static_cast<float>(clientPos.y)
    };

    const ImVec2 newSize
    {
        static_cast<float>(width),
        static_cast<float>(height)
    };

    const bool changed =
        newPos.x != m_posClone.x ||
        newPos.y != m_posClone.y ||
        newSize.x != m_szClone.x ||
        newSize.y != m_szClone.y;

    m_posClone = newPos;
    m_szClone = newSize;
    bValidClone = true;

    if (!changed)
        return;

    SetWindowPos(m_hwnd, HWND_TOPMOST, static_cast<int>(m_posClone.x), static_cast<int>(m_posClone.y), static_cast<int>(m_szClone.x), static_cast<int>(m_szClone.y), SWP_NOACTIVATE);
}

LRESULT WINAPI DxWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    DxWindow* window = reinterpret_cast<DxWindow*>( GetWindowLongPtrW(hWnd, GWLP_USERDATA) );

    if (msg == WM_NCCREATE)
    {
        auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);

        window = static_cast<DxWindow*>(create->lpCreateParams);

        SetWindowLongPtrW(
            hWnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(window)
        );
    }

    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
        case WM_SIZE:
        {
            if (window && wParam != SIZE_MINIMIZED)
            {
                window->m_pendingWidth = LOWORD(lParam);
                window->m_pendingHeight = HIWORD(lParam);
            }

            return 0;
        }

        case WM_SYSCOMMAND:
        {
            if ((wParam & 0xFFF0) == SC_KEYMENU)
                return 0;

            break;
        }

        case WM_DESTROY:
        {
            ::PostQuitMessage(0);
            return 0;
        }
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}