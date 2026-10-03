#include "LoadoutUI.h"
#include <DXWindow/DXWindow.h>
#include "Menu.h"

#include <algorithm>


//
// ============================================================================
//  HELPERS
// ============================================================================
//

static const char* GetLoadoutSlotName(int slot)
{
	static const char* names[] =
	{
		"PRIMARY",
		"SECONDARY",
		"EQUIPMENT 1",
		"EQUIPMENT 2",
		"EQUIPMENT 3"
	};

	if (slot < 0 || slot >= IM_ARRAYSIZE(names))
		return "UNKNOWN";

	return names[slot];
}


//
// ============================================================================
//  INVENTORY
// ============================================================================
//

void LoadoutUI::RefreshInventory()
{
	for (auto& slot : m_inventory)
		slot = {};

	__int64 eemem = g_PSXMemory.GetEEMemory();
	if (!eemem)
		return;

	i64_t sealAddr = 0;

	Engine::zdb::Classes::CZSealBody seal{};

	if (!Engine::zdb::Tools::Entity::GetLocalSeal(
		seal,
		&sealAddr))
	{
		return;
	}

	if (!sealAddr)
		return;

	const i64_t weaponArray =
		sealAddr +
		offsetof(
			Engine::zdb::Classes::CZSealBody,
			pPrimaryWeapon
		);

	const i64_t ammoArray =
		sealAddr +
		offsetof(
			Engine::zdb::Classes::CZSealBody,
			pPrimaryAmmoType
		);

	const int slotCount =
		std::min<int>(
			seal.MaxWeaponIndex,
			5
			);

	for (int i = 0; i < slotCount; i++)
	{
		auto& slot = m_inventory[i];

		//
		// Weapon
		//

		slot.weaponAddress =
			g_PSXMemory.Read<i32_t>(
				weaponArray +
				(i * sizeof(i32_t))
				);

		if (slot.weaponAddress)
		{
			auto weapon =
				g_PSXMemory.Read<Engine::zdb::Classes::CZWeapon>(
					eemem + slot.weaponAddress
					);

			if (weapon.pName)
			{
				g_PSXMemory.ReadString(
					eemem + weapon.pName,
					slot.weaponName,
					64
				);
			}
		}

		if (slot.weaponName.empty())
			slot.weaponName = "<empty>";


		//
		// Ammo
		//

		slot.ammoAddress =
			g_PSXMemory.Read<i32_t>(
				ammoArray +
				(i * sizeof(i32_t))
				);

		if (slot.ammoAddress)
		{
			auto ammo =
				g_PSXMemory.Read<Engine::zdb::Classes::CZAmmo>(
					eemem + slot.ammoAddress
					);

			if (ammo.pDisplayName)
			{
				g_PSXMemory.ReadString(
					eemem + ammo.pDisplayName,
					slot.ammoName,
					64
				);
			}

			if (slot.ammoName.empty() &&
				ammo.pAmmoName)
			{
				g_PSXMemory.ReadString(
					eemem + ammo.pAmmoName,
					slot.ammoName,
					64
				);
			}
		}

		if (slot.ammoName.empty())
			slot.ammoName = "<none>";
	}
}


//
// ============================================================================
//  DATABASE
// ============================================================================
//

