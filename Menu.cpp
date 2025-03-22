#include "Menu.h"

Menu::Menu()
{
    //  establish overlay elements
    elements.bIsShown = bShowMenu;
    elements.Menu = std::bind(&Menu::Draw, this);
    elements.Shroud = std::bind(&Menu::SHROUD, this);
    elements.Hud = std::bind(&Menu::HUD, this);

    bRunning = g_PSXMemory.bAttached;
}

Menu::~Menu()
{
    elements = DxWindow::SOverlay();
    bRunning = false;
}

ImRect Menu::GetImGuiMenuBounds()
{
    const ImVec2& posClone = g_dxWindow->GetCloneWindowPos();   //  get the position of the cloned application window
    const ImVec2& szClone = g_dxWindow->GetCloneWindowSize();   //  get the size of the cloned application window
    const ImVec2& halfClone = szClone * .5;                     //  half application window size

    //  Get Window Size
    ImVec2 szMenu(halfClone);           //  overlay imgui menu window size
    ImVec2 szMenuMax(800.f, 600.f);     //  max overlay imgui menu window size
    szMenu.x = (szMenu.x > szMenuMax.x) ? szMenuMax.x : szMenu.x;
    szMenu.y = (szMenu.y > szMenuMax.y) ? szMenuMax.y : szMenu.y;

    //  Get Window Position
    ImVec2 posMenu = posClone + halfClone - szMenu * .5;   //  overlay imgui menu window position
    return ImRect(posMenu, posMenu + szMenu);
}

ImRect Menu::GetCloneOverlayBounds()
{
    return ImRect(
        g_dxWindow->GetCloneWindowPos(),
        g_dxWindow->GetCloneWindowPos() + g_dxWindow->GetCloneWindowSize()
    );
}

ImRect Menu::GetClientScreenBounds()
{
    return ImRect(
        ImVec2(0, 0),
        ImVec2(g_dxWindow->GetScreenSize())
    );
}

void Menu::Draw()
{
    if (!bShowMenu)
        return;
    
    MainMenu();
}

void Menu::MainMenu()
{
    auto MenuRect = GetImGuiMenuBounds();
    ImGui::SetNextWindowPos(MenuRect.Min);
    ImGui::SetNextWindowSize(MenuRect.GetSize());
    if (!ImGui::Begin("SOCOM", &bShowMenu, 96))
    {
        ImGui::End();
        return;
    }
    auto width = ImGui::GetContentRegionAvail().x;
    auto height = ImGui::GetContentRegionAvail().y;

    //  ESP
    ImGui::Checkbox("ESP", &this->bESP);
    if (this->bESP)
    {
        ImGui::SameLine();
        ImGui::Checkbox("##names", &this->bESPName);
        GUI::Tooltip("NAMES");
        ImGui::SameLine();
        ImGui::Checkbox("##snap_lines", &this->bESPSnap);
        GUI::Tooltip("SNAP LINES");
        ImGui::SameLine();
        ImGui::Checkbox("##box_2D", &this->bESPBox2D);
        GUI::Tooltip("2D BOX");
        ImGui::SameLine();
        ImGui::Checkbox("##box_health", &this->bESPHealth);
        GUI::Tooltip("HEALTH");

        ImGui::SameLine();
        ImGui::SetCursorPosX(width * .5);
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::SliderFloat("##ESP_DISTANCE", &mESPDist, 0.0f, 100.f, "%.0f");
    }
    
    if (ImGui::Button("REFILL AMMO"))
    {
		Engine::zdb::Patches::RefillAllAmmo();
    }
    
    if (ImGui::Button("FORCE COMPLETE MISSION"))
    {
        Engine::zdb::Patches::ForceCompleteMission();
    }

    if (ImGui::Button("GIVE M79"))
    {
        Engine::zdb::Patches::SetWeapon(
            Engine::zdb::Enums::EWeaponIndex::EWeaponIndex_Secondary, // weapon slot
            Engine::zdb::Enums::EWeapon::EWeapon_P_GL_M79 	    // weapon to give
        );
        Engine::zdb::Patches::SetWeapon(
            Engine::zdb::Enums::EWeaponIndex::EWeaponIndex_EqSlot3, // weapon slot
            Engine::zdb::Enums::EWeapon::EWeapon_EQ_M79HE 	    // weapon to give
        );

        Engine::zdb::Patches::SetWeaponAmmoType(
            Engine::zdb::Enums::EWeaponIndex::EWeaponIndex_Secondary, // weapon slot
            Engine::zdb::Enums::EWeaponAmmo::EWeaponAmmo_EMPTY
        );

        Engine::zdb::Patches::SetWeaponAmmoType(
            Engine::zdb::Enums::EWeaponIndex::EWeaponIndex_EqSlot3, // weapon slot
            Engine::zdb::Enums::EWeaponAmmo::EWeaponAmmo_M79_HE
        );
    }
    
    if (ImGui::Button("GIVE EXPLOSIVE BULLETS"))
    {
		Engine::zdb::Patches::SetWeaponAmmoType(
			Engine::zdb::Enums::EWeaponIndex::EWeaponIndex_Primary, // weapon slot
			Engine::zdb::Enums::EWeaponAmmo::EWeaponAmmo_Satchel                // ammo type
		);
    }

    if (ImGui::Button("SET CUSTOM AMMO"))
    {
		Engine::zdb::Classes::CZAmmo ammo;
		ammo.bulletImpactDmg = 1337.f;
		ammo.stun = 1337.f;
		ammo.piercing = 1337.f;
		ammo.explosionDamage = 0.f;
		ammo.explosionRadius = 0.f;

        Engine::zdb::Patches::SetAmmoProperties(
            Engine::zdb::Enums::EWeaponIndex::EWeaponIndex_Primary, // weapon slot
            ammo
        );
    }

    ImGui::SetCursorPosY(height - ImGui::GetTextLineHeightWithSpacing() * 2);
    if (ImGui::Button("EXIT", ImGui::GetContentRegionAvail()))
    {
        //  shutdown
        this->bRunning = false;
    }

    ImGui::End();
}

