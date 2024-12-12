#include "Menu.h"

Menu::Menu()
{
    elements.Hud = std::bind(&Menu::HUD, this);
}

void Menu::HUD()
{
    ImVec2 draw_pos{ 0.0f, 0.0f };
    ImVec2 draw_size{ g_dxWindow->GetScreenSize() };
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

    text = "PRESS [END] KEY TO EXIT";
    szText = ImGui::CalcTextSize(text.c_str());
    posText = ImVec2(screen_rect.Max.x - (szText.x + style.WindowPadding.x), screen_rect.Max.y - (szText.y + style.WindowPadding.y));
    pDraw->AddRectFilled(posText, posText + szText, IM_COL32_BLACK, 1.f);
    pDraw->AddText(posText, IM_COL32_WHITE, text.c_str());


//
    RenderCache();

    ImGui::End();
}

DxWindow::SOverlay Menu::GetOverlay() { return elements; }

void Menu::RenderCache()
{

    auto& cache = g_SOCOM->imCache;
    ImVec2 screen_pos{ 0.0f, 0.0f };
    ImVec2 screen_size{ g_dxWindow->GetScreenSize() };
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
        if (!Engine::Tools::ProjectWorldToScreen(ent_origin, cache.cameraView, 60.f, szScreen, &screen))   
            continue;

        ImVec2 pos(screen.x, screen.y);

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