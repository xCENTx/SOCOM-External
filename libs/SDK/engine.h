
#include <cmath>
#include <vector>

namespace Engine
{
	struct Vec2
	{
		float x, y;
		Vec2() : x(0), y(0) {}
		Vec2(float x, float y) : x(x), y(y) {}

		Vec2 operator+(const Vec2& other) const;
		Vec2 operator-(const Vec2& other) const;
		Vec2 operator*(float scalar) const;
		Vec2 operator/(float scalar) const;
		Vec2 operator=(float* other) const;
		float operator*(const Vec2& other) const;
		Vec2& operator*=(const Vec2& other);
		Vec2 operator*=(float scalar);

		float length() const;
		void normalize();
	};

	struct Vec3
	{
		float x, y, z;
		Vec3() : x(0), y(0), z(0) {}
		Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

		Vec3 operator+(const Vec3& other) const;
		Vec3 operator-(const Vec3& other) const;
		Vec3 operator*(float scalar) const;
		Vec3 operator/(float scalar) const;
		float operator*(const Vec3& other) const;
		Vec3 operator^(const Vec3& other) const;
		Vec3 operator=(float* other) const;
		Vec3& operator+=(const float* other);
		Vec3& operator-=(const float* other);
		Vec3& operator+=(const Vec3& other);
		Vec3& operator-=(const Vec3& other);
		Vec3& operator*=(const Vec3& other);
		Vec3 operator*=(const float scalar);
		bool operator==(Vec3 other) const;

		bool isValid();
		float dot(const Vec3& other);
		float length() const;
		void normalize();
		float Distance(Vec3& other);
	};

	struct Vec4
	{
		float x, y, z, w;
		Vec4() : x(0), y(0), z(0), w(0) {}
		Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
		Vec4 operator+(const Vec4& other) const;
		Vec4 operator-(const Vec4& other) const;
		Vec4 operator*(float scalar) const;
		Vec4 operator/(float scalar) const;
		float operator*(const Vec4& other) const;
		Vec4 operator=(float* other) const;
		Vec4& operator*=(const Vec4& other);
		float length() const;
		void normalize();
	};

	struct Matrix16
	{
		float m[16];

		Vec4 MatrixMultiply(const Vec3& v) const;
		Vec4 MatrixMultiply(const Vec4& v) const;
	};

	struct Matrix4x4
	{
		float m[4][4];

		Vec4 operator*(const Vec4& v) const;

		Vec4 MatrixMultiply(const Vec3& v) const;
		Vec4 MatrixMultiply(const Vec4& v) const;
	};
}