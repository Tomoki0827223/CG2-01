#pragma once
struct Vector3 final {
	float x;
	float y;
	float z;

	// 加算演算子のオーバーロード
	Vector3 operator+(const Vector3& other) const {
		return { x + other.x, y + other.y, z + other.z };
	}

	// 減算演算子のオーバーロード
	Vector3 operator-(const Vector3& other) const {
		return { x - other.x, y - other.y, z - other.z };
	}

	// 乗算演算子のオーバーロード（スカラー乗算）
	Vector3 operator*(float scalar) const {
		return { x * scalar, y * scalar, z * scalar };
	}

	// 除算演算子のオーバーロード（スカラー除算）
	Vector3 operator/(float scalar) const {
		return { x / scalar, y / scalar, z / scalar };
	}

	// 加算代入演算子のオーバーロード
	Vector3& operator+=(const Vector3& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	// 減算代入演算子のオーバーロード
	Vector3& operator-=(const Vector3& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	// 乗算代入演算子のオーバーロード（スカラー乗算）
	Vector3& operator*=(float scalar) {
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}

	// 除算代入演算子のオーバーロード（スカラー除算）
	Vector3& operator/=(float scalar) {
		x /= scalar;
		y /= scalar;
		z /= scalar;
		return *this;
	}
};
