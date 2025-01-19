#pragma once
#include "engine.h"
#include <Memory/exMemory.hpp>

struct pcsx2PROCESSINFO : public PROCESSINFO64
{
	i64_t dwEEBase{ 0 };		//	
	i64_t dwIOPBase{ 0 };		//	
	i64_t dwVUBase{ 0 };		//	
}; typedef pcsx2PROCESSINFO pcsx2Info_t;

class pcsx2Memory : public exMemory
{
public:
	explicit pcsx2Memory();
	explicit pcsx2Memory(const std::string& name);
	explicit pcsx2Memory(const std::string& name, const DWORD& dwAccess);

public:
	/**/
	virtual bool Attach(const std::string& name, const DWORD& dwAccess = PROCESS_ALL_ACCESS) override;

	/**/
	virtual bool Detach() override;

	/**/
	virtual void update() override;

private:
	pcsx2Info_t pcxInfo;

public:
	/**/
	inline const pcsx2Info_t& GetPsxInfo() const { return pcxInfo; }

	/**/
	inline const i64_t& GetEEMemory() const { return pcxInfo.dwEEBase; }

	/**/
	inline const i64_t& GetIOPMemory() const { return pcxInfo.dwIOPBase; }

	/**/
	inline const i64_t& GetVUMemory() const { return pcxInfo.dwVUBase; }

public:
	/**/
	inline bool GetPSXAddress(const unsigned int& offset, i64_t* lpResult);
	
	/**/
	inline i64_t GetPSXAddress(const unsigned int& offset);
	
	/**/
	inline i64_t ReadPSXPointerChain(const i64_t& addr, std::vector<unsigned int>& offsets, i64_t* lpResult = nullptr);
};

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
				__int32							m_pSealObj;					//0x0028	//	CZSealObject
				char							pad_002C[148];				//0x002C
				__int32							m_pSealCtrl;				//0x00C0	//	CZSealCtrl
				__int32							m_teamID;					//0x00C4	//	ETEAM
				char							pad_00C8[152];				//0x00C8
				__int8							m_zoomPos;					//0x0160
				char							pad_0161[3];				//0x0161
				float							m_fZoom;					//0x0164
				char							pad_0168[704];				//0x0168
				Vec3							m_relativeRotation;			//0x0428
				char							pad_0434[168];				//0x0434
				__int32							m_totalShotsFired;			//0x04DC
				char							pad_04E0[48];				//0x04E0
				__int32							m_totalShotsFired2;			//0x0510
				char							pad_0514[56];				//0x0514
				float							m_xhairBounce;				//0x054C
				char							pad_0550[188];				//0x0550
				__int32							m_PrimaryWeapon;			//0x060C
				__int32							m_SecondaryWeapon;			//0x0610
				__int32							m_EqSlot1;					//0x0614
				__int32							m_EqSlot2;					//0x0618
				__int32							m_EqSlot3;					//0x061C
				char							pad_0620[100];				//0x0620
				__int32							m_PrimaryAmmoType;			//0x0684
				__int32							m_SecondaryAmmoType;		//0x0688
				__int32							m_EqSlot1AmmoType;			//0x068C
				__int32							m_EqSlot2AmmoType;			//0x0690
				__int32							m_EqSlot3AmmoType;			//0x0694
				char							pad_0698[100];				//0x0698
				__int32							m_PrimaryMags[10];			//0x06FC
				__int32							m_SecondaryMags[10];		//0x0724
				__int32							m_EqSlot1Ammo;				//0x074C
				char							pad_0750[36];				//0x0750
				__int32							m_EqSlot2Ammo;				//0x0774
				char							pad_0778[36];				//0x0778
				__int32							m_EqSlot3Ammo;				//0x079C
				char							pad_07A0[1156];				//0x07A0
				__int32							m_primaryFireType;			//0x0C24	//	EFIRETYPE
				__int32							m_secondaryFireType;		//0x0C28	//	EFIRETYPE
				char							pad_0C2C[272];				//0x0C2C
				__int32							m_curShotsFired;			//0x0D3C
				float							m_fireCooldown;				//0x0D40
				char							pad_0D44[84];				//0x0D44
				__int32							m_totalShotsFired3;			//0x0D98
				char							pad_0D9C[108];				//0x0D9C
				__int32							m_gunHot;					//0x0E08
				char							pad_0E0C[120];				//0x0E0C
				__int32							m_gunHot2;					//0x0E84
				char							pad_0E88[72];				//0x0E88
				float							m_Health;					//0x0ED0
				char							pad_0ED4[1180];				//0x0ED4
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
			bool GetModelViewMatrix(Matrix4x4& ModelView);
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
		bool bAlive{ false };
		__int64 pAddr{ 0 };
		float health{ 0.0f };
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
		Engine::Matrix4x4 mvmatrix;
		std::vector<SImGuiPlayer> render;
	};

public:
	SGlobals imCache;

public:
	void Update();
	void ShutDown();

}; 

inline pcsx2Memory g_PSXMemory = pcsx2Memory("pcsx2-qt.exe");
inline std::unique_ptr<SOCOM> g_SOCOM;