void Menu::SHROUD()
{
    const ImRect& wndw = GetClientScreenBounds();
    ImGui::SetNextWindowPos(wndw.Min);
    ImGui::SetNextWindowSize(wndw.GetSize());
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
    const ImRect& wndw = GetCloneOverlayBounds();
    ImGui::SetNextWindowPos(wndw.Min);
    ImGui::SetNextWindowSize(wndw.GetSize());
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
    auto center = wndw.GetCenter();
    auto top_center = ImVec2({ center.x, wndw.Min.y });

#if !_DEBUG
    GUI::DrawBGTextCentered(top_center, IM_COL32_WHITE, "SOCOM ESP - NIGHTFYRE", IM_COL32_BLACK, 24.0f);
#endif

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
        if (!obj.bAlive)
            continue;

        auto ent_origin = obj.pos;
        auto distance = cache.cameraView.origin().Distance(ent_origin);
        if (distance > this->mESPDist)
            continue;

        //  calc head pos
        auto ent_headOrigin = ent_origin;
		obj.stance == 0 ? ent_headOrigin.y += 20.f : obj.stance == 1 ? ent_headOrigin.y += 14.f : ent_headOrigin.y += 3.0f;

        float mFOV = 60.f / cache.localPlayer.seal.ZoomLevel;

        Engine::Vec2 screen;
        Engine::Vec2 screenHead;
        if (!Engine::zdb::Tools::ProjectWorldToScreen(ent_origin, cache.cameraView, mFOV, szScreen, &screen)
            || !Engine::zdb::Tools::ProjectWorldToScreen(ent_headOrigin, cache.cameraView, mFOV, szScreen, &screenHead))
            continue;

        ImVec2 pos = screen_pos + ImVec2(screen.x, screen.y);
        ImVec2 head_pos = screen_pos + ImVec2(screenHead.x, screenHead.y);
        float corner_height = abs(head_pos.y - pos.y);		                                    //	Width
        float corner_width = corner_height * 0.65;			                                    //	Height
        ImVec2 pos_box(head_pos.x - (corner_width / 2), head_pos.y);	                        //	Top Left Corner
        ImRect bbox(pos_box, ImVec2(pos_box.x + corner_width, pos_box.y + corner_height));      //  2d bounding box


        //  NAME
        if (this->bESPName)
        {
            char buf_dist[16];
            sprintf_s(buf_dist, "[%.0fm]", distance);
            std::string nameDist(buf_dist);
            std::string nameEnt = obj.name;
            ImVec2 szTextDist = ImGui::CalcTextSize(nameDist.c_str());
            ImVec2 szTextName = ImGui::CalcTextSize(nameEnt.c_str());
            ImVec2 posTextName = ImVec2(pos.x - (szTextName.x * .5f), pos.y);
            ImVec2 posTextDist = ImVec2(pos.x - (szTextDist.x * .5f), pos.y + (szTextName.y * 1.5f));
            GUI::DrawBorderText(posTextName, IM_COL32_WHITE, nameEnt, IM_COL32_WHITE);
            GUI::DrawText_(posTextDist, IM_COL32_WHITE, nameDist);
        }

        //  SNAP
        if (this->bESPSnap)
            GUI::CleanLine(pos, screen_pos, IM_COL32_WHITE);
        
        // skip drawing certain visuals if prone
        if (obj.stance >= 2)
            continue;

        //  BOX
        if (this->bESPBox2D) // draw 2d box but skip prone players
			pDraw->AddRect(bbox.Min, bbox.Max, IM_COL32_WHITE, 0.f, 0, 1.f);

        //	HEALTH
        if (this->bESPHealth)
        {
            ImColor mColHealth(255 - obj.health * 2.55, obj.health * 2.55, 0);					//	health color
            float heightBarHP = bbox.GetHeight() - (bbox.GetHeight() * (obj.health / 100.f));	//	health bar height
            auto lBar = ImRect(bbox.Min, ImVec2(bbox.Min.x + 1.f, bbox.Max.y));					//	left healthbar
            pDraw->AddRect(lBar.Min, lBar.Max, IM_COL32_WHITE);
            pDraw->AddRect(ImVec2(lBar.Min.x, lBar.Min.y + heightBarHP), lBar.Max, mColHealth);
        }
    }
}

