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

	Vec4 Matrix16::MatrixMultiply(const Vec3& v) const {
		
		/// DX
		return Vec4
		(
			m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3],
			m[4] * v.x + m[5] * v.y + m[6] * v.z + m[7],
			m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11],
			m[12] * v.x + m[13] * v.y + m[14] * v.z + m[15]
		);


		/// OpenGL
		//	return Vec4
		//	(
		//		m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12],
		//		m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13],
		//		m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14],
		//		m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15]
		//	);
	}

	Vec4 Matrix16::MatrixMultiply(const Vec4& v) const {

		/// DX
		return Vec4
		(
			m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3] * v.w,
			m[4] * v.x + m[5] * v.y + m[6] * v.z + m[7] * v.w,
			m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11] * v.w,
			m[12] * v.x + m[13] * v.y + m[14] * v.z + m[15] * v.w
		);


		/// OpenGL
		//	return Vec4
		//	(
		//		m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12] * v.w,
		//		m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13] * v.w,
		//		m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w,
		//		m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w
		//	);
	}

	Vec4 Matrix4x4::operator*(const Vec4& v) const {
		return Vec4(
			m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w,
			m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w,
			m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w,
			m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w
		);
	}

	Vec4 Matrix4x4::MatrixMultiply(const Vec3& v) const {
		return Vec4
		(
			m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * 1.0f,
			m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * 1.0f,
			m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * 1.0f,
			m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * 1.0f
		);
	}

	Vec4 Matrix4x4::MatrixMultiply(const Vec4& v) const {
		return Vec4(
			m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w,
			m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w,
			m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w,
			m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w
		);
	}
}