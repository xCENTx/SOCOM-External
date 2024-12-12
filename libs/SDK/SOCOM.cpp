#include "SOCOM.h"

namespace Engine
{
	namespace SOCOM1
	{
		bool Tools::GetCameraViewMatrix(ZViewModel& CameraView)
		{
			__int64 eemem = Memory::GetEEMemory();
			if (!eemem)
				return false;

			auto pCamera = Memory::Read<__int32>(eemem + Offsets::o_LocalCamera);
			if (!pCamera)
				return false;

			auto camera = Memory::Read<Classes::CZCamera>(eemem + pCamera);
			if (!camera.pCameraMatrix)
				return false;

			CameraView = Memory::Read<ZViewModel>(eemem + camera.pCameraMatrix);

			return true;
		}

		bool Tools::GetPlayers(std::vector<Classes::CZSeal>* players)
		{
			std::vector<Classes::CZSeal> seals;

			__int64 eemem = Memory::GetEEMemory();
			if (!eemem)
				return false;

			auto sealArray = Memory::Read<ZArray>(eemem + Offsets::o_SealArray);
			if (sealArray.count <= 1 || sealArray.begin <= 0 || sealArray.end <= 0)
				return false;


			auto it = Memory::Read<ZIterator>(eemem + sealArray.begin);
			auto end = Memory::Read<ZIterator>(eemem + it.prev);
			do
			{
				auto data = it.data;
				if (data > 0)
				{
					auto seal = Memory::Read<Classes::CZSeal>(eemem + data);
					if (seal.m_pName)
						seals.push_back(seal);
				}

				it = Memory::Read<ZIterator>(eemem + it.next);

			} while (it.data != end.data);

			*players = seals;

			return players->size() > 0;
		}
	}

	namespace SOCOM2
	{
		bool Tools::GetCameraViewMatrix(ZViewModel& CameraView)
		{
			__int64 eemem = Memory::GetEEMemory();
			if (!eemem)
				return false;

			auto pCamera = Memory::Read<__int32>(eemem + Offsets::o_LocalCamera);
			if (!pCamera)
				return false;

			auto camera = Memory::Read<Classes::CZCamera>(eemem + pCamera);
			if (!camera.pCameraMatrix)
				return false;

			CameraView = Memory::Read<ZViewModel>(eemem + camera.pCameraMatrix);

			return true;
		}

		bool Tools::GetPlayers(std::vector<Classes::CZSeal>* players)
		{
			std::vector<Classes::CZSeal> seals;

			__int64 eemem = Memory::GetEEMemory();
			if (!eemem)
				return false;

			auto sealArray = Memory::Read<ZArray>(eemem + Offsets::o_SealArray);
			if (sealArray.count <= 1 || sealArray.begin <= 0 || sealArray.end <= 0)
				return false;


			auto it = Memory::Read<ZIterator>(eemem + sealArray.begin);
			auto end = Memory::Read<ZIterator>(eemem + it.prev);
			do
			{
				auto data = it.data;
				if (data > 0)
				{
					auto seal = Memory::Read<Classes::CZSeal>(eemem + data);
					if (seal.m_pName)
						seals.push_back(seal);
				}

				it = Memory::Read<ZIterator>(eemem + it.next);

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

void SOCOM::Update()
{
	using namespace Engine::SOCOM2;

	static auto reset = [this](){ this->imCache = SGlobals(); };

	SGlobals globals;
	auto& game = globals.game;
	auto& player = globals.localPlayer;

	__int64 eemem = Memory::GetEEMemory();
	if (!eemem)
		return reset();

	//	GET LOCAL PLAYER
	auto pLocalPlayer = Memory::Read<__int32>(eemem + Offsets::o_LocalSeal);;
	if (!pLocalPlayer)
		return reset();

	auto localSeal = Memory::Read<Classes::CZSeal>(eemem + pLocalPlayer);
	player.pAddr = pLocalPlayer;
	player.pos = localSeal.m_relativeLocation;
	if (!Memory::ReadVirtualString(eemem + localSeal.m_pName, player.name, 32))
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
			if (!Memory::ReadVirtualString(eemem + ent.m_pName, imPlayer.name, 32))
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