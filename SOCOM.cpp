#include "SOCOM.h"

namespace Engine
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

	//	 is kinda goofy ... please help me fix
	bool Tools::ProjectWorldToScreen(Vec3 WorldLocation, ZViewModel CameraView, float fov, Vec2 szScreen, Vec2* screen2D)
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

void SOCOM::Update()
{
	static auto reset = [this](){ this->imCache = SGlobals(); };

	SGlobals globals;
	auto& game = globals.game;
	auto& player = globals.localPlayer;

	__int64 eemem = Memory::GetEEMemory();
	if (!eemem)
		return reset();

	//	GET LOCAL PLAYER
	auto pLocalPlayer = Memory::Read<__int32>(eemem + Engine::Offsets::o_LocalSeal);;
	if (!pLocalPlayer)
		return reset();

	auto localSeal = Memory::Read<Engine::Classes::CZSeal>(eemem + pLocalPlayer);
	player.pAddr = pLocalPlayer;
	player.pos = localSeal.m_relativeLocation;
	if (!Memory::ReadVirtualString(eemem + localSeal.m_pName, player.name, 32))
		return reset();

	//	GET PLAYERS
	std::vector<SImGuiPlayer> players;
	std::vector<Engine::Classes::CZSeal> seals;
	if (Engine::Tools::GetPlayers(&seals))
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
	if (!Engine::Tools::GetCameraViewMatrix(globals.cameraView))
		return reset();

	globals.bValid = true;
	imCache = globals;
}