void LoadoutUI::RefreshWeapons()
{
	m_weapons.clear();
	m_ammo.clear();

	__int64 eemem = g_PSXMemory.GetEEMemory();
	if (!eemem)
		return;

	auto array =
		g_PSXMemory.Read<Engine::zdb::Structs::ZArray>(
			eemem + Engine::zdb::Offsets::gWeaponsArray
			);

	if (array.count <= 0)
		return;

	Engine::zdb::Tools::Container::ZArray_ForEach<
		Engine::zdb::Classes::CZWeapon>(
			array,
			[&](
				const Engine::zdb::Classes::CZWeapon& weapon,
				i32_t address)
			{
				std::string name;

				if (weapon.pName)
				{
					g_PSXMemory.ReadString(
						eemem + weapon.pName,
						name,
						64
					);
				}

				if (!name.empty())
				{
					WeaponEntry entry{};

					entry.name = std::move(name);
					entry.address = address;
					entry.weapon = weapon;

					m_weapons.push_back(
						std::move(entry)
					);
				}


				//
				// Harvest every unique ammo definition referenced
				// by the weapon database.
				//

				Engine::zdb::Tools::Container::ZArray_ForEachAddress(
					weapon.mLegalAmmoList,
					[&](i32_t ammoAddress)
					{
						bool exists = false;

						for (const auto& existing : m_ammo)
						{
							if (existing.address == ammoAddress)
							{
								exists = true;
								break;
							}
						}

						if (exists)
							return;

						auto ammo =
							g_PSXMemory.Read<
							Engine::zdb::Classes::CZAmmo>(
								eemem + ammoAddress
								);

						std::string ammoName;

						if (ammo.pDisplayName)
						{
							g_PSXMemory.ReadString(
								eemem + ammo.pDisplayName,
								ammoName,
								64
							);
						}

						if (ammoName.empty() &&
							ammo.pAmmoName)
						{
							g_PSXMemory.ReadString(
								eemem + ammo.pAmmoName,
								ammoName,
								64
							);
						}

						if (ammoName.empty())
							ammoName = "<unknown>";

						AmmoEntry entry{};

						entry.name = std::move(ammoName);
						entry.address = ammoAddress;
						entry.ammo = ammo;

						m_ammo.push_back(
							std::move(entry)
						);
					}
				);
			}
	);

	if (m_selectedWeapon >=
		static_cast<int>(m_weapons.size()))
	{
		m_selectedWeapon = 0;
	}

	if (m_selectedAmmo >=
		static_cast<int>(m_ammo.size()))
	{
		m_selectedAmmo = 0;
	}

	RefreshLegalAmmo();
}


//
// ============================================================================
//  LEGAL AMMO
// ============================================================================
//

void LoadoutUI::RefreshLegalAmmo()
{
	m_legalAmmo.clear();

	if (m_selectedWeapon < 0 ||
		m_selectedWeapon >=
		static_cast<int>(m_weapons.size()))
	{
		return;
	}

	__int64 eemem = g_PSXMemory.GetEEMemory();
	if (!eemem)
		return;

	const auto& weapon =
		m_weapons[m_selectedWeapon].weapon;

	Engine::zdb::Tools::Container::ZArray_ForEachAddress(
		weapon.mLegalAmmoList,
		[&](i32_t address)
		{
			//
			// Prefer the already cached global ammo entry.
			//

			for (const auto& entry : m_ammo)
			{
				if (entry.address == address)
				{
					m_legalAmmo.push_back(entry);
					return;
				}
			}


			//
			// Fallback in case the ammo wasn't harvested.
			//

			auto ammo =
				g_PSXMemory.Read<
				Engine::zdb::Classes::CZAmmo>(
					eemem + address
					);

			std::string name;

			if (ammo.pDisplayName)
			{
				g_PSXMemory.ReadString(
					eemem + ammo.pDisplayName,
					name,
					64
				);
			}

			if (name.empty() &&
				ammo.pAmmoName)
			{
				g_PSXMemory.ReadString(
					eemem + ammo.pAmmoName,
					name,
					64
				);
			}

			if (name.empty())
				name = "<unknown>";

			AmmoEntry entry{};

			entry.name = std::move(name);
			entry.address = address;
			entry.ammo = ammo;

			m_legalAmmo.push_back(
				std::move(entry)
			);
		}
	);
}


//
// ============================================================================
//  SNAPSHOT
// ============================================================================
//

void LoadoutUI::CaptureSnapshot()
{
	if (m_snapshot.valid)
		return;

	//
	// Make sure we're capturing what is actually
	// equipped immediately before the first edit.
	//

	RefreshInventory();

	for (int i = 0; i < 5; i++)
	{
		m_snapshot.weapon[i] =
			m_inventory[i].weaponAddress;

		m_snapshot.ammo[i] =
			m_inventory[i].ammoAddress;
	}

	m_snapshot.valid = true;
}