void GUI::TextCentered(const char* pText)
{
    ImVec2 textSize = ImGui::CalcTextSize(pText);
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 textPos = ImVec2((windowSize.x - textSize.x) * 0.5f, (windowSize.y - textSize.y) * 0.5f);
    ImGui::SetCursorPos(textPos);
    ImGui::Text("%s", pText);
}

void GUI::Tooltip(const char* tip)
{
    if (!ImGui::IsItemHovered())
        return;

    ImGui::SetTooltip(tip);
}

void GUI::DrawText_(const ImVec2& pos, const ImColor& color, const std::string& text, const float& szFont)
{
    ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), szFont, pos, color, text.c_str(), text.c_str() + text.size(), 800.f, nullptr);
}

void GUI::DrawBGText(const ImVec2& pos, const ImColor& color, const std::string& text, const ImColor& background, const float& szFont)
{
    auto pFont = ImGui::GetFont();
    const ImVec2& textSize = ImGui::CalcTextSize(text.c_str());
    ImRect textRect = ImRect(pos, pos + textSize);
    if (szFont > 0.f)
    {
        const ImVec2& scaledTextSize = ImVec2(textSize.x * szFont / pFont->FontSize, szFont);
        ImVec2 scaledTextPos = ImVec2(pos.x - (scaledTextSize.x * .5f), pos.y);
    }
    ImGui::GetWindowDrawList()->AddRectFilled(textRect.Min, textRect.Max, background);
    DrawText_(textRect.Min, color, text, szFont);
}

void GUI::DrawBorderText(const ImVec2& pos, const ImColor& color, const std::string& text, const ImColor& border, const float& szFont)
{
    auto pFont = ImGui::GetFont();
    const ImVec2& textSize = ImGui::CalcTextSize(text.c_str());
    ImRect textRect = ImRect(pos, pos + textSize);
    if (szFont > 0.f)
    {
        const ImVec2& scaledTextSize = ImVec2(textSize.x * szFont / pFont->FontSize, szFont);
        ImVec2 scaledTextPos = ImVec2(pos.x - (scaledTextSize.x * .5f), pos.y);
		textRect = (ImRect(scaledTextPos, scaledTextPos + scaledTextSize));
    }
    ImGui::GetWindowDrawList()->AddRect(textRect.Min, textRect.Max, border);
    DrawText_(textRect.Min, color, text, szFont);
}

