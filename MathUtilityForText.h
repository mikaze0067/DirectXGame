#include <Affine.h>
#include "Matrix4x4.h"
#include "Vector3.h"
#include <AABB.h>

Vector3 operator+(const Vector3& v);
Vector3 operator-(const Vector3& v);

// 代入演算子オーバーロード
// Vector3の足算
Vector3& operator+=(Vector3& lhv, const Vector3& rhv);

Vector3& operator-=(Vector3& lhv, const Vector3& rhv);

// 代入演算子オーバーロード
// Vector3の掛け算
Vector3& operator*=(Vector3& v, float s);

Vector3& operator/=(Vector3& v, float s);

// 2項演算子オーバーロード
// Vector3の足し算
const Vector3 operator+(const Vector3& v1, const Vector3& v2);

const Vector3 operator-(const Vector3& v1, const Vector3& v2);

// 2項演算子オーバーロード
// Vector3の掛け算
const Vector3 operator*(const Vector3& v, float s);

const Vector3 operator*(float s, const Vector3& v);

const Vector3 operator/(const Vector3& v, float s);

float EaseInOut(float x1, float x2, float t);
float Lerp(float x1, float x2, float t);
Vector3 Leap(const Vector3& v1, const Vector3& v2, float t);

bool IsCollision(const AABB& aabb1, const AABB& aabb2);

Matrix4x4 PlayerAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);