void LoadoutUI::ResetLoadout()
{
	if (!m_snapshot.valid)
		return;

	for (int i = 0; i < 5; i++)
	{
		if (m_snapshot.weapon[i])
		{
			Engine::zdb::Patches::SetWeapon(
				i,
				static_cast<
				Engine::zdb::Enums::EWeapon>(
					m_snapshot.weapon[i]
					)
			);
		}

		if (m_snapshot.ammo[i])
		{
			Engine::zdb::Patches::SetWeaponAmmoType(
				i,
				static_cast<
				Engine::zdb::Enums::EWeaponAmmo>(
					m_snapshot.ammo[i]
					)
			);
		}
	}

	//
	// The restored state becomes the new baseline.
	//

	m_snapshot = {};

	RefreshInventory();
}


void LoadoutUI::SyncSelectionToInventory()
{
	if (m_selectedSlot < 0 ||
		m_selectedSlot >= 5)
	{
		return;
	}

	const auto& slot = m_inventory[m_selectedSlot];


	//
	// Weapon
	//

	for (int i = 0;
		i < static_cast<int>(m_weapons.size());
		i++)
	{
		if (m_weapons[i].address ==
			slot.weaponAddress)
		{
			m_selectedWeapon = i;
			break;
		}
	}

	RefreshLegalAmmo();


	//
	// Ammo
	//
	// Match against the global database so this also
	// works if the game already has custom ammo equipped.
	//

	for (int i = 0;
		i < static_cast<int>(m_ammo.size());
		i++)
	{
		if (m_ammo[i].address ==
			slot.ammoAddress)
		{
			m_selectedAmmo = i;
			break;
		}
	}
}


void LoadoutUI::SaveCurrentLoadout()
{
	RefreshInventory();

	SavedLoadout saved{};

	for (int i = 0; i < 5; i++)
	{
		saved.slots[i].weapon =
			m_inventory[i].weaponAddress;

		saved.slots[i].ammo =
			m_inventory[i].ammoAddress;
	}

	saved.name =
		"Loadout " +
		std::to_string(
			m_savedLoadouts.size() + 1
		);

	m_savedLoadouts.push_back(
		std::move(saved)
	);

	m_selectedSavedLoadout =
		static_cast<int>(
			m_savedLoadouts.size()
			) - 1;
}

//
// ============================================================================
//  WEAPON / AMMO SELECTION
// ============================================================================
//

void LoadoutUI::SelectWeapon(int weaponIndex)
{
	if (weaponIndex < 0 ||
		weaponIndex >=
		static_cast<int>(m_weapons.size()))
	{
		return;
	}

	CaptureSnapshot();

	m_selectedWeapon = weaponIndex;

	RefreshLegalAmmo();

	const auto& weapon =
		m_weapons[m_selectedWeapon];

	//
	// Always apply the selected weapon.
	//

	Engine::zdb::Patches::SetWeapon(
		m_selectedSlot,
		static_cast<
		Engine::zdb::Enums::EWeapon>(
			weapon.address
			)
	);


	//
	// Normal mode:
	//
	// Selecting a weapon, including clicking the
	// currently selected weapon again, restores its
	// first legal/default ammunition type.
	//

	if (!m_customAmmo &&
		!m_legalAmmo.empty())
	{
		const auto& ammo =
			m_legalAmmo[0];

		Engine::zdb::Patches::SetWeaponAmmoType(
			m_selectedSlot,
			static_cast<
			Engine::zdb::Enums::EWeaponAmmo>(
				ammo.address
				)
		);

		//
		// Synchronize the global ammo selection.
		//

		for (int i = 0;
			i < static_cast<int>(m_ammo.size());
			i++)
		{
			if (m_ammo[i].address ==
				ammo.address)
			{
				m_selectedAmmo = i;
				break;
			}
		}
	}

	RefreshInventory();
}


