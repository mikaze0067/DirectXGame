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

	//temerのリソース
	num[0] = TextureManager::Load("./Resources/0.png");
	num[1] = TextureManager::Load("./Resources/1.png");
	num[2] = TextureManager::Load("./Resources/2.png");
	num[3] = TextureManager::Load("./Resources/3.png");
	num[4] = TextureManager::Load("./Resources/4.png");
	num[5] = TextureManager::Load("./Resources/5.png");
	num[6] = TextureManager::Load("./Resources/6.png");
	num[7] = TextureManager::Load("./Resources/7.png");
	num[8] = TextureManager::Load("./Resources/8.png");
	num[9] = TextureManager::Load("./Resources/9.png");

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
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(35, 16);
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

	// ゴールの生成
	for (int32_t i = 1; i < 52; ++i) {
		modelGoal_ = Model::CreateFromOBJ("enemy", true);
		Goal* newgoal_ = new Goal();

		Vector3 goalPosition = mapChipField_->GetMapChipPositionByIndex(99, 18);

		newgoal_->Initialize(modelGoal_, &viewProjection_, goalPosition);

		goal_.push_back(newgoal_);
	}

	// ダメージブロックの生成
	modelDamageBlock_ = Model::CreateFromOBJ("player", true);

	//アイテムの生成
	modelItem_ = Model::CreateFromOBJ("enemy", true);

		// アイテムの生成
	modelItems10_ = Model::CreateFromOBJ("cube", true);

	// ゲームプレイフェーズから開始
	phase_ = Phase::kPlay;

	//クリア
	clear_ = new ClearScene();

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
	CameraController::Rect cameraArea = {10.0f, 80.0f, -50.0f, 30.0f};
	//
	cameraController_->SetMovableArea(cameraArea);

	GenerateBlocks();

}

void GameScene::Update() {

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

		for (Goal* goal : goal_) {

			goal->Update();
		}

		// timer６０秒
		timerscene -= 1;
		time60 = int(timerscene / 60);

		time10 = time60 / 10;
		time60 = time60 % 10;
		Time1 = time60 / 1;

		num_ = Sprite::Create(num[time10], {260, 80});
		num_1 = Sprite::Create(num[Time1], {560, 80});
		;

		ChangePhase();
		if (timerscene == 0) {
			timerscene = 3600;
		}
		// カメラコントローラの更新
		cameraController_->Update();

		// カメラ処理
		viewProjection_.matView = cameraController_->GetViewProjection().matView;
		viewProjection_.matProjection = cameraController_->GetViewProjection().matProjection;
		// ビュープロジェクション行列の更新と転送
		viewProjection_.TransferMatrix();

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

		// ゴールの当たり判定を行う
		CheckGoalCollisions();

		break;

	case Phase::kDeath:

		// 天球の更新
		skydome_->Update();

		// 敵キャラの更新
		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		for (Goal* goal : goal_) {
			goal->Update();
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
	case Phase::kGoal:
		finished_ = true;
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
	
	switch (phase_) {

	case Phase::kPlay:

		num_->Draw();

		num_1->Draw();

		break;
	case Phase::kClear:

		clear_->Draw();

		break;

	}






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

	for (Goal* goal : goal_) {
		goal->Draw();
	}

	if (deathParticles_) {
		deathParticles_->Draw();
	}

	// 天球の描画
	// skydome_->Draw();

	// 縦横ブロック描画
	for (uint32_t i = 0; i < worldTransformBlocks_.size(); ++i) {
		for (uint32_t j = 0; j < worldTransformBlocks_[i].size(); ++j) {
			WorldTransform* worldTransformBlock = worldTransformBlocks_[i][j];
			if (!worldTransformBlock)
				continue;

			// インデックスとしてiをyIndex、jをxIndexとして扱う
			uint32_t xIndex = j;
			uint32_t yIndex = i;

			// kBlockの場合
			if (mapChipField_->GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kBlock) {
				modelBlock_->Draw(*worldTransformBlock, viewProjection_);
			}
			// kDamageBlockの場合
			else if (mapChipField_->GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kDamageBlock) {
				modelDamageBlock_->Draw(*worldTransformBlock, viewProjection_);
			} 
			// kItemの場合
			else if (mapChipField_->GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kItem) {
				modelItem_->Draw(*worldTransformBlock, viewProjection_);
			}
			// kItems10個の場合
			else if (mapChipField_->GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kItems10) {
				modelItems10_->Draw(*worldTransformBlock, viewProjection_);
			}
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
		if (player_->IsGoal()) {
			phase_ = Phase::kGoal;
		}

		break;
	case Phase::kDeath:

		if (deathParticles_ && deathParticles_->IsFinished()) {
			finished_ = true;
		}

		break;
	case Phase::kGoal:
		phase_ = Phase::kClear;
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

void GameScene::CheckGoalCollisions() {

	// 判定対象1と2の座標
	AABB aabb1, aabb2;

	// 自キャラの座標
	aabb1 = player_->GetAABB();

	// 自キャラとgola
	for (Goal* goal : goal_) {
		// goalの座標
		aabb2 = goal->GetAABB();

		// AABB同士の交差判定
		if (IsCollision(aabb1, aabb2)) {
			// 自キャラの衝突時コールバックを呼び起こす
			player_->OnCollision(goal);
			// ゴール時
			goal->OnCollision(player_);
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
			MapChipType type = mapChipField_->GetMapChipTypeByIndex(j, i);

			if (type == MapChipType::kBlock) {
				// 通常ブロックの生成処理
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			} else if (type == MapChipType::kDamageBlock) {
				// ダメージブロックの生成処理
				WorldTransform* damageBlock = new WorldTransform();
				damageBlock->Initialize();
				worldTransformBlocks_[i][j] = damageBlock;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			} else if (type == MapChipType::kItem) {
				// アイテムの生成処理
				WorldTransform* item = new WorldTransform();
				item->Initialize();
				worldTransformBlocks_[i][j] = item;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			} else if (type == MapChipType::kItems10) {
				// アイテム10個の生成処理
				WorldTransform* items10 = new WorldTransform();
				items10->Initialize();
				worldTransformBlocks_[i][j] = items10;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			} else {
				worldTransformBlocks_[i][j] = nullptr;
			}
		}
	}
}
			