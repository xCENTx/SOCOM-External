#include "SOCOM.h"

namespace Engine
{
	namespace zdb
	{

		namespace Structs
		{
			Vec3 ZViewModel::origin() const { return Vec3(pos.x, pos.y, pos.z); }
		}

		namespace Tools
		{
			/* GET */
			bool GetCameraMatrixSet(Structs::tag_ZCAM_MTX_SET& mtxSet)
			{
				__int64 eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return false;

				auto pCamera = g_PSXMemory.Read<__int32>(eemem + Offsets::gCamera);
				if (!pCamera)
					return false;
				
				auto camera = g_PSXMemory.Read<Classes::zdb_CCamera>(eemem + pCamera);

				mtxSet = camera.m_mtxSet;
				
				return true;
			}

			bool GetCameraViewMatrix(Structs::ZViewModel& CameraView)
			{
				__int64 eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return false;

				auto pCamera = g_PSXMemory.Read<__int32>(eemem + Offsets::gAppCamera);
				if (!pCamera)
					return false;

				auto camera = g_PSXMemory.Read<Classes::CAppCamera>(eemem + pCamera);
				if (!camera.pCamera)
					return false;

				CameraView = g_PSXMemory.Read<Structs::ZViewModel>(eemem + camera.pCamera);

				return true;
			}

			bool GetCameraModelViewMatrix(Matrix16 & ModelView)
			{
				__int64 eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return false;

				auto pCamera = g_PSXMemory.Read<__int32>(eemem + Offsets::gAppCamera);
				if (!pCamera)
					return false;

				auto camera = g_PSXMemory.Read<Classes::CAppCamera>(eemem + pCamera);
				if (!camera.pCamera)
					return false;

				ModelView = g_PSXMemory.Read<Matrix16>(eemem + camera.pCamera);

				return true;
			}

			bool GetLocalSeal(Classes::CZSealBody& seal, i64_t* pSealAddr)
			{
				//	get eemem
				__int64 eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return false;
				
				//	pointer to local seal
				auto pSeal = g_PSXMemory.Read<__int32>(eemem + Offsets::gLocalSeal);
				if (!pSeal)
					return false;

				//	result
				seal = g_PSXMemory.Read<Classes::CZSealBody>(eemem + pSeal);
				*pSealAddr = eemem + pSeal;
				
				return true;
			}

			bool GetPlayers(std::vector<Classes::CZSealBody>*players)
			{
				std::vector<Classes::CZSealBody> seals;

				__int64 eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return false;

				auto sealArray = g_PSXMemory.Read<Structs::ZArray>(eemem + Offsets::gEntityArray);
				if (sealArray.count <= 1 || sealArray.begin <= 0 || sealArray.end <= 0)
					return false;


				auto it = g_PSXMemory.Read<Structs::ZIterator>(eemem + sealArray.begin);
				auto end = g_PSXMemory.Read<Structs::ZIterator>(eemem + it.prev);
				do
				{
					auto data = it.data;
					if (data > 0)
					{
						auto seal = g_PSXMemory.Read<Classes::CZSealBody>(eemem + data);
						if (seal.pName)
							seals.push_back(seal);
					}

					it = g_PSXMemory.Read<Structs::ZIterator>(eemem + it.next);

				} while (it.data != end.data);

				*players = seals;

				return players->size() > 0;
			}

			bool GetWeapon(const int& weaponIndex, Classes::CZWeapon& weapon, i64_t* pWeaponAddr)
			{
				__int64 eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return false;

				i64_t sealAddr = 0;
				Classes::CZSealBody czSeal;
				if (!Tools::GetLocalSeal(czSeal, &sealAddr) || !sealAddr)
					return false;

				const auto szWeaponArray = czSeal.MaxWeaponIndex;
				if (weaponIndex >= szWeaponArray)
					return false;

				const auto& pBaseWeapon = sealAddr + offsetof(Classes::CZSealBody, pPrimaryWeapon);
				if (pBaseWeapon <= sealAddr)
					return false;

				const auto& pWeapon = pBaseWeapon + (weaponIndex * 0x4);
				if (!pWeapon)
					return false;

				weapon = g_PSXMemory.Read<Classes::CZWeapon>(eemem + pWeapon);
				*pWeaponAddr = eemem + pWeapon;

				return true;
			}

