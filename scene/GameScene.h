#pragma once

#include "Audio.h"
#include "CameraController.h"
#include "DeathParticles.h"
#include "DebugCamera.h"
#include "DirectXCommon.h"
#include "Enemy.h"
#include "Goal.h"
#include "Input.h"
#include "Model.h"
#include "Player.h"
#include "Skydome.h"
#include "Sprite.h"
#include "ViewProjection.h"
#include "WorldTransform.h"
#include <MapChipField.h>
#include <vector>
#include "Clear.h"

/// <summary>
/// ゲームシーン
/// </summary>
class GameScene {

public: // メンバ関数
	/// <summary>
	/// コンストクラタ
	/// </summary>
	GameScene();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~GameScene();

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

	void ChangePhase();

	void CheckAllCollisions();

	void CheckGoalCollisions();

	void GenerateBlocks();

	// デスフラグのgetter
	bool IsFinished() const { return finished_; }

	

private: // メンバ変数
	DirectXCommon* dxCommon_ = nullptr;
	Input* input_ = nullptr;
	Audio* audio_ = nullptr;

	/// <summary>
	/// ゲームシーン用
	/// </summary>
	// テクスチャハンドル
	uint32_t textureHandle_ = 0;

	// 3Dモデル
	Model* model_ = nullptr;
	Model* modelBlock_ = nullptr;
	Model* modelEnemy_ = nullptr;
	Model* modelDeathParticles = nullptr;
	Model* modelGoal_ = nullptr;
	Model* modelDamageBlock_ = nullptr;
	Model* modelItem_ = nullptr;
	Model* modelItems10_ = nullptr;

	// ワールドトランスフォーム
	WorldTransform worldTransform_;
	// ビュープロジェクション
	ViewProjection viewProjection_;

	// マップチップフィールド
	MapChipField* mapChipField_;

	// 自キャラ
	Player* player_ = nullptr;

	// 敵キャラ
	std::list<Enemy*> enemies_;
	
	// ゴール
	std::list<Goal*> goal_;

	DeathParticles* deathParticles_ = nullptr;
	// ゲームのフェーズ（型）
	enum class Phase {
		kPlay,  // ゲームプレイ
		kDeath, // デス演出
		kGoal,  // ゴール
		kClear, // クリア
	};
	// ゲームの現在フェーズ（変数）
	Phase phase_;

	ClearScene* clearScene_ = nullptr;

	// 縦横ブロック配列
	std::vector<std::vector<WorldTransform*>> worldTransformBlocks_;

	// デバッグカメラ有効
	bool isDebugCameraActive_ = false;
	// デバッグカメラ
	DebugCamera* debugCamera_ = nullptr;

	// 天球
	Skydome* skydome_ = nullptr;
	// 3Dモデル
	Model* modelSkydome_ = nullptr;

	// カメラコントローラ
	CameraController* cameraController_ = nullptr;
	// 終了フラグ
	bool finished_ = false;




	uint32_t time10 = 0;

	uint32_t Time1 = 0;

	uint32_t timerscene = 3600;

	uint32_t time60 = 0;

	Sprite* num_ = nullptr;

	Sprite* num_1 = nullptr;

	uint32_t num[10];

	
};