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

			/* */
			bool Camera::GetCamera(Classes::zdb_CCamera& camera)
			{
				__int64 eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return false;

				auto pCamera = g_PSXMemory.Read<__int32>(eemem + Offsets::gCamera);
				if (!pCamera || pCamera == Offsets::gCamera)
					return false;

				camera = g_PSXMemory.Read<Classes::zdb_CCamera>(eemem + pCamera);

				return true;
			}

			/* */
			bool Camera::GetModelMtx(Matrix4x4& ModelView)
			{
				Classes::zdb_CCamera camera{};
				if (!GetCamera(camera))
					return false;

				ModelView = camera.modelView;

				return true;
			}

			/* */
			bool Camera::GetViewMtx(Structs::ZViewModel& CameraView)
			{
				__int64 eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return false;

				auto pCamera = g_PSXMemory.Read<__int32>(eemem + Offsets::gCamera);
				if (!pCamera || pCamera == Offsets::gCamera)
					return false;

				CameraView = g_PSXMemory.Read<Classes::ZViewModel>(eemem + pCamera);

				return true;
			}

			/* */
			bool Camera::GetMtxSet(Structs::tag_ZCAM_MTX_SET& mtxSet)
			{
				Classes::zdb_CCamera camera{};
				if (!GetCamera(camera))
					return false;

				mtxSet = camera.m_mtxSet;

				return true;
			}

			/* */
			bool Camera::GetViewport(Structs::tag_RECT& viewport)
			{

				Classes::zdb_CCamera camera{};
				if (!GetCamera(camera))
					return false;

				viewport = camera.m_screen;

				return true;
			}

			/* */
			bool Entity::GetLocalSeal(Classes::CZSealBody& seal, i64_t* pSealAddr)
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

			/* */
			bool Entity::GetPlayers(std::vector<Classes::CZSealBody>*players)
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

			/* */
			bool Entity::GetPickups(std::vector<Classes::CPickup>& pickups)
			{
				std::vector<Classes::CPickup> container;

				__int64 eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return false;

				auto pickupArray = g_PSXMemory.Read<Structs::ZArray>(eemem + Offsets::gPickups);
				if (pickupArray.count <= 0 || pickupArray.begin <= 0 || pickupArray.end <= 0)
					return false;

				auto it = g_PSXMemory.Read<Structs::ZIterator>(eemem + pickupArray.begin);
				auto end = g_PSXMemory.Read<Structs::ZIterator>(it.prev);
				do
				{
					auto data = it.data;
					if (data > 0)
					{
						auto pickup = g_PSXMemory.Read<Classes::CPickup>(eemem + data);
						if (pickup.pNode && pickup.pData)
							container.push_back(pickup);
					}

					it = g_PSXMemory.Read<Structs::ZIterator>(eemem + it.next);

				} while (it.data != end.data);

				if (container.empty())
					return false;

				pickups = std::move(container);

				return true;
			}

			/* */
			bool Weapon::GetWeapon(const int& weaponIndex, Classes::CZWeapon& weapon, i64_t* pWeaponAddr)
			{
				__int64 eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return false;

				i64_t sealAddr = 0;
				Classes::CZSealBody czSeal;
				if (!Tools::Entity::GetLocalSeal(czSeal, &sealAddr) || !sealAddr)
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

			/* */
			std::string Weapon::GetWeaponName(const Enums::EWeapon& weapon)
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

			/* */
			std::string Weapon::GetAmmoName(const Enums::EWeaponAmmo& ammo)
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

			/* */
			int Game::GetFramerate()
			{
				const int default_frame = 30;

				__int64 eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return default_frame;

				const i64_t& pFPS = eemem + Offsets::gFPS;
				if (!pFPS || pFPS == Offsets::gFPS)
					return default_frame;

				return g_PSXMemory.Read<int>(pFPS);
			}

			Matrix4x4 Transform::BuildViewToClip(const zdb::Classes::zdb_CCamera& camera)
			{
				/*
				
				* ViewToClip
					1.428			0				0				0
					0				1.020			0				0
					0				0				1				1
					0				0				-8.002			0

				crucial components
				key
				 - [0][0] = 457.007 / 320 = 1.428146875
				 - [1][1] = 457.007 / 448 = 1.020105
				*/

				const float viewportWidth = camera.m_screen.right - camera.m_screen.left;
				const float viewportHeight = camera.m_screen.bottom - camera.m_screen.top;

				const float viewToClip_00 = camera.m_scrZ / (viewportWidth / 2) * camera.m_screenAspect.x;
				const float viewToClip_11 = camera.m_scrZ / viewportHeight * camera.m_screenAspect.y;

				return {
					viewToClip_00, 0.0f, 0.0f, 0.0f,
					0.0f, viewToClip_11, 1.0f, 1.0f,
					0.0f, 0.0f, 0.0f, 0.0f
				};
			}

			Matrix4x4 Transform::BuildViewToScreen(const zdb::Classes::zdb_CCamera& camera)
			{
				/*
				* ViewToScreen
					457.007			0				0				0
					0				457.007			0				0
					2048.000		2048.000		-3.722			1
					0				0				262154.900		0

				crucial components
				 - scaleX	= 457.007	[0][0]
				 - scaleY	= 457.007	[1][1]
				 - centerX	=	2048	[2][0]
				 - centerY	=	2048	[2][1]
				 - depth	= -3.722
				 - ZMapping = 262154.900

				key
				 - scale = m_scrZ * m_screenAspect
				 - center = m_screenCenter + m_screenOffset
				*/

				const float scaleX = camera.m_scrZ * camera.m_screenAspect.x;
				const float scaleY = camera.m_scrZ * camera.m_screenAspect.y;

				const float centerX = camera.m_screenCenter.x + camera.m_screenOffset.x;
				const float centerY = camera.m_screenCenter.y + camera.m_screenOffset.y;

				/*
					Depth mapping still unknown.

					These values do not affect WorldToScreen because
					ScreenSpaceToNormalized only consumes x, y, and w.
				*/
				const float depthScale = 1.f; // unknown how to obtain , seems to come from viewtoclip
				const float depthBias = camera.m_Zmax * (camera.m_camera_params.m_near_plane/* + depthScale*/); // 65535.0000 * (4.00000000) = 262140.000

				return {
					scaleX, 0.0f, 0.0f, 0.0f,
					0.0f, scaleY, 0.0f, 0.0f,
					centerX, centerY, depthScale, 1.0f,
					0.0f, 0.0f, depthBias, 0.0f
				};
			}

			/* */
			Vec4 Transform::WorldToView(const Vec3& worldPosition, const Matrix4x4& worldToView)
			{
				return worldToView.TransformPoint(worldPosition);
			}

			/* */
			Vec3 Transform::WorldToViewFromModel(const Vec3& worldPosition, const Matrix4x4& modelMatrix)
			{
				const Vec3 cam_right{
					modelMatrix.m[0][0],
					modelMatrix.m[0][1],
					modelMatrix.m[0][2]
				};

				const Vec3 cam_up{
					modelMatrix.m[1][0],
					modelMatrix.m[1][1],
					modelMatrix.m[1][2]
				};

				const Vec3 cam_forward{
					modelMatrix.m[2][0],
					modelMatrix.m[2][1],
					modelMatrix.m[2][2]
				};

				const Vec3 cam_pos{
					modelMatrix.m[3][0],
					modelMatrix.m[3][1],
					modelMatrix.m[3][2]
				};

				Vec3 heading = worldPosition - cam_pos;

				return {
					heading.dot(cam_right),
					-heading.dot(cam_up),
					-heading.dot(cam_forward)
				};
			}

			/* */
			Vec4 Transform::ViewToScreenSpace(const Vec4& view, const Matrix4x4& viewToScreen)
			{
				return viewToScreen.TransformPoint(view);
			}

			/* */
			bool Transform::ScreenSpaceToNormalized(const Vec4& screenSpace, const Structs::tag_RECT& viewport, Vec2* out)
			{
				/* check if behind camera */
				if (screenSpace.w <= 0.001f)
					return false;

				const float x = screenSpace.x / screenSpace.w;
				const float y = screenSpace.y / screenSpace.w;

				const float width = viewport.right - viewport.left;
				const float height = viewport.bottom - viewport.top;

				if (width <= 0.0f || height <= 0.0f)
					return false;

				const Vec2 result
				{
					(x - viewport.left) / width,
					(y - viewport.top) / height
				};

				if (result.x < 0.0f || result.x > 1.0f ||
					result.y < 0.0f || result.y > 1.0f)
				{
					return false;
				}

				*out = result;
				return true;
			}

			/* */
			bool Transform::NormalizedToScreen(const Vec2& normalized, const Vec2& screenSize, Vec2* out)
			{
				Vec2 result{ normalized };
				if (result.x < 0.0f || result.x > 1.0f ||
					result.y < 0.0f || result.y > 1.0f)
				{
					return false;
				}

				*out = (result *= screenSize);

				return true;
			}

			/* */
			bool Transform::WorldToScreen(const Vec3& worldLocation, Vec2* out)
			{
				Classes::zdb_CCamera camera;
				if (!Tools::Camera::GetCamera(camera))
					return false;

				return WorldToScreen(worldLocation, camera, out);
			}

			/* */
			bool Transform::WorldToScreen(const Vec3& worldLocation, const Vec2& szScreen, Vec2* out)
			{
				Classes::zdb_CCamera camera;
				if (!Tools::Camera::GetCamera(camera))
					return false;

				return WorldToScreen(worldLocation, camera, szScreen, out);
			}

			/* */
			bool Transform::WorldToScreen(const Vec3& worldLocation, zdb::Classes::zdb_CCamera& camera, Vec2* out)
			{
				/* transform world point to view space */
				Vec4 view = WorldToView(worldLocation, camera.m_mtxSet.mtxWorldToView);

				/* get native screen space (GS) */
				Vec4 gs = ViewToScreenSpace(view, camera.m_mtxSet.mtxViewToScreen);

				/* screen space to normalized */
				Vec2 result;
				if (ScreenSpaceToNormalized(gs, camera.m_screen, &result) == false)
					return false;

				*out = result;

				return true;
			}

			/* */
			bool Transform::WorldToScreen(const Vec3& worldLocation, zdb::Classes::zdb_CCamera& camera, const Vec2& szScreen, Vec2* out)
			{
				Vec2 normalized{};
				if (WorldToScreen(worldLocation, camera, &normalized) == false)
					return false;

				return NormalizedToScreen(normalized, szScreen, out);
			}

			/* */
			bool Transform::Debug::ProjectWorldToScreenFromModelMtx(const Vec3& worldLocation, zdb::Classes::zdb_CCamera& camera, const Engine::Matrix4x4& modelMatrix, const Vec2& szScreen, Vec2* out)
			{
				/* transform world point to view space */
				auto view = WorldToView(worldLocation, modelMatrix);

				/* get native screen space (GS) */
				auto gs = ViewToScreenSpace(view, camera.m_mtxSet.mtxViewToScreen);

				Vec2 normalized{};
				if (ScreenSpaceToNormalized(gs, camera.m_screen, &normalized) == false)
					return false;

				return NormalizedToScreen(normalized, szScreen, out);
			}

			/* */
			bool Transform::Debug::ProjectWorldToScreenFromCameraView(Vec3 WorldLocation, Structs::ZViewModel CameraView, float fov, Vec2 szScreen, Vec2* screen2D)
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

				///  apply perspective projection [this was a custom method based on static input fov]
				float aspect = szScreen.x / szScreen.y;
				float fov_radians = tan(fov * 0.5f * (M_PI / 180.f)); // Convert fov to radians and compute scaling factor
				float pX = camX / (camZ * fov_radians * aspect);
				float pY = camY / (camZ * fov_radians);

				/// apply perspective projection using camera values in class
				//	float tanHalfHFov = 0.611;
				//	float tanHalfVFov = 0.428;    
				//	float pX = camX / (camZ * tanHalfHFov);
				//	float pY = camY / (camZ * tanHalfVFov);

				Vec2 res =
				{
					(pX + 1.0f) * 0.5f * szScreen.x,
					(1.0f - pY) * 0.5f * szScreen.y // Invert Y because screen coordinates are top-down
				};

				if (res.x <= 0.f ||
					res.y <= 0.0f ||
					res.x > szScreen.x ||
					res.y > szScreen.y)
					return false;

				*screen2D = res;

				return true;
			}

			/* */
			bool Transform::Debug::ProjectWorldToScreen_TEST(const Vec3& world, const Matrix4x4 worldToView, const Matrix4x4 viewToScreen, const Vec2& screenSize, Vec2* out)
			{
				//
				// World -> View
				//
				Vec4 WorldLocation =
				{
					world.x,
					world.y,
					world.z,
					1.0f
				};
				Vec4 view = worldToView.TransformPoint(WorldLocation); // TransformPoint(WorldLocation, worldToView);

				//
				// View -> native PS2 screen space
				//
				Vec4 gs = viewToScreen.TransformPoint(view); // TransformPoint(view, viewToScreen);

				if (gs.w <= 0.001f)
					return false;

				float gsX = gs.x / gs.w;
				float gsY = gs.y / gs.w;

				//
				// Camera's hardcoded native viewport:
				//
				constexpr float left = 1728.0f;
				constexpr float top = 1824.0f;
				constexpr float right = 2368.0f;
				constexpr float bottom = 2272.0f;

				constexpr float nativeWidth = right - left;    // 640
				constexpr float nativeHeight = bottom - top;    // 448

				//
				// Convert native PS2 viewport -> normalized [0, 1]
				//
				float u = (gsX - left) / nativeWidth;
				float v = (gsY - top) / nativeHeight;

				//
				// PCSX2 is currently set to Stretch, so the complete native
				// image is stretched over the entire ImGui/output area.
				//
				Vec2 result =
				{
					u * screenSize.x,
					v * screenSize.y
				};

				if (result.x < 0.0f ||
					result.y < 0.0f ||
					result.x > screenSize.x ||
					result.y > screenSize.y)
				{
					return false;
				}

				*out = result;
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
				if (!Tools::Entity::GetLocalSeal(czSeal, &sealAddr) || !sealAddr)
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
				if (!Tools::Entity::GetLocalSeal(czSeal, &sealAddr) || !sealAddr)
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
				if (!Tools::Entity::GetLocalSeal(czSeal, &sealAddr) || !sealAddr)
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
				if (!Tools::Entity::GetLocalSeal(czSeal, &sealAddr) || !sealAddr)
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
				if (!Tools::Entity::GetLocalSeal(czSeal, &sealAddr) || !sealAddr)
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

			void SetFramerate(int frames)
			{
				struct iVec2 {
					i32_t m[2];
				};
				
				const i64_t& eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return;

				const i64_t& pFPS = eemem + Offsets::gFPS;
				if (!pFPS || pFPS == Offsets::gFPS)
					return;

				i32_t fps = frames;
				if (fps <= 0)
					fps = 30;

				g_PSXMemory.Write<iVec2>(pFPS, { fps, fps });
			}

			/* CUSTOM */
			void SetAmmoProperties(const int& weaponIndex, Classes::CZAmmo& newAmmoType)
			{
				__int64 eemem = g_PSXMemory.GetEEMemory();
				if (!eemem)
					return;

				i64_t sealAddr = 0;
				Classes::CZSealBody czSeal;
				if (!Tools::Entity::GetLocalSeal(czSeal, &sealAddr) || !sealAddr)
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

	static auto reset = [this](const char* reason) 
	{ 
		{
			std::lock_guard<std::mutex> lock(this->m_cacheMutex);
			this->m_cache = SGlobalSnapshot();
			this->m_tick = 0;
		} // free lock
		printf("[!] SOCOM::Update - reset `%s`\n", reason);
	};

	SGlobalSnapshot globals;
	auto& game = globals.m_ctx;
	auto& player = globals.m_localPlayer;

	globals.m_EE = g_PSXMemory.GetEEMemory();
	if (!globals.m_EE)
		return reset("failed to obtain eemem");

	//	GET LOCAL PLAYER
	auto pLocalPlayer = g_PSXMemory.Read<__int32>(globals.m_EE + Offsets::gLocalSeal);;
	if (!pLocalPlayer)
		return reset("failed to obtain local player");

	auto localSeal = g_PSXMemory.Read<Classes::CZSealBody>(globals.m_EE + pLocalPlayer);
	
	/* get the tick rate */
	{
		auto tick = g_PSXMemory.Read<__int32>(globals.m_EE + (localSeal.pSealTM + 0xA4));
		if (this->m_tick == 0) // tick not yet set
			this->m_tick = tick; // set the tick rate
	}

	player.m_RVA = pLocalPlayer;
	player.m_pos = localSeal.origin;
	player.m_seal = localSeal;
	if (!g_PSXMemory.ReadString(globals.m_EE + localSeal.pName, player.m_name, 32))
		return reset("failed to read local player name");

	//	GET PLAYERS
	std::vector<SImGuiPlayer> imPlayers;
	std::vector<Classes::CZSealBody> seals;
	if (Tools::Entity::GetPlayers(&seals))
	{
		imPlayers.reserve(seals.size());
		for (auto& ent : seals)
		{
			SImGuiPlayer imPlayer;

			if (ent.pName == localSeal.pName)
				continue;	//	skip local player

			if (!g_PSXMemory.ReadString(globals.m_EE + ent.pName, imPlayer.m_name, 32))
				continue;

			imPlayer.m_pos = ent.origin;
			imPlayer.m_health = ent.Health * 100.f;
			imPlayer.m_bAlive = (imPlayer.m_health > 0.f);
			imPlayer.m_stance = ent.Stance;
			imPlayer.m_class = ent;

			imPlayers.push_back(imPlayer);
		}
	}

	// GET PICKUPS
	std::vector<SImGuiPickup> imPickups;
	std::vector<Classes::CPickup> pickups;
	if (pickups.empty() && Tools::Entity::GetPickups(pickups))
	{
		imPickups.reserve(pickups.size());
		for (auto& pickup : pickups)
		{
			SImGuiPickup imPickup;

			i32_t pName{ 0 };
			if (pickup.mType == Engine::zdb::Structs::EPickupType::PICKUP_TYPE_WEAPON)
				pName = g_PSXMemory.Read<__int32>(globals.m_EE + (pickup.pData + 0x4)); // CZWeapon::pDisplayName
			else
			{
				pName = g_PSXMemory.Read<__int32>(globals.m_EE + (pickup.pData + 0x14)); //
				if (pName)
					pName = g_PSXMemory.Read<__int32>(globals.m_EE + pName); // 
					if (pName)
						pName = g_PSXMemory.Read<__int32>(globals.m_EE + (pName + 0x4)); // CZAmmo::pDisplayName
			}
			if (!pName)
				continue;

			if (!g_PSXMemory.ReadString(globals.m_EE + pName, imPickup.m_name))
				continue;

			auto model_matrix = g_PSXMemory.Read<Engine::Matrix4x4>(globals.m_EE + pickup.pNode);

			imPickup.m_pos = model_matrix.Translate();
			imPickup.m_class = pickup;

			imPickups.push_back(imPickup);
		}
	}

	game.m_bInGame = seals.size() > 1;
	game.m_playerCount = imPlayers.size();
	game.m_pickupCount = imPickups.size();
	globals.m_players = std::move(imPlayers);
	globals.m_pickups = std::move(imPickups);

	// GET CAMERA
	if (!Tools::Camera::GetCamera(globals.m_camera))
		return reset("failed to obtain camera");

	/* CAMERA REFRESH TIMING */
	{
		static Engine::Matrix4x4 world_lastMatrix{};
		static auto world_lastChange = std::chrono::steady_clock::now();
		static bool world_initialized = false;
		
		const auto& mtxWorldToView = globals.m_camera.m_mtxSet.mtxWorldToView;
		if (!world_initialized)
		{
			world_lastMatrix = mtxWorldToView;
			world_lastChange = std::chrono::steady_clock::now();
			world_initialized = true;
		}
		else if (memcmp(&mtxWorldToView, &world_lastMatrix, sizeof(mtxWorldToView)) != 0)
		{
			const auto now = std::chrono::steady_clock::now();
		
			this->m_cameraRefreshTime[0] = std::chrono::duration<float, std::milli>(now - world_lastChange).count();
		
			world_lastChange = now;
			world_lastMatrix = mtxWorldToView;
		}
	}

	globals.m_bValid = true;
	{
		std::lock_guard<std::mutex> lock(this->m_cacheMutex);
		this->m_cache = std::move(globals);
	} // free lock
}

void SOCOM::ShutDown()
{
	// disable any enabled patches

	g_PSXMemory.Detach();
}