			std::string GetWeaponName(const Enums::EWeapon& weapon)
			{
				std::string result = "";
				__int64 eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return result;

				const auto& addr = eemem + (i32_t)weapon;
				if (addr <= (i32_t)weapon)
					return result;

				const auto& czWeapon = g_PSXMemory.Read<Classes::CZWeapon>(addr);

				g_PSXMemory.ReadString(eemem + czWeapon.pName, result);

				return result;
			}

			std::string GetAmmoName(const Enums::EWeaponAmmo& ammo)
			{
				std::string result = "";
				__int64 eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return result;

				const auto& addr = eemem + (i32_t)ammo;
				if (addr <= (i32_t)ammo)
					return result;

				const auto& czAmmo = g_PSXMemory.Read<Classes::CZAmmo>(addr);

				g_PSXMemory.ReadString(eemem + czAmmo.pAmmoName, result);

				return result;
			}


			/* RENDER */
			bool ProjectWorldToScreen(Vec3 WorldLocation, Structs::ZViewModel CameraView, float fov, Vec2 szScreen, Vec2* screen2D)
			{
				Vec3 cam_pos = Vec3(CameraView.pos.x, CameraView.pos.y, CameraView.pos.z);
				Vec3 cam_look = Vec3(-CameraView.fwd.x, -CameraView.fwd.y, -CameraView.fwd.z);
				Vec3 cam_right = Vec3(CameraView.right.x, CameraView.right.y, CameraView.right.z);
				Vec3 cam_up = Vec3(CameraView.up.x, CameraView.up.y, CameraView.up.z);

				//  get direction or heading
				Vec3 heading = WorldLocation - cam_pos;
				float camX = heading.dot(cam_right);
				float camY = heading.dot(cam_up);
				float camZ = heading.dot(cam_look);

				//  check if object is behind the camera
				if (camZ <= 0.f)
					return false;

				//  apply perspective projection
				float aspect = szScreen.x / szScreen.y;
				float fov_radians = tan(fov * 0.5f * (M_PI / 180.f)); // Convert fov to radians and compute scaling factor
				float pX = camX / (camZ * fov_radians * aspect);
				float pY = camY / (camZ * fov_radians);

				Vec2 res =
				{
					(pX + 1.0f) * 0.5f * szScreen.x,
					(1.0f - pY) * 0.5f * szScreen.y // Invert Y because screen coordinates are top-down
				};

				if (res.x <= 0.f || res.y <= 0.0f)
					return false;

				if (res.x > szScreen.x || res.y > szScreen.y)
					return false;

				*screen2D = res;

				return true;
			}
		}

		namespace Patches
		{
			void RefillAllAmmo()
			{
				__int64 eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return;

				i64_t sealAddr = 0;
				Classes::CZSealBody czSeal;
				if (!Tools::GetLocalSeal(czSeal, &sealAddr) || !sealAddr)
					return;

				const auto szWeaponArray = czSeal.MaxWeaponIndex;
				const auto& pBaseWeapon = sealAddr + offsetof(Classes::CZSealBody, pPrimaryWeapon);
				if (pBaseWeapon <= sealAddr)
					return;

				for (int i = 0; i < szWeaponArray; i++)
				{
					const auto& pWeapon = pBaseWeapon + (i * 0x4);
					if (!pWeapon)
						continue;

					const auto& wepAddr = eemem + g_PSXMemory.Read<__int32>(pWeapon);
					if (!wepAddr)
						continue;

					Classes::CZWeapon czWeapon = g_PSXMemory.Read<Classes::CZWeapon>(wepAddr);

					switch (i)
					{
					case 0 :
						for (int i = 0; i < czWeapon.defaultMags; i++)
							czSeal.PrimaryMags[i] = czWeapon.szMag;
						break;

					case 1:
						for (int i = 0; i < czWeapon.defaultMags; i++)
							czSeal.SecondaryMags[i] = czWeapon.szMag;
						break;

					case 2:
						czSeal.EqSlot1Ammo = czWeapon.szMag;
						break;

					case 3: 
						czSeal.EqSlot2Ammo = czWeapon.szMag;
						break;

					case 4: 
						czSeal.EqSlot3Ammo = czWeapon.szMag;
						break;
					
					default: break;
					}
				}

				//	overwrite section ... kinda dirty but whatever it works
				g_PSXMemory.Write<Classes::CZSealBody>(sealAddr, czSeal);
			}

