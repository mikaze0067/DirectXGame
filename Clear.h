#pragma once
#include "Model.h"
#include "ViewProjection.h"
#include "WorldTransform.h"
#include <Input.h>
#include "Player.h"
#include"TextureManager.h"
#include <Sprite.h>

class ClearScene {
public:
	~ClearScene();
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();
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

	int GetScore100() const { return score100; }
	int GetScore10() const { return score10; }
	int GetScore1() const { return score1; }

private:
	ViewProjection viewProjection_;
	WorldTransform worldTransformTitle_;
	WorldTransform worldTransformPlayer_;

	Model* model_ = nullptr;

	float counter_ = 0.0f;
	// 終了フラグ
	bool finished_ = false;
	static inline const float kTimeTitleMove = 2.0f;

	int time = 420;

	int score100;

	int score1;
	int score10;

	uint32_t num[10];

	Sprite* score_100 = nullptr;

	Sprite* score_10 = nullptr;

	Sprite* score_1 = nullptr;
	
};