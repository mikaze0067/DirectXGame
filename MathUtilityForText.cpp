#include "MathUtilityForText.h"
#include <numbers>


// 代入演算子オーバーロード
// Vector3の足算
Vector3& operator+=(Vector3& lhv, const Vector3& rhv) {
	lhv.x += rhv.x;
	lhv.y += rhv.y;
	lhv.z += rhv.z;
	return lhv;
}

Vector3& operator-=(Vector3& lhv, const Vector3& rhv) {
	lhv.x -= rhv.x;
	lhv.y -= rhv.y;
	lhv.z -= rhv.z;
	return lhv;
}


// 代入演算子オーバーロード
// Vector3の掛け算
Vector3& operator*=(Vector3& v, float s) {
	v.x *= s;
	v.y *= s;
	v.z *= s;
	return v;
}

Vector3& operator/=(Vector3& v, float s) {
	v.x /= s;
	v.y /= s;
	v.z /= s;
	return v;
}

// 2項演算子オーバーロード
// Vector3の足し算
const Vector3 operator+(const Vector3& v1, const Vector3& v2) {
	Vector3 temp(v1);
	return temp += v2;
}
Vector3 operator+(const Vector3& v) { return v; }

const Vector3 operator-(const Vector3& v1, const Vector3& v2) {
	Vector3 temp(v1);
	return temp -= v2;
}
Vector3 operator-(const Vector3& v) { return Vector3(-v.x, -v.y, -v.z); }

// 2項演算子オーバーロード
// Vector3の掛け算
const Vector3 operator*(const Vector3& v, float s) {
	Vector3 temp(v);
	return temp *= s;
}
const Vector3 operator*(float s, const Vector3& v) { return v * s; }

const Vector3 operator/(const Vector3& v, float s) {
	Vector3 temp(v);
	return temp /= s;
}


float EaseInOut(float x1, float x2, float t) { 
	float easedT = -(std::cosf(std::numbers::pi_v<float> * t) - 1.0f) / 2.0f;
	return Lerp(x1, x2, easedT);
}

float Lerp(float x1, float x2, float t) { return (1.0f - t) * x1 + t * x2; }

Vector3 Leap(const Vector3& v1, const Vector3& v2, float t) { 
	return Vector3(Lerp(v1.x, v2.x, t), Lerp(v1.y, v2.y, t), Lerp(v1.z, v2.z, t));
}

bool IsCollision(const AABB& aabb1, const AABB& aabb2) {
	return (aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) && // x軸
			(aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y) && // y軸
			(aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z);   // z軸
}

//アフィン変換行列の作成
Matrix4x4 PlayerAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	//エラー対策（使用しない）
	Vector3 dm = scale;
	
	//回転＊平行移動だけをワールド変換行列に
	Matrix4x4 ansMat = Multiply(MakeRotateYMatrix(rotate.y), MakeTranslateMatrix(translate));

	return ansMat;
}