			void RefillAmmo(const int& weaponIndex)
			{
				__int64 eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return;

				i64_t sealAddr = 0;
				Classes::CZSealBody czSeal;
				if (!Tools::GetLocalSeal(czSeal, &sealAddr) || !sealAddr)
					return;

				const auto szWeaponArray = czSeal.MaxWeaponIndex;
				if (weaponIndex >= szWeaponArray)
					return;

				const auto& pBaseWeapon = sealAddr + offsetof(Classes::CZSealBody, pPrimaryWeapon);
				if (pBaseWeapon <= sealAddr)
					return;

				const auto& pWeapon = pBaseWeapon + (weaponIndex * 0x4);
				if (!pWeapon)
					return;

				const auto& wepAddr = eemem + g_PSXMemory.Read<__int32>(pWeapon);
				if (!wepAddr)
					return;

				Classes::CZWeapon czWeapon = g_PSXMemory.Read<Classes::CZWeapon>(wepAddr);
				switch (weaponIndex)
				{
				case 0:
					for (int i = 0; i < czWeapon.defaultMags; i++)
						czSeal.PrimaryMags[i] = czWeapon.szMag;
					break;

				case 1:
					for (int i = 0; i < czWeapon.defaultMags; i++)
						czSeal.SecondaryMags[i] = czWeapon.szMag;
					break;

				case 2:
					czSeal.EqSlot1Ammo = czWeapon.szMag;
					break;

				case 3:
					czSeal.EqSlot2Ammo = czWeapon.szMag;
					break;

				case 4:
					czSeal.EqSlot3Ammo = czWeapon.szMag;
					break;

				default: break;
				}

				g_PSXMemory.Write<Classes::CZSealBody>(sealAddr, czSeal);
			}

			void SetWeapon(const int& weaponIndex, const Enums::EWeapon& newWeapon)
			{
				__int64 eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return;

				i64_t sealAddr = 0;
				Classes::CZSealBody czSeal;
				if (!Tools::GetLocalSeal(czSeal, &sealAddr) || !sealAddr)
					return;

				const auto szWeaponArray = czSeal.MaxWeaponIndex;
				if (weaponIndex >= szWeaponArray)
					return;

				const auto& pBaseWeapon = sealAddr + offsetof(Classes::CZSealBody, pPrimaryWeapon);
				if (pBaseWeapon <= sealAddr)
					return;

				const auto& pWeapon = pBaseWeapon + (weaponIndex * 0x4);
				if (!pWeapon)
					return;

				g_PSXMemory.Write<__int32>(pWeapon, (i32_t)newWeapon);
			}

			void SetWeaponFireMode(const int& weaponIndex, const Enums::EWeaponFireMode& newFireMode)
			{
				__int64 eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return;

				i64_t sealAddr = 0;
				Classes::CZSealBody czSeal;
				if (!Tools::GetLocalSeal(czSeal, &sealAddr) || !sealAddr)
					return;

				const auto szWeaponArray = czSeal.MaxWeaponIndex;
				if (weaponIndex >= szWeaponArray || weaponIndex >= 2)
					return;

				const auto& pBaseFireModeEntry = sealAddr + offsetof(Classes::CZSealBody, PrimaryFireMode);
				if (pBaseFireModeEntry <= sealAddr)
					return;

				const auto& pFireMode = pBaseFireModeEntry + (weaponIndex * 0x4);
				if (!pFireMode)
					return;

				g_PSXMemory.Write<Enums::EWeaponFireMode>(pFireMode, newFireMode);
			}

