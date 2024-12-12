#pragma once
#include "engine.h"
#include <Memory/Memory.h>

namespace Engine
{
	namespace SOCOM1
	{
		namespace Classes
		{
			class CZCamera
			{
			public:
				char							pad_0000[52];				//0x0000
				int								pCameraMatrix;				//0x0034
			};

			class CZSeal
			{
			public:
				char							pad_0000[20];				//0x0000
				__int32							m_pName;					//0x0014
				char							pad_0018[4];				//0x0018
				Vec3							m_relativeLocation;			//0x001C
			};
		}

		namespace Offsets
		{
			constexpr auto o_LocalCamera{ 0x48D488 };	//	415FF0	: SOCOM2
			constexpr auto o_LocalSeal{ 0x48D548 };		//	440C38	: SOCOM2
			constexpr auto o_SealArray{ 0x4D46A0 };		//	4362E4	: SOCOM2
		}

		namespace Tools
		{
			bool GetCameraViewMatrix(ZViewModel& CameraView);
			bool GetPlayers(std::vector<Classes::CZSeal>* players);
		}
	}

	namespace SOCOM2
	{
		namespace Classes
		{
			class CZCamera
			{
			public:
				char							pad_0000[180];				//0x0000
				int								pCameraMatrix;				//0x00B4
			};

			class CZSeal
			{
			public:
				char							pad_0000[20];				//0x0000
				__int32							m_pName;					//0x0014
				char							pad_0018[4];				//0x0018
				Vec3							m_relativeLocation;			//0x001C
			};
		}

		namespace Offsets
		{
			constexpr auto o_LocalCamera{ 0x415FF0 };	
			constexpr auto o_LocalSeal{ 0x440C38 };		
			constexpr auto o_SealArray{ 0x4362E0 };		
		}

		namespace Tools
		{
			bool GetCameraViewMatrix(ZViewModel& CameraView);
			bool GetPlayers(std::vector<Classes::CZSeal>* players);
		}
	}

	//	@TODO:
	namespace SOCOM3
	{
		namespace Classes
		{
			class CZCamera
			{
			public:
			};

			class CZSeal
			{
			public:
			};
		}

		namespace Offsets
		{
			constexpr auto o_LocalCamera{ 0x0 };	
			constexpr auto o_LocalSeal{ 0x0 };
			constexpr auto o_SealArray{ 0x0 };
		}

		namespace Tools
		{
			bool GetCameraViewMatrix(ZViewModel& CameraView);
			bool GetPlayers(std::vector<Classes::CZSeal>* players);
		}
	}

	//	@TODO:
	namespace SOCOMCA
	{
		namespace Classes
		{
			class CZCamera
			{
			public:
			};

			class CZSeal
			{
			public:
			};
		}

		namespace Offsets
		{
			constexpr auto o_LocalCamera{ 0x0 };	//	415FF0	: SOCOM2
			constexpr auto o_LocalSeal{ 0x0 };		//	440C38	: SOCOM2
			constexpr auto o_SealArray{ 0x0 };		//	4362E4	: SOCOM2
		}

		namespace Tools
		{
			bool GetCameraViewMatrix(ZViewModel& CameraView);
			bool GetPlayers(std::vector<Classes::CZSeal>* players);
		}
	}
}

class SOCOM
{
public:
	struct SImGuiPlayer
	{
		__int64 pAddr{ 0 };
		std::string name;
		Engine::Vec3 pos;
	};

	struct SGame
	{
		bool bInGame{ false };
		__int32 playerCount{ 0 };
	};

	struct SLocalPlayer
	{
		__int64 pAddr{ 0 };
		std::string name;
		Engine::Vec3 pos;
	};

	struct SGlobals
	{
		bool bValid{ false };
		SGame game;
		SLocalPlayer localPlayer;
		Engine::ZViewModel cameraView;
		std::vector<SImGuiPlayer> render;
	};

public:
	SGlobals imCache;

public:
	void Update();

}; inline std::unique_ptr<SOCOM> g_SOCOM;