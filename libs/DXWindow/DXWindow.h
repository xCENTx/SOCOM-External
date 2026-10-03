// https://github.com/NightFyre/DX11-ImGui-External-Base/tree/main

#pragma once
#include <functional>
#include <memory>

//	DirectX
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

//  ImGui
#define IMGUI_DISABLE_DEMO_WINDOWS                        // Disable demo windows: ShowDemoWindow()/ShowStyleEditor() will be empty.
#define IMGUI_DISABLE_DEBUG_TOOLS                         // Disable metrics/debugger and other debug tools: ShowMetricsWindow(), ShowDebugLogWindow() and ShowIDStackToolWindow() will be empty.
#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class DxWindow
{
public:
	using OverlayFunctionPtr = std::function<void()>;
	struct SOverlay
	{
		bool bIsShown{ false };

		OverlayFunctionPtr Menu;
		OverlayFunctionPtr Shroud;
		OverlayFunctionPtr Hud;
	};

public:
	explicit DxWindow();
	~DxWindow();

	bool Init();
	void Shutdown();
	void Tick(const SOverlay& bind);
	
public:
	void UpdateClone(HWND wndw);
	void ClickThrough(bool bIsClickThrough);
	void SetWindowFocus(HWND window);
	void FocusOverlay();
	void FocusTarget();

public:
	ImVec2 GetCloneWindowSize() const;
	ImVec2 GetCloneWindowPos() const;

public:
	HWND GetWindowHandle() const;

	ID3D11Device* GetD3DDevice() const;
	IDXGISwapChain* GetSwapChain() const;
	ID3D11DeviceContext* GetDeviceContext() const;
	ID3D11RenderTargetView* GetRTV() const;

private:
	bool CreateDeviceD3D(HWND hWnd);
	void CleanupDeviceD3D();
	bool CreateRenderTarget();
	void CleanupRenderTarget();

private:
	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	HWND m_hwnd{ nullptr };
	WNDCLASSEXW m_wc{ };

	ID3D11Device* m_pd3dDevice{ nullptr };
	IDXGISwapChain* m_pSwapChain{ nullptr };
	ID3D11DeviceContext* m_pd3dDeviceContext{ nullptr };
	ID3D11RenderTargetView* m_mainRenderTargetView{ nullptr };
	UINT m_pendingWidth{ 0 };
	UINT m_pendingHeight{ 0 };
	

	HWND m_hwndTarget{ nullptr };
	ImVec2 m_szClone{ 0.0f, 0.0f };
	ImVec2 m_posClone{ 0.0f, 0.0f };

	bool m_clickThrough{ false };
	bool m_ValidClone{ false };

}; inline std::unique_ptr<DxWindow> g_dxWindow;