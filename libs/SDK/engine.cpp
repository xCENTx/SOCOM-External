#include "engine.h"

namespace Engine
{
	Vec2 Vec2::operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }

	Vec2 Vec2::operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }

	Vec2 Vec2::operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }

	Vec2 Vec2::operator/(float scalar) const { return Vec2{ x / scalar, y / scalar }; }

	Vec2 Vec2::operator=(float* other) const { return Vec2{ other[0], other[1] }; }

	float Vec2::operator*(const Vec2& other) const { return x * other.x + y * other.y; }

	Vec2& Vec2::operator*=(const Vec2& other)
	{
		x += other.x;
		y += other.y;
		return *this;
	}

	Vec2 Vec2::operator*=(float scalar) {
		x *= scalar;
		y *= scalar;
		return *this;
	}

	float Vec2::length() const { return std::sqrt(x * x + y * y); }

	void Vec2::normalize()
	{
		float len = length();
		if (len > 0) {
			x /= len;
			y /= len;
		}
	}

	Vec3 Vec3::operator+(const Vec3& other) const { return Vec3{ x + other.x, y + other.y, z + other.z }; }

	Vec3 Vec3::operator-(const Vec3& other) const { return Vec3{ x - other.x, y - other.y, z - other.z }; }

	Vec3 Vec3::operator*(float scalar) const { return Vec3{ x * scalar, y * scalar, z * scalar }; }

	Vec3 Vec3::operator/(float scalar) const { return Vec3{ x / scalar, y / scalar, z / scalar }; }

	float Vec3::operator*(const Vec3& other) const { return x * other.x + y * other.y + z * other.z; }

	Vec3 Vec3::operator^(const Vec3& other) const { return Vec3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x); }

	Vec3 Vec3::operator=(float* other) const { return Vec3{ other[0], other[1], other[2] }; }

	Vec3& Vec3::operator+=(const float* other)
	{
		x += other[0];
		y += other[1];
		z += other[2];
		return *this;
	}

	Vec3& Vec3::operator-=(const float* other)
	{
		x -= other[0];
		y -= other[1];
		z -= other[2];
		return *this;
	}

	Vec3& Vec3::operator+=(const Vec3& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	Vec3& Vec3::operator-=(const Vec3& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	Vec3& Vec3::operator*=(const Vec3& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	Vec3 Vec3::operator*=(const float scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}

	bool Vec3::operator==(Vec3 other) const { return other.x == x && other.y == y && other.z == z; }

	bool Vec3::isValid(){ return x == 0.0f && y == 0.0f; }

	float Vec3::dot(const Vec3& other) { return x * other.x + y * other.y + z * other.z; }

	float Vec3::length() const { return std::sqrt(x * x + y * y + z * z); }

	void Vec3::normalize()
	{
		float len = length();
		if (len > 0) {
			x /= len;
			y /= len;
			z /= len;
		}
	}

	float Vec3::Distance(Vec3& other)
	{
		auto dist = *this - other;
		return sqrt(dist.x * dist.x + dist.y * dist.y + dist.z * dist.z) * 0.1f;
	}

	Vec4 Vec4::operator+(const Vec4& other) const { return Vec4(x + other.x, y + other.y, z + other.z, w + other.w); }

	Vec4 Vec4::operator-(const Vec4& other) const { return Vec4(x - other.x, y - other.y, z - other.z, w - other.w); }

	Vec4 Vec4::operator*(float scalar) const { return Vec4(x * scalar, y * scalar, z * scalar, w * scalar); }

	Vec4 Vec4::operator/(float scalar) const { return Vec4{ x / scalar, y / scalar, z / scalar , w / scalar }; }

	float Vec4::operator*(const Vec4& other) const { return x * other.x + y * other.y + z * other.z + w * other.w; }

	Vec4 Vec4::operator=(float* other) const { return Vec4{ other[0], other[1], other[2] , other[3] }; }

	Vec4& Vec4::operator*=(const Vec4& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
		return *this;
	}

	float Vec4::length() const { return std::sqrt(x * x + y * y + z * z + w * w); }

	void Vec4::normalize()
	{
		float len = length();
		if (len > 0) {
			x /= len;
			y /= len;
			z /= len;
			w /= len;
		}
	}

	namespace Tools
	{
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

	Vec3 ZViewModel::origin() const { return Vec3(pos.x, pos.y, pos.z); }
}