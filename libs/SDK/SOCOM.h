#pragma once
#include "engine.h"
#include <Memory/exMemory.hpp>

#ifdef _WIN64
typedef unsigned __int32  i32_t;
#endif

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
	namespace zdb
	{
		/* WEAPONS */
#define WEAPON_AR_M4A1 0xCC3CA0
#define WEAPON_AR_M4A1_SD 0xCC3F80
#define WEAPON_AR_M4A1_M203 0xCC4270
#define WEAPON_AR_M16A2 0xCC36E0
#define WEAPON_AR_M16A2_M203 0xCC39C0
#define WEAPON_AR_552 0xCC4550
#define WEAPON_AR_AK74 0xCC4850
#define WEAPON_AR_AKS74 0xCC4B20
#define WEAPON_AR_M14 0xCC4E00
#define WEAPON_SMG_UZI 0xCC61F0
#define WEAPON_SMG_HK5 0xCC5970
#define WEAPON_SMG_HK5_SD 0xCC5C40
#define WEAPON_SMG_F90 0xCC5F20
#define WEAPON_LMG_M60E3 0xCC50D0
#define WEAPON_LMG_M63A 0xCC53A0
#define WEAPON_SR_SR25 0xCC7080
#define WEAPON_SR_SR25_SD 0xCC6D90
#define WEAPON_SR_M40A1 0xCC67A0
#define WEAPON_SR_M82A1A 0xCC64C0
#define WEAPON_SR_M87ELR 0xCC6AA0
#define WEAPON_GL_M79 0xCCA6E0
#define WEAPON_GL_MGL 0xCCA410
#define WEAPON_HG_9MM 0xCC2B60
#define WEAPON_HG_P226 0xCC25B0
#define WEAPON_HG_M18 0xCC1730
#define WEAPON_HG_F57 0xCC1CF0
#define WEAPON_HG_DE50 0xCC1FD0
#define WEAPON_HG_MK23 0xCC1A10
#define WEAPON_HG_MK23SD 0xCC2880
#define WEAPON_EQ_M67 0xCC81A0
#define WEAPON_EQ_HE 0xCC8490
#define WEAPON_EQ_SMOKE 0xCC8760
#define WEAPON_EQ_M141 0xCC8A60
#define WEAPON_EQ_M79HE 0xCCA9B0
#define WEAPON_EQ_M79FRAG 0xCCAC90
#define WEAPON_EQ_M79SMOKE 0xCCAF70
#define WEAPON_EQ_M203HE 0xCC9B50
#define WEAPON_EQ_M203FRAG 0xCC9E30
#define WEAPON_EQ_M203SMOKE 0xCCA120
#define WEAPON_EQ_C4 0xCC9300
#define WEAPON_EQ_CLAYMORE 0xCC9030
#define WEAPON_EQ_SATCHEL 0xCC8D50
#define WEAPON_EQ_AMMOx2 0xCCB800

		/* AMMO TYPES */
#define AMMO_556x45_SD 0xCC0ED0		//	M4A1_SD , 
#define AMMO_556x45 0xCC0F40		//	M4A1, M16A2, 552 , M63A
#define AMMO_762x39 0xCC1050		//	AK47 , 
#define AMMO_545x39 0xCC0E60		//	AKS74
#define AMMO_9x19p 0xCC0C90			//	SMG_HK5, SMG_9MM, PISTOL_MODEL_18, PISTOL_9MM
#define AMMO_9x19p_SD 0xCC0CD0		//	SMG_HK5_SD, PISTOL_9MM_SD
#define AMMO_45ACP 0xCC0DC0			//	PISTOL_MK23
#define AMMO_762x51 0xCC0F90		//	M40A1 , M14 , M60E3 , SR25
#define AMMO_50AE 0xCC0E10			//	DE50 , 
#define AMMO_57x28 0xCC0D20			//	F57 , F90
#define AMMO_50BMG 0xCC10C0			//	M82A1A , M87ELR
#define AMMO_2xAmmo 0x0
#define AMMO_Satchel 0xCC12C0
#define AMMO_M67 0xCC1160
#define AMMO_M141 0xCC1260
#define AMMO_Smoke 0xCC1210
#define AMMO_Claymore 0xCC1320
#define AMMO_C4 0xCC1380
#define AMMO_M79_HE 0xCC1490
#define AMMO_M79_FRAG 0xCC15C0
#define AMMO_M79_SMOKE 0xCC1620
#define AMMO_M203_HE 0xCC1420
#define AMMO_M203_FRAG 0xCC1490
#define AMMO_M203_SMOKE 0xCC1500

		namespace Offsets
		{
			constexpr auto gAppCamera{ 0x48D488 };		//	black label
			constexpr auto gLocalSeal{ 0x48D548 };		//	black label
			constexpr auto gEntityArray{ 0x4D46A0 };	//	black label
			constexpr auto gMission{ 0x4D4880 };		//	black label
			constexpr auto gCamera{ 0x51E778 };			//	black label
		}

		namespace Enums
		{
			enum EZoomState : char
			{
				EZoomState_default = 0,
				EZoomState_1stperson = 1,
				EZoomState_unknown = 2,
				EZoomState_binocs = 3,
				EZoomState_weapondef_1 = 4
			};

			enum class ESealTeam : i32_t
			{
				ETeam_SEALS = 0x40000001,		//	Seal
				ETeam_TERRORIST = 0x80000100,		//	Terrorist
				ETeam_TURRET = 0x48000000,		//	Turret
				ETeam_SPECTATOR = 0x00010000,		//	Spectator

				// CAMPAIGN
				ETeam_SP_ABLE = 0x84000006,			//	Alpha Team
				ETeam_SP_BRAVO = 0x8400000A,		//	Bravo Team
				ETeam_SP_ENEMY_A = 0x40000050,		//	Iron Brother / Iron Leader
				ETeam_SP_ENEMY_B = 0x40000100,		//	
				ETeam_SP_ENEMY_C = 0x40000210,		//	
				ETeam_SP_ENEMY_D = 0x40000410,		//	
				ETeam_SP_ENEMY_E = 0x40000810,		//	
				ETeam_SP_ENEMY_F = 0x40001010,		//	
				ETeam_SP_ENEMY_G = 0x40002010,		//	
				ETeam_SP_ENEMY_H = 0x40004010,		//	
				ETeam_SP_ENEMY_I = 0x40021010,		//	
				ETeam_NONE = 0
			};

			enum ESealStance : char
			{
				ESealStance_Stand = 0,
				ESealStance_Crouch = 1,
				ESealStance_Prone = 2
			};

			enum EWeaponIndex : __int32
			{
				EWeaponIndex_Primary = 0,
				EWeaponIndex_Secondary = 1,
				EWeaponIndex_EqSlot1 = 2,
				EWeaponIndex_EqSlot2 = 3,
				EWeaponIndex_EqSlot3 = 4
			};

			enum EWeaponFireMode : __int32
			{
				EFireMode_Default = 0,
				EFireMode_Single = 1,
				EFireMode_Burst = 2,
				EFireMode_Auto = 3
			};

			enum class EWeapon : i32_t
			{
				EWeapon_P_AR_M4A1_SD = WEAPON_AR_M4A1_SD,
				EWeapon_P_AR_M4A1_M203 = WEAPON_AR_M4A1_M203,
				EWeapon_P_AR_M16A2 = WEAPON_AR_M16A2,
				EWeapon_P_AR_M16A2_M203 = WEAPON_AR_M16A2_M203,
				EWeapon_P_AR_552 = WEAPON_AR_552,
				EWeapon_P_AR_AK74 = WEAPON_AR_AK74,
				EWeapon_P_AR_AKS74 = WEAPON_AR_AKS74,
				EWeapon_P_SMG_UZI = WEAPON_SMG_UZI,
				EWeapon_P_SMG_HK5 = WEAPON_SMG_HK5,
				EWeapon_P_SMG_HK5_SD = WEAPON_SMG_HK5_SD,
				EWeapon_P_SNIPER_M40A1 = WEAPON_SR_M40A1,
				EWeapon_P_SNIPER_M87ELR = WEAPON_SR_M87ELR,
				EWeapon_P_GL_M79 = WEAPON_GL_M79,
				EWeapon_P_GL_MGL = WEAPON_GL_MGL,
				EWeapon_S_PISTOL_P226 = WEAPON_HG_P226,
				EWeapon_S_PISTOL_F57 = WEAPON_HG_F57,
				EWeapon_S_PISTOL_MODEL18 = WEAPON_HG_M18,
				EWeapon_S_PISTOL_DE50 = WEAPON_HG_DE50,
				EWeapon_S_PISTOL_MK23 = WEAPON_HG_MK23,
				EWeapon_S_PISTOL_MK23_SD = WEAPON_HG_MK23SD,
				EWeapon_EQ_M67 = WEAPON_EQ_M67,
				EWeapon_EQ_HE = WEAPON_EQ_HE,
				EWeapon_EQ_SMOKE = WEAPON_EQ_SMOKE,
				EWeapon_EQ_M141 = WEAPON_EQ_M141,
				EWeapon_EQ_AMMOx2 = WEAPON_EQ_AMMOx2,
				EWeapon_EQ_M79HE = WEAPON_EQ_M79HE,
				EWeapon_EQ_M79FRAG = WEAPON_EQ_M79FRAG,
				EWeapon_EQ_M79SMOKE = WEAPON_EQ_M79SMOKE,
				EWeapon_EQ_M203HE = WEAPON_EQ_M203HE,
				EWeapon_EQ_M203FRAG = WEAPON_EQ_M203FRAG,
				EWeapon_EQ_M203SMOKE = WEAPON_EQ_M203SMOKE,
				EWeapon_EQ_FLASHBANG = WEAPON_EQ_M141,
				EWeapon_EQ_C4 = WEAPON_EQ_C4,
				EWeapon_EQ_CLAYMORE = WEAPON_EQ_CLAYMORE,
				EWeapon_EQ_SATCHEL = WEAPON_EQ_SATCHEL
			};

			enum class EWeaponAmmo : i32_t
			{
				EWeaponAmmo_EMPTY = 0,

				EWeaponAmmo_M4A1 = AMMO_556x45,
				EWeaponAmmo_M4A1_M203 = AMMO_556x45,
				EWeaponAmmo_M4A1_SD = AMMO_556x45_SD,
				EWeaponAmmo_M16A2 = AMMO_556x45,
				EWeaponAmmo_M16A2_M203 = AMMO_556x45,
				EWeaponAmmo_552 = AMMO_556x45,
				EWeaponAmmo_AK74 = AMMO_762x39,
				EWeaponAmmo_AKS74 = AMMO_545x39,
				EWeaponAmmo_M14 = AMMO_762x51,

				EWeaponAmmo_UZI = AMMO_9x19p,
				EWeaponAmmo_HK5 = AMMO_9x19p,
				EWeaponAmmo_HK5_SD = AMMO_9x19p_SD,
				EWeaponAmmo_F90 = AMMO_57x28,
				
				EWeaponAmmo_M60E3 = AMMO_762x51,
				EWeaponAmmo_M63A = AMMO_556x45,
				
				EWeaponAmmo_SR25 = AMMO_762x51,
				EWeaponAmmo_SR25_SD = AMMO_762x51,
				EWeaponAmmo_M40A1 = AMMO_762x51,
				EWeaponAmmo_M82A1A = AMMO_50BMG,
				EWeaponAmmo_M87ELR = AMMO_50BMG,
				
				EWeaponAmmo_M79 = AMMO_2xAmmo,
				EWeaponAmmo_MGL = AMMO_2xAmmo,
				
				EWeaponAmmo_9MM = AMMO_9x19p,
				EWeaponAmmo_P226 = AMMO_9x19p,
				EWeaponAmmo_M18 = AMMO_9x19p,
				EWeaponAmmo_F57 = AMMO_57x28,
				EWeaponAmmo_DE50 = AMMO_50AE,
				EWeaponAmmo_MK23 = AMMO_45ACP,
				EWeaponAmmo_MK23_SD = AMMO_45ACP,
				
				EWeaponAmmo_M67 = AMMO_M67,
				EWeaponAmmo_HE = AMMO_M141,
				EWeaponAmmo_M141 = AMMO_M141,
				EWeaponAmmo_Smoke = AMMO_Smoke,
				EWeaponAmmo_Claymore = AMMO_Claymore,
				EWeaponAmmo_C4 = AMMO_C4,
				
				EWeaponAmmo_M79_HE = AMMO_M79_HE,
				EWeaponAmmo_M79_FRAG = AMMO_M79_FRAG,
				EWeaponAmmo_M79_SMOKE = AMMO_M79_SMOKE,
				EWeaponAmmo_M203_HE = AMMO_M203_HE,
				EWeaponAmmo_M203_FRAG = AMMO_M203_FRAG,
				EWeaponAmmo_M203_SMOKE = AMMO_M203_SMOKE,

				EWeaponAmmo_Satchel = AMMO_Satchel,
				EWeaponAmmo_2xAmmo = AMMO_2xAmmo,
			};
		}

		namespace Structs
		{
			using namespace Enums;

			struct SCameraFrustrum
			{
				Vec3 m_frustum[3];    //0x0000
				Vec3 m_fullfrustum[6];    //0x0024
				i32_t m_full_frustum_points;    //0x006C
			};    //Size: 0x0070

			struct tag_ZCAM_MTX_SET
			{
				Matrix4x4 mtxWorldToView;    //0x0000
				Matrix4x4 mtxWorldToClip;    //0x0040
				Matrix4x4 mtxViewToClip;    //0x0080
				Matrix4x4 mtxViewToScreen;    //0x00C0
			};    //Size: 0x0100

			struct ZCAM_CLIP_DATA
			{
				Vec4 ViewPlanePoint;    //0x0000
				Vec4 NearPlanePoint;    //0x0010
				Vec4 NearPlaneNormal;    //0x0020
				Vec4 LeftPlaneNormal;    //0x0030
				Vec4 RightPlaneNormal;    //0x0040
				Vec4 TopPlaneNormal;    //0x0050
				Vec4 BottompPlaneNormal;    //0x0060
			};    //Size: 0x0070

			struct ZViewModel
			{
				Vec4 right;		//	Vector3 : right vector
				Vec4 up;		//	Vector3 : up vector
				Vec4 fwd;		//	Vector3 : look vector ( - is forward )
				Vec4 pos;		//	Vector3 : position

				Vec3 origin() const;
			};

			struct ZIterator
			{
				i32_t next;	//	pointer to next position in array
				i32_t prev;	//	pointer to last position in array
				i32_t data;	//	pointer to object
			};

			struct ZArray
			{
				__int32 count{ 0 };	//	objects in array
				__int32 begin{ 0 };	//	pointer to first object in the array
				__int32 end{ 0 };	//	pointer to last object in the array
			};
		}

		namespace Classes
		{
			using namespace Structs;

			class CMission
			{
			public:
				char							pad_0000[96];				//0x0000
				float							mTimeElapsed;				//0x0060
				char							pad_0064[144];				//0x0064
				i32_t							pMissionSuccessComplete;	//0x00F4
				i32_t							pMissionFailOnDeath;		//0x00F8
				i32_t							pMissionFailOnAbort;		//0x00FC
				char							pad_0100[128];				//0x0100
			};	//Size: 0x0180

			class CAppCamera
			{
			public:
				i32_t							pVFTable;					//0x0000
				char							pad_0004[48];				//0x0004
				i32_t							pCamera;					//0x0034 * zdb::CCamera
				char							pad_0038[4];				//0x0038
				i32_t							pAttachedSeal;				//0x003C
				char							pad_0040[16];				//0x0040
				Vec3							origin;						//0x0050
				char							mViewMode;					//0x005C	//	0 = normal view : 2+ = zoom of sorts
			};	//Size: 0x005D

			class zdb_CCamera
			{
			public:
				Matrix4x4						modelView;					//0x0000
				char							pad_0040[168];				//0x0040
				float							N000008F8;					//0x00E8
				float							N000006F3;					//0x00EC
				float							N000008FA;					//0x00F0
				char							pad_00F4[92];				//0x00F4
				SCameraFrustrum					frustrum;					//0x0150
				char							pad_01C0[16];				//0x01C0
				float							N00000709;					//0x01D0
				float							N00000926;					//0x01D4
				char							pad_01D8[184];				//0x01D8
				float							N00000721;					//0x0290
				char							pad_0294[44];				//0x0294
				tag_ZCAM_MTX_SET				m_mtxSet;					//0x02C0
				unsigned int					N000019DF;					//0x03C0
				char							pad_03C4[12];				//0x03C4
				ZCAM_CLIP_DATA					m_ClipSet;					//0x03D0
				float							N00001A08;					//0x0440
				float							N000019E2;					//0x0444
				float							N00001A0A;					//0x0448
				float							N000019E3;					//0x044C
				float							N00001A0C;					//0x0450
				float							N000019E4;					//0x0454
				char							pad_0458[24];				//0x0458
				float							N00001A13;					//0x0470
				float							N000019E9;					//0x0474
				char							pad_0478[40];				//0x0478
				float							N00001A1F;					//0x04A0
				float							N000019EF;					//0x04A4
				char							pad_04A8[756];				//0x04A8
			};    //Size: 0x079C

			class CZSealObject
			{
			private:
				char							pad_0000[48];				//0x0000
				Vec3							m_absPosition;				//0x0030
				char							pad_003C[96];				//0x003C
				float							m_selfVisible;				//0x009C	//	255 = visible : 0 = invisible
			};	//Size: 0x0100

			class CZSealBody
			{
			public:
				i32_t							pVFTable;					//0x0000
				char							pad_0004[16];				//0x0004
				i32_t							pName;						//0x0014
				char							pad_0018[4];				//0x0018
				Vec3							origin;						//0x001C
				i32_t							pSealTM;					//0x0028	* CZSealObject
				char							pad_002C[148];				//0x002C
				i32_t							pSealCTRL;					//0x00C0	* CZSealCTRL
				__int32							TeamID;						//0x00C4
				char							pad_00C8[152];				//0x00C8
				EZoomState						ZoomState;					//0x0160
				char							pad_0161[3];				//0x0161
				float							ZoomLevel;					//0x0164
				char							pad_0168[412];				//0x0168
				ESealStance						Stance;						//0x0304
				char							pad_0305[291];				//0x0305
				Vec3							rotation;					//0x0428
				char							pad_0434[168];				//0x0434
				__int32							TotalShotsFired;			//0x04DC
				char							pad_04E0[48];				//0x04E0
				__int32							PrimaryShotsFired;			//0x0510
				__int32							SecondaryShotsFired;		//0x0514
				char							pad_0518[52];				//0x0518
				float							RecoilPunch;				//0x054C
				char							pad_0550[188];				//0x0550
				__int32							pPrimaryWeapon;				//0x060C	* CZWeapon
				__int32							pSecondaryWeapon;			//0x0610	* CZWeapon
				__int32							pEqSlot1Weapon;				//0x0614	* CZWeapon
				__int32							pEqSlot2Weapon;				//0x0618	* CZWeapon
				__int32							pEqSlot3Weapon;				//0x061C	* CZWeapon
				char							pad_0620[100];				//0x0620
				i32_t							pPrimaryAmmoType;			//0x0684	* CZAmmo
				i32_t							pSecondaryAmmoType;			//0x0688	* CZAmmo
				i32_t							pEqSlot1AmmoType;			//0x068C	* CZAmmo
				i32_t							pEqSlot2AmmoType;			//0x0690	* CZAmmo
				i32_t							pEqSlot3AmmoType;			//0x0694	* CZAmmo
				char							pad_0698[100];				//0x0698
				__int32							PrimaryMags[10];			//0x06FC
				__int32							SecondaryMags[10];			//0x0724
				__int32							EqSlot1Ammo;				//0x074C
				char							pad_0750[36];				//0x0750
				__int32							EqSlot2Ammo;				//0x0774
				char							pad_0778[36];				//0x0778
				__int32							EqSlot3Ammo;				//0x079C
				char							pad_07A0[1156];				//0x07A0
				EWeaponFireMode					PrimaryFireMode;			//0x0C24
				EWeaponFireMode					SecondaryFireMode;			//0x0C28
				char							pad_0C2C[272];				//0x0C2C
				__int32							CurrentShotsFired;			//0x0D3C
				float							FiringCooldown;				//0x0D40
				__int32							CurrentWeaponIndex;			//0x0D44
				char							pad_0D48[4];				//0x0D48
				__int32							MaxWeaponIndex;				//0x0D4C
				char							pad_0D50[308];				//0x0D50
				__int32							WeaponReadyState;			//0x0E84
				char							pad_0E88[64];				//0x0E88
				float							Elevation;					//0x0EC8
				char							pad_0ECC[4];				//0x0ECC
				float							Health;						//0x0ED0
			};	//Size: 0x0ED4

			class CZWeapon
			{
			public:
				char							pad_0000[4];				//0x0000
				i32_t							pName;						//0x0004
				i32_t							pNameDesc;					//0x0008
				char							pad_000C[4];				//0x000C
				i32_t							pIconName;					//0x0010
				char							pad_0014[4];				//0x0014
				i32_t							pNameFull;					//0x0018
				i32_t							pBulletImpactName;			//0x001C
				EWeaponFireMode					mMaxFireMode;				//0x0020
				__int32							szMag;						//0x0024
				__int32							defaultMags;				//0x0028
				char							pad_002C[12];				//0x002C
				float							maxRange;					//0x0038
				float							effectiveRange;				//0x003C
				char							pad_0040[4];				//0x0040
				float							impactRadius;				//0x0044
				float							fireWait;					//0x0048
				char							pad_004C[4];				//0x004C
				bool							bReloadAfterShot;			//0x0050
				char							pad_0051[415];				//0x0051

			};	//Size: 0x0080

			class CZAmmo
			{
			public:
				i32_t							pAmmoName;					//0x0000
				i32_t							pDisplayName;				//0x0004
				float							bulletImpactDmg;			//0x0008
				float							stun;						//0x000C
				float							piercing;					//0x0010
				float							explosionDamage;			//0x0014
				float							explosionRadius;			//0x0018
				char							pad_001C[4];				//0x001C
			};	//Size: 0x0080
		}

		namespace Tools
		{
			/* GET */
			bool GetCameraMatrixSet(Structs::tag_ZCAM_MTX_SET& mtxSet);
			bool GetCameraViewMatrix(Structs::ZViewModel& CameraView); 
			bool GetCameraModelViewMatrix(Matrix16& ModelView);
			bool GetLocalSeal(Classes::CZSealBody& pSeal, i64_t* pAddr);
			bool GetPlayers(std::vector<Classes::CZSealBody>* players);
			bool GetWeapon(const int& weaponIndex, Classes::CZWeapon& weapon, i64_t* pWeaponAddr);
			std::string GetWeaponName(const Enums::EWeapon& weapon);
			std::string GetAmmoName(const Enums::EWeaponAmmo& ammo);

			/* RENDER */
			bool ProjectWorldToScreen(Vec3 WorldLocation, Structs::ZViewModel CameraView, float fov, Vec2 szScreen, Vec2* screen2D);	
		}

		namespace Patches
		{
			/* WEAPON */
			void RefillAllAmmo();		
			void RefillAmmo(const int& weaponIndex);
			void SetWeapon(const int& weaponIndex, const Enums::EWeapon& newWeapon);
			void SetWeaponFireMode(const int& weaponIndex, const Enums::EWeaponFireMode& fireMode);
			void SetWeaponAmmoType(const int& weaponIndex, const Enums::EWeaponAmmo& ammoType);
			void SetWeaponMagCount(const int& weaponIndex, const int& newMagCount);

			/* MISSION */
			void ForceCompleteMission();

			/* CUSTOM */
			void SetAmmoProperties(const int& weaponIndex, Classes::CZAmmo& newAmmoType);
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
		__int8 stance{ 0 };	//	0 = standing, 1 = crouching, 2 = prone
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
		Engine::zdb::Classes::CZSealBody seal;
	};

	struct SGlobals
	{
		bool bValid{ false };
		SGame game;
		SLocalPlayer localPlayer;
		std::vector<SImGuiPlayer> render;
		Engine::Matrix16 mvmatrix;
		Engine::zdb::Structs::ZViewModel cameraView;
		Engine::zdb::Structs::tag_ZCAM_MTX_SET mtxSet;
	};

public:
	SGlobals imCache;

public:
	void Update();
	void ShutDown();
}; 

inline pcsx2Memory g_PSXMemory = pcsx2Memory("pcsx2-qt.exe");
inline std::unique_ptr<SOCOM> g_SOCOM;