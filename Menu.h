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

public:	//	visuals
	bool bESP{ false };
	bool bESPName{ false };
	bool bESPSnap{ false };
	bool bESPHealth{ false };
	bool bESPBox2D{ false };
	float mESPDist{ 100.f };

public:
	void Draw();
	void MainMenu();
	void SHROUD();
	void HUD();

public:
	DxWindow::SOverlay GetOverlay();
	void UpdateOverlayViewState(bool bState);

	/* obtains a bounding box for the overlay menu based on the dimensions of the cloned window
	* returns a menu size of half the cloned window size , with its position centered on the cloned window
	*/
	ImRect GetImGuiMenuBounds();

	/* obtains a bounding box for the game overlay window based on the dimensions of the cloned window
	* returns a window size of the cloned window size, with its position at the top left corner of the cloned window
	*/
	ImRect GetCloneOverlayBounds();

	/* */
	ImRect GetClientScreenBounds();

public:
	explicit Menu();
	~Menu() noexcept;

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
	static void DrawText_(const ImVec2& pos, const ImColor& color, const std::string& text, const float& szFont = 0.f);
	static void DrawBGText(const ImVec2& pos, const ImColor& color, const std::string& text, const ImColor& background, const float& szFont = 0.f);
	static void DrawBorderText(const ImVec2& pos, const ImColor& color, const std::string& text, const ImColor& border, const float& szFont = 0.f);
	static void DrawTextCentered(const ImVec2& pos, const ImColor& color, const std::string& text, const float& szFont = 0.f);
	static void DrawBGTextCentered(const ImVec2& pos, const ImColor& color, const std::string& text, const ImColor& background, const float& szFont = 0.f);
	static void DrawBorderTextCentered(const ImVec2& pos, const ImColor& color, const std::string& text, const ImColor& border, const float& szFont = 0.f);
	static void Line(const ImVec2& posA, const ImVec2& posB, const ImColor& color, const float& thickness = 1.0f);
	static void Circle(const ImVec2& pos, const ImColor& color, const float& radius, const float& thickness = 1.0f, const float& segments = 64);
	static void CleanLine(const ImVec2& posA, const ImVec2& posB, const ImColor& color, const float& thickness = 1.0f);
	static void CleanCircle(const ImVec2& pos, const ImColor& color, const float& radius, const float& thickness = 1.0f, const float& segments = 64);
};