			void SetWeaponAmmoType(const int& weaponIndex, const Enums::EWeaponAmmo& newAmmoType)
			{
				__int64 eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return;

				i64_t sealAddr = 0;
				Classes::CZSealBody czSeal;
				if (!Tools::GetLocalSeal(czSeal, &sealAddr) || !sealAddr)
					return;

				const auto szWeaponArray = czSeal.MaxWeaponIndex;
				if (weaponIndex >= szWeaponArray)
					return;

				const auto& pBaseAmmo = sealAddr + offsetof(Classes::CZSealBody, pPrimaryAmmoType);
				if (pBaseAmmo <= sealAddr)
					return;

				const auto& pAmmo = pBaseAmmo + (weaponIndex * 0x4);
				if (!pAmmo)
					return;

				g_PSXMemory.Write<Enums::EWeaponAmmo>(pAmmo, newAmmoType);
			}

			void SetWeaponMagCount(const int& weaponIndex, const int& newMagCount)
			{

			}

			/* MISSION */
			void ForceCompleteMission()
			{
				const i64_t& eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return;

				const i64_t& pCMission = eemem + Offsets::gMission;
				if (!pCMission)
					return;

				const auto& pMission = g_PSXMemory.Read<Classes::CMission>(pCMission);
				const auto& pMissionComplete = pMission.pMissionSuccessComplete;
				if (!pMissionComplete)
					return;

				const auto& pSuccessAddr = (eemem + pMissionComplete) + 0x4;
				g_PSXMemory.Write<int>(pSuccessAddr, 1);
			}


			/* CUSTOM */
			void SetAmmoProperties(const int& weaponIndex, Classes::CZAmmo& newAmmoType)
			{
				__int64 eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return;

				i64_t sealAddr = 0;
				Classes::CZSealBody czSeal;
				if (!Tools::GetLocalSeal(czSeal, &sealAddr) || !sealAddr)
					return;

				//	get ammo type for cloning some info
				const auto& pBaseAmmoType = sealAddr + offsetof(Classes::CZSealBody, pPrimaryAmmoType);	
				if (pBaseAmmoType <= sealAddr)
					return;

				const auto& pAmmoType = pBaseAmmoType + (weaponIndex * 0x4);
				if (!pAmmoType)
					return;

				const auto& pAmmoAddr = eemem + g_PSXMemory.Read<__int32>(pAmmoType);
				if (!pAmmoAddr)
					return;

				// clone ammo type names
				Classes::CZAmmo czAmmo = g_PSXMemory.Read<Classes::CZAmmo>(pAmmoAddr);
				newAmmoType.pAmmoName = czAmmo.pAmmoName;
				newAmmoType.pDisplayName = czAmmo.pDisplayName;
				for (int i = 0; i < 4; i++)
					newAmmoType.pad_001C[i] = czAmmo.pad_001C[i];

				// empty storage for custom ammo type ( the game will empty this via game tick )
				const auto& pEmptyCharArray = sealAddr + offsetof(Classes::CZSealBody, pad_07A0) + 0x9C; 
				if (pEmptyCharArray <= sealAddr)
					return;

				//	write data
				g_PSXMemory.Write<Classes::CZAmmo>(pEmptyCharArray, newAmmoType);
			
				i32_t mCustomAmmo = pEmptyCharArray - eemem;

				g_PSXMemory.Write<i32_t>(pBaseAmmoType, mCustomAmmo);
			}
		}
	}
}

pcsx2Memory::pcsx2Memory() : exMemory()
{
	bAttached = Attach("pcsx2-qt.exe", PROCESS_ALL_ACCESS);
}

pcsx2Memory::pcsx2Memory(const std::string& name) : exMemory(name)
{
	bAttached = Attach("pcsx2-qt.exe", PROCESS_ALL_ACCESS);
}

pcsx2Memory::pcsx2Memory(const std::string& name, const DWORD& dwAccess) : exMemory(name, dwAccess)
{
	bAttached = Attach("pcsx2-qt.exe", dwAccess);
}

bool pcsx2Memory::Attach(const std::string& name, const DWORD& dwAccess)
{
	procInfo_t proc;
	if (!AttachEx(name, &proc, dwAccess))
		return false;

	vmProcess = proc;

	i64_t eemem = 0;
	if (!GetProcAddressEx(proc.hProc, proc.dwModuleBase, "EEMem", &eemem))
		return false;

	i64_t iopmem = 0;
	if (!GetProcAddressEx(proc.hProc, proc.dwModuleBase, "EEMem", &iopmem))
		return false;

	i64_t vumem = 0;
	if (!GetProcAddressEx(proc.hProc, proc.dwModuleBase, "EEMem", &vumem))
		return false;

	pcsx2Info_t pcx = reinterpret_cast<pcsx2Info_t&>(vmProcess);
	pcx.dwEEBase = ReadEx<i64_t>(pcx.hProc, eemem);
	pcx.dwIOPBase = ReadEx<i64_t>(pcx.hProc, iopmem);
	pcx.dwVUBase = ReadEx<i64_t>(pcx.hProc, vumem);
	pcxInfo = pcx;

	return pcxInfo.bAttached;
}

