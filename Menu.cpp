#include "Menu.h"

Menu::Menu()
{
    elements.bIsShown = bShowMenu;
    elements.Menu = std::bind(&Menu::Draw, this);
    elements.Shroud = std::bind(&Menu::SHROUD, this);
    elements.Hud = std::bind(&Menu::HUD, this);

    //  load game data
    g_SOCOM = std::make_unique<SOCOM>();

    //	Initialize d3d window
    g_dxWindow->Init();

    //  load configuration settings
}

void Menu::Draw()
{
    if (bShowMenu)
        MainMenu();
}

static bool bESP{ false };
static float mESPDist{ 100.f };
void Menu::MainMenu()
{
    static ImVec2 MenuSize = ImVec2(800, 600);
    ImVec2 ScreenSize = g_dxWindow->GetScreenSize();
    ImVec2 start = { ScreenSize.x / 2 - MenuSize.x / 2, ScreenSize.y / 2 - MenuSize.y / 2 };
    ImGui::SetNextWindowPos(start);
    ImGui::SetNextWindowSize(MenuSize);
    if (!ImGui::Begin("SOCOM", &bShowMenu, 96))
    {
        ImGui::End();
        return;
    }
    auto width = ImGui::GetContentRegionAvail().x;
    auto height = ImGui::GetContentRegionAvail().y;

    //  ESP
    ImGui::Checkbox("ESP", &bESP);
    if (bESP)
    {
        ImGui::SameLine();
        ImGui::SetCursorPosX(width * .25);
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::SliderFloat("##ESP_DISTANCE", &mESPDist, 0.0f, 100.f, "%.0f");
    }

    ImGui::SetCursorPosY(height - ImGui::GetTextLineHeightWithSpacing() * 2);
    if (ImGui::Button("EXIT", ImGui::GetContentRegionAvail()))
    {
        // disable any enabled patches

        //  shutdown
        this->bRunning = false;
    }

    ImGui::End();
}

void Menu::SHROUD()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(g_dxWindow->GetScreenSize());
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4());
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    if (!ImGui::Begin("##SHROUDWINDOW", (bool*)true, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs))
    {
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::End();
        return;
    }
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::End();
}

void Menu::HUD()
{
    ImVec2 draw_pos = g_dxWindow->GetCloneWindowPos();
    ImVec2 draw_size = g_dxWindow->GetCloneWindowSize();
    ImGui::SetNextWindowPos(draw_pos);
    ImGui::SetNextWindowSize(draw_size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.00f, 0.00f, 0.00f, 0.00f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.00f, 0.00f, 0.00f));
    if (!ImGui::Begin("##HUDWINDOW", (bool*)true, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs))
    {
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::End();
        return;
    }
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();


    ImGuiStyle style = ImGui::GetStyle();
    ImDrawList* pDraw = ImGui::GetWindowDrawList();
    ImRect screen_rect(draw_pos, draw_pos + draw_size);
    auto center = screen_rect.GetCenter();
    auto top_center = ImVec2({ center.x, screen_rect.Min.y });

    std::string text("SOCOM ESP - NIGHTFYRE");
    ImVec2 szText = ImGui::CalcTextSize(text.c_str());
    ImVec2 posText = ImVec2(top_center.x - (szText.x * .5f), top_center.y + style.WindowPadding.y);
    pDraw->AddRectFilled(posText, posText + szText, IM_COL32_BLACK, 1.f);
    pDraw->AddText(posText, IM_COL32_WHITE, text.c_str());


//
    RenderCache();

    ImGui::End();
}

DxWindow::SOverlay Menu::GetOverlay() { return elements; }

void Menu::UpdateOverlayViewState(bool bState) { elements.bIsShown = bState; }

void Menu::RenderCache()
{

    auto& cache = g_SOCOM->imCache;
    ImVec2 screen_pos = g_dxWindow->GetCloneWindowPos();
    ImVec2 screen_size = g_dxWindow->GetCloneWindowSize();
    ImRect screen_rect(screen_pos, screen_pos + screen_size);
    ImVec2 screen_center = screen_rect.GetCenter();
    ImDrawList* pDraw = ImGui::GetWindowDrawList();

    // 
    Engine::Vec2 szScreen = { screen_size.x , screen_size.y };

    for (auto& obj : cache.render)
    {
        auto ent_origin = obj.pos;
        auto distance = cache.cameraView.origin().Distance(ent_origin);

        Engine::Vec2 screen;
        Engine::Vec2 screen2;
        if (!Engine::Tools::ProjectWorldToScreen(ent_origin, cache.cameraView, 60.f, szScreen, &screen))   
            continue;
        Engine::Tools::WorldToScreen(ent_origin, cache.mvmatrix, szScreen, &screen2);

        ImVec2 pos = screen_pos + ImVec2(screen.x, screen.y);

        char buf_dist[16];
        sprintf_s(buf_dist, "[%.1fm]", distance);
        std::string nameDist(buf_dist);
        std::string nameEnt = obj.name;
        ImVec2 szTextDist = ImGui::CalcTextSize(nameDist.c_str());
        ImVec2 szTextName = ImGui::CalcTextSize(nameEnt.c_str());
        ImVec2 posTextName = ImVec2(pos.x - (szTextName.x * .5f), pos.y);
        ImVec2 posTextDist = ImVec2(pos.x - (szTextDist.x * .5f), pos.y + (szTextName.y * 1.5f));
        pDraw->AddText(posTextName, IM_COL32_WHITE, nameEnt.c_str());
        pDraw->AddText(posTextDist, IM_COL32_WHITE, nameDist.c_str());
    }
}