void LoadoutUI::SelectAmmo(int ammoIndex)
{
	if (ammoIndex < 0 ||
		ammoIndex >=
		static_cast<int>(m_ammo.size()))
	{
		return;
	}

	CaptureSnapshot();

	m_selectedAmmo = ammoIndex;

	const auto& ammo =
		m_ammo[m_selectedAmmo];

	Engine::zdb::Patches::SetWeaponAmmoType(
		m_selectedSlot,
		static_cast<
		Engine::zdb::Enums::EWeaponAmmo>(
			ammo.address
			)
	);

	RefreshInventory();
}


//
// ============================================================================
//  PRESETS
// ============================================================================
//

void LoadoutUI::ApplyPreset(const LoadoutPreset& preset)
{
	CaptureSnapshot();

	for (int i = 0; i < 5; i++)
	{
		const auto& slot =
			preset.slots[i];

		if (slot.weapon)
		{
			Engine::zdb::Patches::SetWeapon(
				i,
				static_cast<
				Engine::zdb::Enums::EWeapon>(
					slot.weapon
					)
			);
		}

		if (slot.ammo)
		{
			Engine::zdb::Patches::SetWeaponAmmoType(
				i,
				static_cast<
				Engine::zdb::Enums::EWeaponAmmo>(
					slot.ammo
					)
			);
		}
	}

	RefreshInventory();
}

void LoadoutUI::ApplySavedLoadout(const SavedLoadout& saved)
{
	CaptureSnapshot();

	for (int i = 0; i < 5; i++)
	{
		const auto& slot =
			saved.slots[i];

		if (slot.weapon)
		{
			Engine::zdb::Patches::SetWeapon(
				i,
				static_cast<
				Engine::zdb::Enums::EWeapon>(
					slot.weapon
					)
			);
		}

		if (slot.ammo)
		{
			Engine::zdb::Patches::SetWeaponAmmoType(
				i,
				static_cast<
				Engine::zdb::Enums::EWeaponAmmo>(
					slot.ammo
					)
			);
		}
	}

	RefreshInventory();
	SyncSelectionToInventory();
}


//
// ============================================================================
//  CURRENT LOADOUT
// ============================================================================
//

void LoadoutUI::DrawInventory()
{
	ImGui::SeparatorText("CURRENT LOADOUT");

	if (!ImGui::BeginTable(
		"##loadout_inventory",
		3,
		ImGuiTableFlags_Borders |
		ImGuiTableFlags_RowBg |
		ImGuiTableFlags_SizingStretchProp))
	{
		return;
	}

	ImGui::TableSetupColumn(
		"SLOT",
		ImGuiTableColumnFlags_WidthFixed,
		110.f
	);

	ImGui::TableSetupColumn("WEAPON");
	ImGui::TableSetupColumn("AMMO");

	ImGui::TableHeadersRow();

	for (int i = 0; i < 5; i++)
	{
		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);

		ImGui::PushID(i);

		if (ImGui::Selectable(
			GetLoadoutSlotName(i),
			m_selectedSlot == i))
		{
			m_selectedSlot = i;

			SyncSelectionToInventory();
		}

		ImGui::PopID();


		ImGui::TableSetColumnIndex(1);

		ImGui::TextUnformatted(
			m_inventory[i].weaponName.c_str()
		);


		ImGui::TableSetColumnIndex(2);

		ImGui::TextUnformatted(
			m_inventory[i].ammoName.c_str()
		);
	}

	ImGui::EndTable();
}


//
// ============================================================================
//  WEAPON SELECTOR
// ============================================================================
//

