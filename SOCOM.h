#pragma once
#include <Memory/Memory.h>
#include <cmath>
#include <vector>

namespace Engine
{
	struct Vec2
	{
		float x, y;
		Vec2() : x(0), y(0) {}
		Vec2(float x, float y) : x(x), y(y) {}


		// Addition operator
		Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }

		// Subtraction operator
		Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }

		// Scalar multiplication operator
		Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }

		//	Scalar division operator
		Vec2 operator/(float scalar) const { return Vec2{ x / scalar, y / scalar }; }

		//	float* return operator
		Vec2 operator=(float* other) const { return Vec2{ other[0], other[1] }; }

		// Dot product operator
		float operator*(const Vec2& other) const { return x * other.x + y * other.y; }

		//	
		Vec2& operator*=(const Vec2& other)
		{
			x += other.x;
			y += other.y;
			return *this;
		}

		// Scalar multiplication and assignment operator
		Vec2 operator*=(float scalar) {
			x *= scalar;
			y *= scalar;
			return *this;
		}

		// Length (magnitude) of the vector
		float length() const { return std::sqrt(x * x + y * y); }

		// Normalize the vector
		void normalize()
		{
			float len = length();
			if (len > 0) {
				x /= len;
				y /= len;
			}
		}
	};

	struct Vec3
	{
		float x, y, z;
		Vec3() : x(0), y(0), z(0) {}
		Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

		// Addition operator
		Vec3 operator+(const Vec3& other) const { return Vec3{ x + other.x, y + other.y, z + other.z }; }

		// Subtraction operator
		Vec3 operator-(const Vec3& other) const { return Vec3{ x - other.x, y - other.y, z - other.z }; }

		// Scalar multiplication operator
		Vec3 operator*(float scalar) const { return Vec3{ x * scalar, y * scalar, z * scalar }; }

		//	Scalar division operator
		Vec3 operator/(float scalar) const { return Vec3{ x / scalar, y / scalar, z / scalar }; }

		// Dot product operator
		float operator*(const Vec3& other) const { return x * other.x + y * other.y + z * other.z; }

		// Cross product operator
		Vec3 operator^(const Vec3& other) const { return Vec3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x); }

		//	
		Vec3 operator=(float* other) const { return Vec3{ other[0], other[1], other[2] }; }

		Vec3& operator+=(const float* other)
		{
			x += other[0];
			y += other[1];
			z += other[2];
			return *this;
		}

		Vec3& operator-=(const float* other)
		{
			x -= other[0];
			y -= other[1];
			z -= other[2];
			return *this;
		}

		Vec3& operator+=(const Vec3& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

		Vec3& operator-=(const Vec3& other)
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		}

		//	
		Vec3& operator*=(const Vec3& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

		Vec3 operator*=(const float scalar)
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
			return *this;
		}

		bool operator==(Vec3 other) const { return other.x == x && other.y == y && other.z == z; }

		bool isValid()
		{
			return x == 0.0f && y == 0.0f;
		}

		//	get the dot product 
		float dot(const Vec3& other) { return x * other.x + y * other.y + z * other.z; }

		// Length (magnitude) of the vector
		float length() const { return std::sqrt(x * x + y * y + z * z); }

		// Normalize the vector
		void normalize()
		{
			float len = length();
			if (len > 0) {
				x /= len;
				y /= len;
				z /= len;
			}
		}

		float Distance(Vec3& other)
		{
			auto dist = *this - other;
			return sqrt(dist.x * dist.x + dist.y * dist.y + dist.z * dist.z) * 0.1f;
		}
	};

	struct Vec4
	{
		float x, y, z, w;
		Vec4() : x(0), y(0), z(0), w(w) {}
		Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

		// Addition operator
		Vec4 operator+(const Vec4& other) const { return Vec4(x + other.x, y + other.y, z + other.z, w + other.w); }

		// Subtraction operator
		Vec4 operator-(const Vec4& other) const { return Vec4(x - other.x, y - other.y, z - other.z, w - other.w); }

		// Scalar multiplication operator
		Vec4 operator*(float scalar) const { return Vec4(x * scalar, y * scalar, z * scalar, w * scalar); }

		// Scalar division operator
		Vec4 operator/(float scalar) const { return Vec4{ x / scalar, y / scalar, z / scalar , w / scalar }; }

		// Dot product operator
		float operator*(const Vec4& other) const { return x * other.x + y * other.y + z * other.z + w * other.w; }

		//	
		Vec4 operator=(float* other) const { return Vec4{ other[0], other[1], other[2] , other[3] }; }

		//	
		Vec4& operator*=(const Vec4& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
			w += other.w;
			return *this;
		}

		// Length (magnitude) of the vector
		float length() const { return std::sqrt(x * x + y * y + z * z + w * w); }

		// Normalize the vector
		void normalize()
		{
			float len = length();
			if (len > 0) {
				x /= len;
				y /= len;
				z /= len;
				w /= len;
			}
		}
	};

	struct ZViewModel
	{
		Vec4 right;		//	Vector3 : right vector
		Vec4 up;		//	Vector3 : up vector
		Vec4 fwd;		//	Vector3 : look vector ( - is forward )
		Vec4 pos;		//	Vector3 : position
	};

	struct ZIterator
	{
		__int32 next;	//	pointer to next position in array
		__int32 prev;	//	pointer to last position in array
		__int32 data;	//	pointer to object
	};

	struct ZArray
	{
		__int32 count{ 0 };	//	objects in array
		__int32 begin{ 0 };	//	pointer to first object in the array
		__int32 end{ 0 };	//	pointer to last object in the array
	};

	namespace Classes
	{
		class CZCamera
		{
		public:
			char							pad_0000[52];			//0x0000
			int								pCameraMatrix;			//0x0034
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
		constexpr auto o_LocalCamera{ 0x48D488 };
		constexpr auto o_LocalSeal{ 0x48D548 };
		constexpr auto o_SealArray{ 0x4D46A0 };
	}

	namespace Tools
	{
		bool GetCameraViewMatrix(ZViewModel& CameraView);
		bool GetPlayers(std::vector<Classes::CZSeal>* players);
		bool ProjectWorldToScreen(Vec3 WorldLocation, ZViewModel CameraView, float fov, Vec2 szScreen, Vec2* screen2D);
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