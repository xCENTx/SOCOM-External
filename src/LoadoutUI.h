#pragma once

#include <SDK/SOCOM.h>

#include <string>
#include <vector>
#include <memory>
#include <filesystem>

class WeaponViewer;
struct ID3D11Device;
struct ID3D11DeviceContext;


class LoadoutUI
{
private:

    bool m_initialized{ false };
    std::unique_ptr<WeaponViewer> m_weaponViewer;
    bool m_modelCOMInitialized{ false };
    bool m_modelAttempted{ false };
    bool m_browseModels{ false };
    bool m_modelLowDetail{ false };
    int m_modelTextureMode{ 1 };
    std::string m_modelError;
    std::filesystem::path m_modelAssetPath;

    //
    // =========================================================================
    //  ENTRIES
    // =========================================================================
    //

    struct WeaponEntry
    {
        std::string name;
        std::string modelName;

        i32_t address{ 0 };

        Engine::zdb::Classes::CZWeapon weapon{};
    };

    struct AmmoEntry
    {
        std::string name;

        i32_t address{ 0 };

        Engine::zdb::Classes::CZAmmo ammo{};
    };


    //
    // =========================================================================
    //  INVENTORY
    // =========================================================================
    //

    struct InventorySlot
    {
        i32_t weaponAddress{ 0 };
        i32_t ammoAddress{ 0 };

        std::string weaponName;
        std::string ammoName;
    };


    //
    // =========================================================================
    //  LOADOUT SNAPSHOT
    // =========================================================================
    //

    struct LoadoutSnapshot
    {
        bool valid{ false };

        i32_t weapon[5]{};
        i32_t ammo[5]{};
    };


    //
    // =========================================================================
    //  PRESETS
    // =========================================================================
    //

    struct LoadoutPresetSlot
    {
        i32_t weapon{ 0 };
        i32_t ammo{ 0 };
    };

    struct LoadoutPreset
    {
        const char* name{ nullptr };
        const char* description{ nullptr };

        LoadoutPresetSlot slots[5]{};
    };


    //
    // =========================================================================
    //  PRESETS
    // =========================================================================
    //

    struct SavedLoadout
    {
        std::string name;

        LoadoutPresetSlot slots[5]{};
    };


private:

    //
    // =========================================================================
    //  DATABASE
    // =========================================================================
    //

    // All weapons discovered from gWeaponsArray.
    std::vector<WeaponEntry> m_weapons;

    // All ammo types discovered from weapon definitions.
    std::vector<AmmoEntry> m_ammo;

    // Ammo types considered legal/default for the selected weapon.
    std::vector<AmmoEntry> m_legalAmmo;

    // 
    std::vector<SavedLoadout> m_savedLoadouts;
    int m_selectedSavedLoadout{ -1 };

    char m_weaponSearch[64]{};
    char m_savedLoadoutName[64]{};


    //
    // =========================================================================
    //  CURRENT LOADOUT
    // =========================================================================
    //

    InventorySlot m_inventory[5]{};


    //
    // =========================================================================
    //  EDITOR STATE
    // =========================================================================
    //

    int m_selectedSlot{ 0 };
    int m_selectedWeapon{ 0 };
    int m_selectedAmmo{ 0 };
    int m_selectedPreset{ 0 };

    // When false, changing a weapon automatically applies
    // the weapon's default/legal ammo.
    //
    // When true, weapon and ammo selection are independent.
    bool m_customAmmo{ false };


    //
    // =========================================================================
    //  RESET STATE
    // =========================================================================
    //

    // Captured immediately before the first editor modification.
    //
    // This gives RESET LOADOUT a stable baseline without trying
    // to continuously track normal gameplay inventory changes.
    LoadoutSnapshot m_snapshot{};


private:

    //
    // =========================================================================
    //  REFRESH
    // =========================================================================
    //

    void RefreshInventory();

    void RefreshWeapons();

    // Rebuilds the legal/default ammunition list for
    // the currently selected weapon.
    void RefreshLegalAmmo();

    void SetupModelView(ID3D11Device* device, const std::filesystem::path& assetOverride = {});
    void ShutdownModelView();
    void DrawModelView(ID3D11DeviceContext* context);

private:

    //
    // =========================================================================
    //  LOADOUT OPERATIONS
    // =========================================================================
    //

    // Selects/applies a weapon to the current slot.
    //
    // If custom ammo is disabled, the weapon's default
    // ammunition is also applied.
    void SelectWeapon(int weaponIndex);

    // Applies an arbitrary ammo type to the current slot.
    void SelectAmmo(int ammoIndex);

    // Captures the current loadout before the first edit.
    void CaptureSnapshot();

    // Restores the loadout captured before editing began.
    void ResetLoadout();

    // Applies all five slots from a predefined loadout.
    void ApplyPreset(const LoadoutPreset& preset);

    // 
    void SyncSelectionToInventory();

    //  
    void SaveCurrentLoadout();

    //  
    void ApplySavedLoadout( const SavedLoadout& saved );

private:

    //
    // =========================================================================
    //  UI
    // =========================================================================
    //

    void DrawInventory();

    void DrawWeaponSelector();

    void DrawAmmoSelector();

    void DrawWeaponInfo();

    void DrawAmmoInfo();

    void DrawPresets();

    void DrawLoadouts();

    void DrawSavedLoadouts();


public:

    //
    // =========================================================================
    //  MAIN
    // =========================================================================
    //

    LoadoutUI();
    ~LoadoutUI();
    static int RunModelViewerSelfTest();
    void Draw();
};