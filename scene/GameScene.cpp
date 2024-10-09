#include "GameScene.h"
#include "TextureManager.h"
#include <Affine.h>
#include <cassert>

GameScene::GameScene() {}

GameScene::~GameScene() {
	delete model_;
	delete player_;
	// delete enemy_;
	for (Enemy* newEnemy : enemies_) {
		delete newEnemy;
	}
	enemies_.clear();

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();

	delete debugCamera_;

	delete modelSkydome_;

	delete mapChipField_;

	delete cameraController_;

	delete modelEnemy_;

	if (deathParticles_) {
		delete deathParticles_;
	}

	delete modelDeathParticles;
}

void GameScene::Initialize() {

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();

	// ファイル名を指定してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("./Resources/cube/cube.jpg");

	// 3Dモデルの生成
	modelBlock_ = Model::CreateFromOBJ("block", true);
	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	// ビュープロジェクションの初期化
	viewProjection_.Initialize();

	// マップチップフィールドの生成と初期化
	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("./Resources/map.csv");

	// 座標をマップチップ番号で指定
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(2, 15);
	// 座標をマップチップ番号で指定
	// Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(10, 18);

	// 自キャラの生成
	player_ = new Player();
	model_ = Model::CreateFromOBJ("player", true);

	// 自キャラの初期化
	player_->Initialize(model_, &viewProjection_, playerPosition);

	player_->SetMapChipField(mapChipField_);

	// 敵キャラの生成
	// enemy_ = new Enemy();
	for (int32_t i = 0; i < 2; ++i) {
		modelEnemy_ = Model::CreateFromOBJ("enemy", true);
		Enemy* newEnemy = new Enemy();
		Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(15, 18);
		newEnemy->Initialize(modelEnemy_, &viewProjection_, enemyPosition);

		enemies_.push_back(newEnemy);
	}

	// 敵キャラの初期化
	/*enemy_->Initialize(modelEnemy_, &viewProjection_, enemyPosition);
	enemy_->SetMapChipField(mapChipField_);*/

	// ゲームプレイフェーズから開始
	phase_ = Phase::kPlay;

	// 天球の生成
	skydome_ = new Skydome();
	// 天球3Dモデルの生成
	modelSkydome_ = Model::CreateFromOBJ("sphere", true);
	// 天球の初期化
	skydome_->Initialize(modelSkydome_, &viewProjection_);

	// カメラコントローラの生成
	cameraController_ = new CameraController();
	// カメラコントローラの初期化
	cameraController_->Initialize();
	// 追従対象をセット
	cameraController_->SetTarget(player_);
	// リセット
	cameraController_->Reset();
	//
	CameraController::Rect cameraArea = {12.0f, 100 - 12.0f, 6.0f, 6.0f};
	//
	cameraController_->SetMovableArea(cameraArea);

	GenerateBlocks();

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);
}

void GameScene::Update() {

#ifdef _DEBUG
	if (input_->TriggerKey(DIK_SPACE)) {
		if (isDebugCameraActive_ == true)
			isDebugCameraActive_ = false;
		else
			isDebugCameraActive_ = true;
	}
#endif

	switch (phase_) {
	case Phase::kPlay:

		// 天球の更新
		skydome_->Update();

		// 自キャラの更新
		player_->Update();

		// 敵キャラの更新
		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		ChangePhase();

		// カメラコントローラの更新
		cameraController_->Update();

		// カメラ処理
		if (isDebugCameraActive_) {
			// デバッグカメラの更新
			debugCamera_->Update();
			viewProjection_.matView = debugCamera_->GetViewProjection().matView;
			viewProjection_.matProjection = debugCamera_->GetViewProjection().matProjection;
			// ビュープロジェクション行列の転送
			viewProjection_.TransferMatrix();
		} else {
			viewProjection_.matView = cameraController_->GetViewProjection().matView;
			viewProjection_.matProjection = cameraController_->GetViewProjection().matProjection;
			// ビュープロジェクション行列の更新と転送
			viewProjection_.TransferMatrix();
		}

		// 縦横ブロック更新
		for (std::vector<WorldTransform*> worldTransformBlockTate : worldTransformBlocks_) {
			for (WorldTransform* worldTransformBlockYoko : worldTransformBlockTate) {
				if (!worldTransformBlockYoko)
					continue;

				// アフィン変換行列の作成
				//(MakeAffineMatrix：自分で作った数学系関数)
				worldTransformBlockYoko->matWorld_ = MakeAffineMatrix(worldTransformBlockYoko->scale_, worldTransformBlockYoko->rotation_, worldTransformBlockYoko->translation_);

				// 定数バッファに転送
				worldTransformBlockYoko->TransferMatrix();
			}
		}

		// 全ての当たり判定を行う
		CheckAllCollisions();

		break;

	case Phase::kDeath:

		// 天球の更新
		skydome_->Update();

		// 敵キャラの更新
		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		//
		if (deathParticles_) {
			deathParticles_->Update();
		}
		if (Input::GetInstance()->PushKey(DIK_R)) {
			finished_ = true;
		}

		// 縦横ブロック更新
		for (std::vector<WorldTransform*> worldTransformBlockTate : worldTransformBlocks_) {
			for (WorldTransform* worldTransformBlockYoko : worldTransformBlockTate) {
				if (!worldTransformBlockYoko)
					continue;

				// アフィン変換行列の作成
				//(MakeAffineMatrix：自分で作った数学系関数)
				worldTransformBlockYoko->matWorld_ = MakeAffineMatrix(worldTransformBlockYoko->scale_, worldTransformBlockYoko->rotation_, worldTransformBlockYoko->translation_);

				// 定数バッファに転送
				worldTransformBlockYoko->TransferMatrix();
			}
		}

		break;
	}
}

