#include "SOCOM.h"

namespace Engine
{
	namespace SOCOM1
	{
		bool Tools::GetCameraViewMatrix(ZViewModel& CameraView)
		{
			__int64 eemem = g_PSXMemory.GetEEMemory();
			if (!eemem)
				return false;

			auto pCamera = g_PSXMemory.Read<__int32>(eemem + Offsets::o_LocalCamera);
			if (!pCamera)
				return false;

			auto camera = g_PSXMemory.Read<Classes::CZCamera>(eemem + pCamera);
			if (!camera.pCameraMatrix)
				return false;
			
			CameraView = g_PSXMemory.Read<ZViewModel>(eemem + camera.pCameraMatrix);

			return true;
		}

		bool Tools::GetModelViewMatrix(Matrix4x4& ModelView)
		{
			__int64 eemem = g_PSXMemory.GetEEMemory();
			if (!eemem)
				return false;

			auto pCamera = g_PSXMemory.Read<__int32>(eemem + Offsets::o_LocalCamera);
			if (!pCamera)
				return false;

			auto camera = g_PSXMemory.Read<Classes::CZCamera>(eemem + pCamera);
			if (!camera.pCameraMatrix)
				return false;

			ModelView = g_PSXMemory.Read<Matrix4x4>(eemem + camera.pCameraMatrix);

			return true;
		}

		bool Tools::GetPlayers(std::vector<Classes::CZSeal>* players)
		{
			std::vector<Classes::CZSeal> seals;

			__int64 eemem = g_PSXMemory.GetEEMemory();
			if (!eemem)
				return false;

			auto sealArray = g_PSXMemory.Read<ZArray>(eemem + Offsets::o_SealArray);
			if (sealArray.count <= 1 || sealArray.begin <= 0 || sealArray.end <= 0)
				return false;


			auto it = g_PSXMemory.Read<ZIterator>(eemem + sealArray.begin);
			auto end = g_PSXMemory.Read<ZIterator>(eemem + it.prev);
			do
			{
				auto data = it.data;
				if (data > 0)
				{
					auto seal = g_PSXMemory.Read<Classes::CZSeal>(eemem + data);
					if (seal.m_pName)
						seals.push_back(seal);
				}

				it = g_PSXMemory.Read<ZIterator>(eemem + it.next);

			} while (it.data != end.data);

			*players = seals;

			return players->size() > 0;
		}
	}

	namespace SOCOM2
	{
		bool Tools::GetCameraViewMatrix(ZViewModel& CameraView)
		{
			__int64 eemem = g_PSXMemory.GetEEMemory();
			if (!eemem)
				return false;

			auto pCamera = g_PSXMemory.Read<__int32>(eemem + Offsets::o_LocalCamera);
			if (!pCamera)
				return false;

			auto camera = g_PSXMemory.Read<Classes::CZCamera>(eemem + pCamera);
			if (!camera.pCameraMatrix)
				return false;

			CameraView = g_PSXMemory.Read<ZViewModel>(eemem + camera.pCameraMatrix);

			return true;
		}

		bool Tools::GetPlayers(std::vector<Classes::CZSeal>* players)
		{
			std::vector<Classes::CZSeal> seals;

			__int64 eemem = g_PSXMemory.GetEEMemory();
			if (!eemem)
				return false;

			auto sealArray = g_PSXMemory.Read<ZArray>(eemem + Offsets::o_SealArray);
			if (sealArray.count <= 1 || sealArray.begin <= 0 || sealArray.end <= 0)
				return false;


			auto it = g_PSXMemory.Read<ZIterator>(eemem + sealArray.begin);
			auto end = g_PSXMemory.Read<ZIterator>(eemem + it.prev);
			do
			{
				auto data = it.data;
				if (data > 0)
				{
					auto seal = g_PSXMemory.Read<Classes::CZSeal>(eemem + data);
					if (seal.m_pName)
						seals.push_back(seal);
				}

				it = g_PSXMemory.Read<ZIterator>(eemem + it.next);

			} while (it.data != end.data);

			*players = seals;

			return players->size() > 0;
		}
	}

	namespace SOCOM3
	{
		bool Tools::GetCameraViewMatrix(ZViewModel& CameraView)
		{
			return false;
		}

		bool Tools::GetPlayers(std::vector<Classes::CZSeal>* players)
		{
			return false;
		}
	}

	namespace SOCOMCA
	{
		bool Tools::GetCameraViewMatrix(ZViewModel& CameraView)
		{
			return false;
		}

		bool Tools::GetPlayers(std::vector<Classes::CZSeal>* players)
		{
			return false;
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
	using namespace Engine::SOCOM2;

	static auto reset = [this](){ this->imCache = SGlobals(); };

	SGlobals globals;
	auto& game = globals.game;
	auto& player = globals.localPlayer;

	__int64 eemem = g_PSXMemory.GetEEMemory();
	if (!eemem)
		return reset();

	//	GET LOCAL PLAYER
	auto pLocalPlayer = g_PSXMemory.Read<__int32>(eemem + Offsets::o_LocalSeal);;
	if (!pLocalPlayer)
		return reset();

	auto localSeal = g_PSXMemory.Read<Classes::CZSeal>(eemem + pLocalPlayer);
	player.pAddr = pLocalPlayer;
	player.pos = localSeal.m_relativeLocation;
	if (!g_PSXMemory.ReadString(eemem + localSeal.m_pName, player.name, 32))
		return reset();

	//	GET PLAYERS
	std::vector<SImGuiPlayer> players;
	std::vector<Classes::CZSeal> seals;
	if (Tools::GetPlayers(&seals))
	{
		players.reserve(seals.size());
		for (auto& ent : seals)
		{
			SImGuiPlayer imPlayer; 

			if (ent.m_pName == localSeal.m_pName)
				continue;	//	skip local player

			imPlayer.pos = ent.m_relativeLocation;
			imPlayer.health = ent.m_Health * 100.f;
			imPlayer.bAlive = (imPlayer.health > 0.f);

			if (!g_PSXMemory.ReadString(eemem + ent.m_pName, imPlayer.name, 32))
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

	globals.bValid = true;
	imCache = globals;
}

void SOCOM::ShutDown()
{
	// disable any enabled patches

	g_PSXMemory.Detach();
}