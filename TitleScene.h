#pragma once
#include <Input.h>
#include "Model.h"
#include"ViewProjection.h"
#include"WorldTransform.h"


class TitleScene {
public:

	~TitleScene();
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

private:

	ViewProjection viewProjection_;
	WorldTransform worldTransformTitle_;
	WorldTransform worldTransformPlayer_;

	Model* model_ = nullptr;

	float counter_ = 0.0f;
	// 終了フラグ
	bool finished_ = false;
	static inline const float kTimeTitleMove = 2.0f;
};