void LoadoutUI::DrawWeaponSelector()
{
	ImGui::SeparatorText(
		GetLoadoutSlotName(m_selectedSlot)
	);

	//
	// Weapon search
	//

	ImGui::SetNextItemWidth(
		ImGui::GetContentRegionAvail().x
	);

	ImGui::InputTextWithHint(
		"##weapon_search",
		"Search weapons...",
		m_weaponSearch,
		sizeof(m_weaponSearch)
	);


	//
	// Selected weapon
	//

	const char* preview = "<none>";

	if (m_selectedWeapon >= 0 &&
		m_selectedWeapon <
		static_cast<int>(m_weapons.size()))
	{
		preview =
			m_weapons[m_selectedWeapon].name.c_str();
	}

	ImGui::SetNextItemWidth(
		ImGui::GetContentRegionAvail().x
	);

	if (ImGui::BeginCombo(
		"##weapon_select",
		preview))
	{
		for (int i = 0;
			i < static_cast<int>(m_weapons.size());
			i++)
		{
			const auto& entry =
				m_weapons[i];


			//
			// Search
			//

			if (m_weaponSearch[0])
			{
				std::string name =
					entry.name;

				std::string search =
					m_weaponSearch;

				std::transform(
					name.begin(),
					name.end(),
					name.begin(),
					[](unsigned char c)
					{
						return static_cast<char>(
							std::tolower(c)
							);
					}
				);

				std::transform(
					search.begin(),
					search.end(),
					search.begin(),
					[](unsigned char c)
					{
						return static_cast<char>(
							std::tolower(c)
							);
					}
				);

				if (name.find(search) ==
					std::string::npos)
				{
					continue;
				}
			}


			//
			// Weapon
			//

			ImGui::PushID(i);

			const bool selected =
				m_selectedWeapon == i;

			if (ImGui::Selectable(
				entry.name.c_str(),
				selected))
			{
				//
				// Selecting the currently equipped
				// weapon again intentionally reapplies
				// its default ammo when custom ammo
				// is disabled.
				//

				SelectWeapon(i);
			}

			if (selected)
				ImGui::SetItemDefaultFocus();

			ImGui::PopID();
		}

		ImGui::EndCombo();
	}
}


//
// ============================================================================
//  AMMO SELECTOR
// ============================================================================
//

void LoadoutUI::DrawAmmoSelector()
{
	//
	// Ammo selection
	//

	if (!m_customAmmo)
	{
		//
		// Normal mode:
		// display the weapon's default/legal ammo.
		//

		if (!m_legalAmmo.empty())
		{
			const auto& ammo =
				m_legalAmmo[0];

			ImGui::SetNextItemWidth(
				ImGui::GetContentRegionAvail().x
			);

			ImGui::BeginDisabled();

			if (ImGui::BeginCombo(
				"##ammo_select",
				ammo.name.c_str()))
			{
				ImGui::EndCombo();
			}

			ImGui::EndDisabled();
		}
		else
		{
			ImGui::SetNextItemWidth(
				ImGui::GetContentRegionAvail().x
			);

			ImGui::BeginDisabled();

			if (ImGui::BeginCombo(
				"##ammo_select",
				"<none>"))
			{
				ImGui::EndCombo();
			}

			ImGui::EndDisabled();
		}
	}
	else
	{
		//
		// Custom mode:
		// expose every discovered ammo definition.
		//

		if (!m_ammo.empty())
		{
			if (m_selectedAmmo < 0 ||
				m_selectedAmmo >=
				static_cast<int>(m_ammo.size()))
			{
				m_selectedAmmo = 0;
			}

			const auto& current =
				m_ammo[m_selectedAmmo];

			ImGui::SetNextItemWidth(
				ImGui::GetContentRegionAvail().x
			);

			if (ImGui::BeginCombo(
				"##ammo_select",
				current.name.c_str()))
			{
				for (int i = 0;
					i < static_cast<int>(m_ammo.size());
					i++)
				{
					const auto& entry =
						m_ammo[i];

					ImGui::PushID(i);

					const bool selected =
						m_selectedAmmo == i;

					if (ImGui::Selectable(
						entry.name.c_str(),
						selected))
					{
						SelectAmmo(i);
					}

					if (selected)
						ImGui::SetItemDefaultFocus();

					ImGui::PopID();
				}

				ImGui::EndCombo();
			}
		}
		else
		{
			ImGui::TextDisabled(
				"No ammo definitions available."
			);
		}
	}


	//
	// Custom ammo toggle
	//

	const bool previousCustomAmmo =
		m_customAmmo;

	ImGui::Checkbox(
		"CUSTOM AMMO",
		&m_customAmmo
	);

	GUI::Tooltip(
		"Allow ammunition types not normally used by this weapon."
	);


	//
	// Turning custom ammo OFF immediately
	// restores the current weapon's default ammo.
	//

	if (previousCustomAmmo &&
		!m_customAmmo &&
		m_selectedWeapon >= 0 &&
		m_selectedWeapon <
		static_cast<int>(m_weapons.size()))
	{
		SelectWeapon(
			m_selectedWeapon
		);
	}

	ImGui::SameLine();


	//
	// Action buttons
	//

	const float spacing =
		ImGui::GetStyle().ItemSpacing.x;

	const float availableWidth =
		ImGui::GetContentRegionAvail().x;

	const float buttonWidth =
		(availableWidth - (spacing * 2.f)) / 3.f;


	//
	// Refill ammo
	//

	if (ImGui::Button(
		"REFILL ALL AMMO",
		ImVec2(buttonWidth, 0.f)))
	{
		Engine::zdb::Patches::RefillAllAmmo();
	}


	//
	// Save loadout
	//

	ImGui::SameLine();

	if (ImGui::Button(
		"SAVE LOADOUT",
		ImVec2(buttonWidth, 0.f)))
	{
		SaveCurrentLoadout();
	}


	//
	// Reset loadout
	//

	ImGui::SameLine();

	ImGui::BeginDisabled(
		!m_snapshot.valid
	);

	if (ImGui::Button(
		"RESET LOADOUT",
		ImVec2(
			ImGui::GetContentRegionAvail().x,
			0.f
		)))
	{
		ResetLoadout();
	}

	ImGui::EndDisabled();
}


