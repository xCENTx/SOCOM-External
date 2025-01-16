#pragma once
#include <DXWindow/DXWindow.h>
#include <SDK/SOCOM.h>
#include <chrono>
#include <thread>
using namespace std::chrono_literals;

class Menu
{
public:
	bool bRunning{ true };
	bool bShowMenu{ true };

public:
	void Draw();
	void MainMenu();
	void SHROUD();
	void HUD();
	DxWindow::SOverlay GetOverlay();
	void UpdateOverlayViewState(bool bState);

public:
	explicit Menu();
	~Menu() noexcept = default;

private:
	DxWindow::SOverlay elements;
	void RenderCache();
}; inline std::unique_ptr<Menu> g_Menu;

//	basic color defines
#define IM_COL32_RED ImColor(1.0f, 0.0f, 0.0f, 1.0f)
#define IM_COL32_BLUE ImColor(0.0f, 1.0f, 0.0f, 1.0f)
#define IM_COL32_GREEN ImColor(0.0f, 1.0f, 0.0f, 1.0f)
#define IM_COL32_YELLOW ImColor(1.0f, 1.0f, 0.0f, 1.0f)

class GUI
{
public:	//	widget
	static void TextCentered(const char* pText);
	static void Tooltip(const char* tip);

public:	//	canvas
	static void DrawText_(ImVec2 pos, ImColor color, const char* pText, float fontSize);
	static void DrawTextCentered(ImVec2 pos, ImColor color, const char* pText, float fontsize = 14.f);
	static void Line(ImVec2 posA, ImVec2 posB, ImColor color, float thickness = 1.0f);
	static void Circle(ImVec2 pos, ImColor color, float radius, float thickness = 1.0f, float segments = 64);
	static void CleanLine(ImVec2 posA, ImVec2 posB, ImColor color, float thickness = 1.0f);
	static void CleanCircle(ImVec2 pos, ImColor color, float radius, float thickness = 1.0f, float segments = 64);
};