bool pcsx2Memory::Detach()
{
	bool result = DetachEx(vmProcess);

	pcxInfo = pcsx2Info_t();

	return result;
}

void pcsx2Memory::update()
{
	const bool& bAttched = pcxInfo.bAttached;	//	is instance attached to a process ?

	//	check if attached process is running
	//	if (!IsProcessRunning(pcxInfo.mProcName))
	//	{
	//		Detach();	//	close handles and free resources if not already done ( safe to call multiple times if nothing is attached )
	//		return;
	//	}

	//	attached process is running, update process information


	//  attempt to get main process window
	EnumWindowData eDat;
	eDat.procId = pcxInfo.dwPID;
	if (EnumWindows(GetProcWindowEx, reinterpret_cast<LPARAM>(&eDat)))
		pcxInfo.hWnd = eDat.hwnd;

	//  Get window title
	char buffer[MAX_PATH];
	if (pcxInfo.hWnd && GetWindowTextA(pcxInfo.hWnd, buffer, MAX_PATH))
		pcxInfo.mWndwTitle = std::string(buffer);

	vmProcess = reinterpret_cast<procInfo_t&>(pcxInfo);
}

bool pcsx2Memory::GetPSXAddress(const unsigned int& offset, i64_t* lpResult)
{

}

i64_t pcsx2Memory::GetPSXAddress(const unsigned int& offset)
{

}

i64_t pcsx2Memory::ReadPSXPointerChain(const i64_t& addr, std::vector<unsigned int>& offsets, i64_t* lpResult)
{

}

void SOCOM::Update()
{
	using namespace Engine::zdb;

	static auto reset = [this](){ this->imCache = SGlobals(); };

	SGlobals globals;
	auto& game = globals.game;
	auto& player = globals.localPlayer;

	__int64 eemem = g_PSXMemory.GetEEMemory();
	if (!eemem)
		return reset();

	//	GET LOCAL PLAYER
	auto pLocalPlayer = g_PSXMemory.Read<__int32>(eemem + Offsets::gLocalSeal);;
	if (!pLocalPlayer)
		return reset();

	auto localSeal = g_PSXMemory.Read<Classes::CZSealBody>(eemem + pLocalPlayer);
	player.pAddr = pLocalPlayer;
	player.pos = localSeal.origin;
	player.seal = localSeal;
	if (!g_PSXMemory.ReadString(eemem + localSeal.pName, player.name, 32))
		return reset();

	//	GET PLAYERS
	std::vector<SImGuiPlayer> players;
	std::vector<Classes::CZSealBody> seals;
	if (Tools::GetPlayers(&seals))
	{
		players.reserve(seals.size());
		for (auto& ent : seals)
		{
			SImGuiPlayer imPlayer; 

			if (ent.pName == localSeal.pName)
				continue;	//	skip local player

			imPlayer.pos = ent.origin;
			imPlayer.health = ent.Health * 100.f;
			imPlayer.bAlive = (imPlayer.health > 0.f);
			imPlayer.stance = ent.Stance;

			if (!g_PSXMemory.ReadString(eemem + ent.pName, imPlayer.name, 32))
				continue;

			players.push_back(imPlayer);
		}
	}
	game.bInGame = seals.size() > 0;
	game.playerCount = players.size();
	globals.render = players;

	//	GET CAMERA VIEW
	if (!Tools::GetCameraViewMatrix(globals.cameraView))
		return reset();

	//	GET CAMERA MATRIX
	if (!Tools::GetCameraMatrixSet(globals.mtxSet))
		return reset();

	globals.bValid = true;
	imCache = globals;
}

void SOCOM::ShutDown()
{
	// disable any enabled patches

	g_PSXMemory.Detach();
}