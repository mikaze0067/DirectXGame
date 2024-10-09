#pragma once
#include <Model.h>
#include <ViewProjection.h>
#include <array>
#include <WorldTransform.h>
#include <math.h>
#include "cmath"
#include <corecrt_math_defines.h>
#include <Affine.h>
#include <MathUtilityForText.h>
#include <algorithm>



class DeathParticles {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Model* model, ViewProjection* viewProjection, const Vector3& position);

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	// デスフラグのgetter
	bool IsFinished() const { return finished_; }



private:

	ViewProjection* viewProjection_ = nullptr;
	// ワールドトランスフォーム
	//WorldTransform worldTransform_;
	// 3Dモデル
	Model* model_ = nullptr;

	//パーティクルの個数
	static inline const uint32_t kNumParticles = 8;

	std::array<WorldTransform, kNumParticles> worldTransforms_;

	//存続時間＜秒＞
	static inline const float kDuration = 2.0f;
	//移動の速さ
	static inline const float kSpeed = 0.05f;
	//分割した一個分の角度
	static inline const float kAngleUint = 2.0f * float(M_PI) / 8.0f;
	//終了フラグ
	bool isFinished_ = false;
	//経過時間カウント
	float counter_ = 0.0f;
	//色変更オブジェクト
	ObjectColor objectColor_;
	//色の数値
	Vector4 color_;
	// 終了フラグ
	bool finished_ = false;

};