//
// ============================================================================
//  WEAPON INFO
// ============================================================================
//

void LoadoutUI::DrawWeaponInfo()
{
	if (m_selectedWeapon < 0 ||
		m_selectedWeapon >=
		static_cast<int>(m_weapons.size()))
	{
		return;
	}

	const auto& entry =
		m_weapons[m_selectedWeapon];

	const auto& weapon =
		entry.weapon;

	ImGui::SeparatorText("WEAPON");

	ImGui::Text(
		"%s",
		entry.name.c_str()
	);

	ImGui::SameLine();

	ImGui::TextDisabled(
		"[0x%08X]",
		static_cast<unsigned int>(
			entry.address
			)
	);

	ImGui::Text(
		"Mag: %d   Mags: %d   Range: %.0f   Effective: %.0f   Fire Wait: %.3f",
		weapon.szMags,
		weapon.defaultMags,
		weapon.mMaxRange,
		weapon.mEffectiveRange,
		weapon.mFireWait
	);
}


//
// ============================================================================
//  AMMO INFO
// ============================================================================
//

void LoadoutUI::DrawAmmoInfo()
{
	const AmmoEntry* entry = nullptr;

	//
	// Custom mode shows the selected arbitrary ammo.
	//

	if (m_customAmmo)
	{
		if (m_selectedAmmo >= 0 &&
			m_selectedAmmo <
			static_cast<int>(m_ammo.size()))
		{
			entry =
				&m_ammo[m_selectedAmmo];
		}
	}

	//
	// Normal mode shows the weapon's default ammo.
	//

	else if (!m_legalAmmo.empty())
	{
		entry =
			&m_legalAmmo[0];
	}

	if (!entry)
		return;

	const auto& ammo =
		entry->ammo;

	ImGui::SeparatorText("AMMO");

	ImGui::Text(
		"%s",
		entry->name.c_str()
	);

	ImGui::SameLine();

	ImGui::TextDisabled(
		"[0x%08X | ID: %d]",
		static_cast<unsigned int>(
			entry->address
			),
		static_cast<int>(
			ammo.m_ID
			)
	);

	ImGui::Text(
		"Damage: %.2f   Stun: %.2f   Piercing: %.2f",
		ammo.bulletImpactDmg,
		ammo.stun,
		ammo.piercing
	);

	if (ammo.explosionDamage != 0.f ||
		ammo.explosionRadius != 0.f)
	{
		ImGui::Text(
			"Explosion: %.2f   Radius: %.2f",
			ammo.explosionDamage,
			ammo.explosionRadius
		);
	}
}