void GameScene::Draw() {

	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに背景スプライトの描画処理を追加できる
	/// </summary>

	// スプライト描画後処理
	Sprite::PostDraw();
	// 深度バッファクリア
	dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画前処理
	Model::PreDraw(commandList);

	/// <summary>
	/// ここに3Dオブジェクトの描画処理を追加できる
	/// </summary>
	// 3Dモデル描画
	// model_->Draw(worldTransform_, viewProjection_, playertextureHandle_);
	if (player_->IsDead() == false) {
		// 自キャラの描画
		player_->Draw();
	}

	// 敵キャラの描画
	for (Enemy* enemy : enemies_) {
		enemy->Draw();
	}

	if (deathParticles_) {
		deathParticles_->Draw();
	}

	// 天球の描画
	skydome_->Draw();

	// 縦横ブロック描画
	for (std::vector<WorldTransform*> worldTransformBlockTate : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlockYoko : worldTransformBlockTate) {
			if (!worldTransformBlockYoko)
				continue;

			modelBlock_->Draw(*worldTransformBlockYoko, viewProjection_);
		}
	}

	// 3Dオブジェクト描画後処理
	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに前景スプライトの描画処理を追加できる
	/// </summary>

	// スプライト描画後処理
	Sprite::PostDraw();

#pragma endregion
}

void GameScene::ChangePhase() {
	switch (phase_) {
	case Phase::kPlay:
		if (player_->IsDead()) {
			// 死亡演出フェーズに切り替え
			phase_ = Phase::kDeath;
			// 自キャラの座標を取得
			player_->Update();
			const Vector3& deathParticlesPosition = player_->GetWorldPosition();

			// パーティクルの初期化
			modelDeathParticles = Model::CreateFromOBJ("deathParticle", true);

			// パーティクルの生成処理
			deathParticles_ = new DeathParticles;
			deathParticles_->Initialize(modelDeathParticles, &viewProjection_, deathParticlesPosition);
		}

		break;
	case Phase::kDeath:

		if (deathParticles_ && deathParticles_->IsFinished()) {
			finished_ = true;
		}

		break;
	}
}

void GameScene::CheckAllCollisions() {
	// 判定対象1と2の座標
	AABB aabb1, aabb2;

	// 自キャラの座標
	aabb1 = player_->GetAABB();

	// 自キャラと敵弾すべての当たり判定
	for (Enemy* enemy : enemies_) {
		// 敵弾の座標
		aabb2 = enemy->GetAABB();

		// AABB同士の交差判定
		if (IsCollision(aabb1, aabb2)) {
			// 自キャラの衝突時コールバックを呼び起こす
			player_->OnCollision(enemy);
			// 敵弾の衝突時コールバックを呼び起こす
			enemy->OnCollision(player_);
		}
	}
}

void GameScene::GenerateBlocks() {
	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	// 要素数を変更する
	// 列数を設定（縦方向のブロック数）
	worldTransformBlocks_.resize(numBlockVirtical);
	// キューブの生成
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}

	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			} else {
				worldTransformBlocks_[i][j] = nullptr;
			}
		}
	}
}