void GUI::DrawTextCentered(const ImVec2& pos, const ImColor& color, const std::string& text, const float& szFont)
{
    const ImVec2& textSize = ImGui::CalcTextSize(text.c_str());
    ImVec2 textPosition = ImVec2(pos.x - (textSize.x * 0.5f), pos.y);
    if (szFont <= 0.f)
    {
        DrawText_(textPosition, color, text, szFont);
        return;
    }

    auto pFont = ImGui::GetFont();
    ImVec2 scaledTextSize = ImVec2(textSize.x * szFont / pFont->FontSize, szFont);
    ImVec2 scaledTextPos = ImVec2(pos.x - (scaledTextSize.x * .5f), pos.y);
    DrawText_(scaledTextPos, color, text, szFont);
}

void GUI::DrawBGTextCentered(const ImVec2& pos, const ImColor& color, const std::string& text, const ImColor& background, const float& szFont)
{
    const ImVec2& textSize = ImGui::CalcTextSize(text.c_str());
    ImVec2 textPosition = ImVec2(pos.x - (textSize.x * 0.5f), pos.y);
    if (szFont <= 0.f)
    {
        DrawBGText(textPosition, color, text, background, szFont);
        return;
    }

    auto pFont = ImGui::GetFont();
    ImVec2 scaledTextSize = ImVec2(textSize.x * szFont / pFont->FontSize, szFont);
    ImVec2 scaledTextPos = ImVec2(pos.x - (scaledTextSize.x * .5f), pos.y);
    ImGui::GetWindowDrawList()->AddRectFilled(scaledTextPos, scaledTextPos + scaledTextSize, background);
    DrawText_(scaledTextPos, color, text, szFont);
}

void GUI::DrawBorderTextCentered(const ImVec2& pos, const ImColor& color, const std::string& text, const ImColor& border, const float& szFont)
{
    const ImVec2& textSize = ImGui::CalcTextSize(text.c_str());
    ImVec2 textPosition = ImVec2(pos.x - (textSize.x * 0.5f), pos.y);
    if (szFont <= 0.f)
    {
        DrawBorderText(textPosition, color, text, border, szFont);
        return;
    }

    auto pFont = ImGui::GetFont();
    ImVec2 scaledTextSize = ImVec2(textSize.x * szFont / pFont->FontSize, szFont);
    ImVec2 scaledTextPos = ImVec2(pos.x - (scaledTextSize.x * .5f), pos.y);
    ImGui::GetWindowDrawList()->AddRect(scaledTextPos, scaledTextPos + scaledTextSize, border);
    DrawText_(scaledTextPos, color, text, szFont);
}

void GUI::Line(const ImVec2& posA, const ImVec2& posB, const ImColor& color, const float& thickness)
{
    ImGui::GetWindowDrawList()->AddLine(posA, posB, color, thickness);
}

void GUI::Circle(const ImVec2& pos, const ImColor& color, const float& radius, const float& thickness, const float& segments)
{
    ImGui::GetWindowDrawList()->AddCircle(pos, radius, color, segments, thickness);
}

void GUI::CleanLine(const ImVec2& posA, const ImVec2& posB, const ImColor& color, const float& thickness)
{
    Line(posA, posB, ImColor(0.0f, 0.0f, 0.0f, color.Value.w), (thickness + 0.25));
    Line(posA, posB, ImColor(1.0f, 1.0f, 1.0f, color.Value.w), (thickness + 0.15));
    Line(posA, posB, color, thickness);
}

void GUI::CleanCircle(const ImVec2& pos, const ImColor& color, const float& radius, const float& thickness, const float& segments)
{
    Circle(pos, ImColor(0.0f, 0.0f, 0.0f, color.Value.w), radius, thickness, segments);
    Circle(pos, ImColor(1.0f, 1.0f, 1.0f, color.Value.w), radius, thickness, segments);
    Circle(pos, color, radius, thickness, segments);
}