//
// ============================================================================
//  PRESETS
// ============================================================================
//

void LoadoutUI::DrawLoadouts()
{
	if (!ImGui::BeginTable(
		"##loadout_lists",
		2,
		ImGuiTableFlags_BordersInnerV |
		ImGuiTableFlags_SizingStretchSame))
	{
		return;
	}

	ImGui::TableNextRow();


	//
	// loadouts
	//

	ImGui::TableSetColumnIndex(0);

	DrawPresets();


	//
	// Saved loadouts
	//

	ImGui::TableSetColumnIndex(1);

	DrawSavedLoadouts();


	ImGui::EndTable();
}

void LoadoutUI::DrawPresets()
{
	//
	// These are intentionally data rather than individual
	// functions so adding/removing presets stays trivial.
	//

	static const LoadoutPreset presets[] =
	{
		{
			"SUPPRESSED ASSAULT",
			"M4A1 SD / Mark 23SD with general-purpose equipment.",
			{
				{ WEAPON_AR_M4A1_SD, AMMO_556x45_SD },
				{ WEAPON_HG_MK23SD,  AMMO_45ACP },
				{ WEAPON_EQ_M67,     AMMO_M67 },
				{ WEAPON_EQ_SMOKE,   AMMO_Smoke },
				{ WEAPON_EQ_M141,    AMMO_M141 }
			}
		},

		{
			"ASSAULT",
			"M4A1 / 226 with grenades and utility equipment.",
			{
				{ WEAPON_AR_M4A1,    AMMO_556x45 },
				{ WEAPON_HG_P226,    AMMO_9x19p },
				{ WEAPON_EQ_M67,     AMMO_M67 },
				{ WEAPON_EQ_SMOKE,   AMMO_Smoke },
				{ WEAPON_EQ_M141,    AMMO_M141 }
			}
		},

		{
			"SNIPER",
			"SR-25 SD / Mark 23SD with supporting equipment.",
			{
				{ WEAPON_SR_SR25_SD, AMMO_762x51 },
				{ WEAPON_HG_MK23SD,  AMMO_45ACP },
				{ WEAPON_EQ_M67,     AMMO_M67 },
				{ WEAPON_EQ_SMOKE,   AMMO_Smoke },
				{ WEAPON_EQ_M141,    AMMO_M141 }
			}
		},

		{
			"HEAVY",
			"M60E3 / DE .50 with explosive equipment.",
			{
				{ WEAPON_LMG_M60E3,  AMMO_762x51 },
				{ WEAPON_HG_DE50,    AMMO_50AE },
				{ WEAPON_EQ_M67,     AMMO_M67 },
				{ WEAPON_EQ_C4,      AMMO_C4 },
				{ WEAPON_EQ_M141,    AMMO_M141 }
			}
		}
	};

	if (m_selectedPreset < 0 ||
		m_selectedPreset >= IM_ARRAYSIZE(presets))
	{
		m_selectedPreset = 0;
	}

	ImGui::SeparatorText("LOADOUTS");


	//
	// Preset list
	//

	const float listHeight =
		ImGui::GetTextLineHeightWithSpacing() * 5.f;

	if (ImGui::BeginListBox(
		"##op_loadouts",
		ImVec2(-1.f, listHeight)))
	{
		for (int i = 0;
			i < IM_ARRAYSIZE(presets);
			i++)
		{
			const bool selected =
				m_selectedPreset == i;

			if (ImGui::Selectable(
				presets[i].name,
				selected))
			{
				m_selectedPreset = i;
			}

			if (selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndListBox();
	}


	//
	// Description
	//

	const auto& preset =
		presets[m_selectedPreset];

	ImGui::TextDisabled(
		"%s",
		preset.description
	);


	//
	// Apply
	//

	if (ImGui::Button(
		"APPLY LOADOUT",
		ImVec2(
			ImGui::GetContentRegionAvail().x,
			0.f
		)))
	{
		ApplyPreset(preset);
	}
}

void LoadoutUI::DrawSavedLoadouts()
{
	ImGui::SeparatorText("SAVED LOADOUTS");


	//
	// Saved loadout list
	//

	const float listHeight =
		ImGui::GetTextLineHeightWithSpacing() * 5.f;

	if (ImGui::BeginListBox(
		"##saved_loadouts",
		ImVec2(-1.f, listHeight)))
	{
		if (m_savedLoadouts.empty())
		{
			ImGui::TextDisabled(
				"No saved loadouts."
			);
		}
		else
		{
			for (int i = 0;
				i < static_cast<int>(m_savedLoadouts.size());
				i++)
			{
				const auto& saved =
					m_savedLoadouts[i];

				const bool selected =
					m_selectedSavedLoadout == i;

				if (ImGui::Selectable(
					saved.name.c_str(),
					selected))
				{
					m_selectedSavedLoadout = i;
				}

				if (selected)
					ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndListBox();
	}


	//
	// Selection
	//

	const bool hasSelection =
		m_selectedSavedLoadout >= 0 &&
		m_selectedSavedLoadout <
		static_cast<int>(m_savedLoadouts.size());


	//
	// Description / status line
	//
	// This intentionally occupies the same vertical
	// space as the loadout description.
	//

	if (hasSelection)
	{
		ImGui::TextDisabled(
			"Saved custom loadout."
		);
	}
	else
	{
		ImGui::TextDisabled(
			"No loadout selected."
		);
	}


	//
	// Actions
	//

	ImGui::BeginDisabled(
		!hasSelection
	);

	const float spacing =
		ImGui::GetStyle().ItemSpacing.x;

	const float availableWidth =
		ImGui::GetContentRegionAvail().x;

	const float buttonWidth =
		(availableWidth - spacing) * 0.5f;


	if (ImGui::Button(
		"LOAD",
		ImVec2(buttonWidth, 0.f)))
	{
		ApplySavedLoadout(
			m_savedLoadouts[
				m_selectedSavedLoadout
			]
		);
	}


	ImGui::SameLine();


	if (ImGui::Button(
		"DELETE",
		ImVec2(
			ImGui::GetContentRegionAvail().x,
			0.f
		)))
	{
		m_savedLoadouts.erase(
			m_savedLoadouts.begin() +
			m_selectedSavedLoadout
		);

		if (m_savedLoadouts.empty())
		{
			m_selectedSavedLoadout = -1;
		}
		else if (
			m_selectedSavedLoadout >=
			static_cast<int>(
				m_savedLoadouts.size()
				))
		{
			m_selectedSavedLoadout =
				static_cast<int>(
					m_savedLoadouts.size()
					) - 1;
		}
	}


	ImGui::EndDisabled();
}

//
// ============================================================================
//  MAIN
// ============================================================================
//

void LoadoutUI::Draw()
{
	if (m_weapons.empty())
	{
		RefreshWeapons();
	}

	//
	// Keep displayed inventory synchronized with the game.
	//
	// We can throttle this later if desired.
	//

	RefreshInventory();

	//
	// Initial editor state should represent what
	// the player actually has equipped.
	//

	if (!m_initialized)
	{
		SyncSelectionToInventory();
		m_initialized = true;
	}


	//
	// Current inventory / slot selection.
	//

	DrawInventory();

	ImGui::Spacing();


	//
	// Current slot editor.
	//

	DrawWeaponSelector();

	DrawAmmoSelector();

	DrawWeaponInfo();

	DrawAmmoInfo();

	ImGui::Spacing();


	//
	// Predefined loadouts.
	//

	DrawLoadouts();
}