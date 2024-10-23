#define NOMINMAX
#include "Player.h"
#include <cassert>
#include <DebugText.h>

void Player::Initialize(Model* model, ViewProjection* viewProjection, const Vector3& position) {
	// NULLポインタチェック
	assert(model);

	model_ = model;
	//textureHandle_ = textureHandle;
	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	viewProjection_ = viewProjection;
}

void Player::Update() {
	//移動入力①
	inputMove();

	// 衝突情報を初期化②
	CollisionMapInfo collisionMapInfo = {};
	// 移動量に速度の値をコピー
	collisionMapInfo.move = velocity_;
	collisionMapInfo.landing = false;
	collisionMapInfo.hitWall = false;

	// マップ衝突チェック②
	ChecMapCollision(collisionMapInfo);

	//移動③
	CheckMapMove(collisionMapInfo);

	//天井に接触している場合の処理④
	CheckMapCelling(collisionMapInfo);

	//⑤
	CheckMapHItWall(collisionMapInfo);
	

	//接地判定⑥
	UpdateOnGround(collisionMapInfo);

	//ワンボタン処理
	OneAction();
	
	//旋回制御⑦
	AnimateTurn();

	////行列を定数バッファに転送
	//worldTransform_.TransferMatrix();
	// 行列計算⑧
	worldTransform_.UpdateMatrix();

	time += 1;

	if (time>3600) {
		isGoal_ = true;
		score100 = score_ / 100;
		score_ = score_ % 100;
		score10 = score_ / 10;
		score_ = score_ % 10;
		score1 = score_ / 1;
	}// ゴールに達した場合、clearScene_にスコアを設定
    if (isGoal_ && clearScene_) { // nullチェック
        clearScene_->SetScore(score100, score10, score1);
    }
}

void Player::Draw() { 
	//3Dモデルを描画
	model_->Draw(worldTransform_, *viewProjection_);
}

Vector3 Player::GetWorldPosition() {
	//ワールド座標を入れる変数
	Vector3 worldPos;
	//ワールド行列の平行移動成分を取得(ワールド座標)
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

AABB Player::GetAABB() { 
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

void Player::OnCollision(const Enemy* enemy) {
	(void)enemy;
	// デスフラグを立てる
	isDead_ = true;
}

void Player::OnCollision(const Goal* goal) {
	(void)goal;
	// ゴールを立てる
	isGoal_ = true;
}

void Player::inputMove() {
	// 移動入力
	if (onGround_) {
		// 左右移動操作
		if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {

			// 左右加速
			Vector3 acceleration = {};
			if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
				// 左移動中の右入力
				if (velocity_.x < 0.0f) {
					// 速度と逆方向に入力中は急ブレーキ
					velocity_.x *= (1.0f - kAttenuation);
				}

				acceleration.x += kAcceleration / 60.0f;

				if (lrDirection_ != LRDirection::kRight) {
					lrDirection_ = LRDirection::kRight;
					// 旋回開始時の角度
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					// 旋回タイマー
					turnTimer_ = kTimeTurn;
				}
			} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
				// 右移動中の左入力
				if (velocity_.x > 0.0f) {
					// 速度と逆方向に入力中は急ブレーキ
					velocity_.x *= (1.0f - kAttenuation);
				}

				acceleration.x -= kAcceleration / 60.0f;

				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;
					// 旋回開始時の角度
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					// 旋回タイマー
					turnTimer_ = kTimeTurn;
				}
			}
			// 加速/減速
			velocity_ += acceleration;
			// 最大速度制限
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
		} else {
			// 非入力時は移動減衰をかける
			velocity_.x *= (1.0f - kAttenuation);
		}
		if (std::abs(velocity_.x) <= 0.0001f) {
			velocity_.x = 0.0f;
		}
		if (Input::GetInstance()->PushKey(DIK_UP)) {
			// ジャンプ初速
			velocity_ += Vector3(0, kJumpAcceleration / 60.0f, 0);
		}
	} else {
		// 落下速度
		velocity_ += Vector3(0, -kGravityAccleration / 60.0f, 0);
		// 落下速度制限
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}
}

void Player::ChecMapCollision(CollisionMapInfo& info) { 
	ChecMapCollisionUp(info);
	ChecMapCollisionDown(info);
	ChecMapCollisionRight(info);
	ChecMapCollisionLeft(info);
}

void Player::OnItemPickup() {
	// アイテム取得時の処理
	score_ += 1;
	DebugText::GetInstance()->ConsolePrintf("Item picked up! Score: %d", score_);
}

void Player::OnItemPickup10() {
	// アイテム取得時の処理
	score_ += 10;
	DebugText::GetInstance()->ConsolePrintf("Item picked up! Score: %d", score_);
}

void Player::ChecMapCollisionUp(CollisionMapInfo& info) {
	// 上昇あり？
	if (info.move.y <= 0) {
		return;
	}

	//移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionNew;

	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}
	
	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真上の当たり判定を行う
	bool hit = false;
	//ダメージ
	bool damage = false;
	// 左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	if (mapChipType == MapChipType::kDamageBlock && mapChipTypeNext != MapChipType::kDamageBlock) {
		hit = true;
		damage = true;
	}
	if (mapChipType == MapChipType::kItem) {
		// アイテム取得時の処理
		OnItemPickup();

		// アイテムの当たり判定を消すため、マップチップを空白に設定
		mapChipField_->SetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex, MapChipType::kBlank);
	}
	if (mapChipType == MapChipType::kItems10) {
		// アイテム取得時の処理
		OnItemPickup10();

		// アイテムの当たり判定を消すため、マップチップを空白に設定
		mapChipField_->SetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex, MapChipType::kBlank);
	}
	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	// ブロック
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	} // ダメージブロック
	if (mapChipType == MapChipType::kDamageBlock && mapChipTypeNext != MapChipType::kDamageBlock) {
		hit = true;
		damage = true;
	} // アイテム
	if (mapChipType == MapChipType::kItem) {
		// アイテム取得時の処理
		OnItemPickup();

		// アイテムの当たり判定を消すため、マップチップを空白に設定
		mapChipField_->SetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex, MapChipType::kBlank);

	} // アイテム10個
	if (mapChipType == MapChipType::kItems10) {
		// アイテム取得時の処理
		OnItemPickup10();

		// アイテムの当たり判定を消すため、マップチップを空白に設定
		mapChipField_->SetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex, MapChipType::kBlank);
	} // ブロックにヒット
	if (hit) {
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(0, +kHeight / 2.0f, 0));
		if (indexSetNow.yIndex != indexSet.yIndex) {
			// めり込みを排除する方向に移動量を設定する
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(0, +kHeight / 2.0f, 0));
			// めり込み先のブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.move.y = std::max(0.0f, rect.bottom - worldTransform_.translation_.y - (kHeight / 2.0f + kBlank));
			// 天井に当たったことを記録する
			info.ceiling = true;
			//ダメージブロックに当たったら
			if (damage) {
				Hit();
			}
		}
	}
	
}

void Player::ChecMapCollisionDown(CollisionMapInfo& info) {
	// 下降あり
	if (info.move.y >= 0) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionNew;

	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真上の当たり判定を行う
	bool hit = false;
	//ダメージ
	bool damage = false;
	// 左下点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	//ブロック
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}//ダメージブロック
	if (mapChipType == MapChipType::kDamageBlock && mapChipTypeNext != MapChipType::kDamageBlock) {
		hit = true;
		damage = true;
	}//アイテム
	if (mapChipType == MapChipType::kItem) {
		// アイテム取得時の処理
		OnItemPickup();

		// アイテムの当たり判定を消すため、マップチップを空白に設定
		mapChipField_->SetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex, MapChipType::kBlank);

	}//アイテム10個
	if (mapChipType == MapChipType::kItems10) {
		// アイテム取得時の処理
		OnItemPickup10();

		// アイテムの当たり判定を消すため、マップチップを空白に設定
		mapChipField_->SetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex, MapChipType::kBlank);
	} // 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	// ブロック
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	} // ダメージブロック
	if (mapChipType == MapChipType::kDamageBlock && mapChipTypeNext != MapChipType::kDamageBlock) {
		hit = true;
		damage = true;
	} // アイテム
	if (mapChipType == MapChipType::kItem) {
		// アイテム取得時の処理
		OnItemPickup();

		// アイテムの当たり判定を消すため、マップチップを空白に設定
		mapChipField_->SetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex, MapChipType::kBlank);

	} // アイテム10個
	if (mapChipType == MapChipType::kItems10) {
		// アイテム取得時の処理
		OnItemPickup10();

		// アイテムの当たり判定を消すため、マップチップを空白に設定
		mapChipField_->SetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex, MapChipType::kBlank);
	} // ブロックにヒット？
	if (hit) {
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(0, -kHeight / 2.0f, 0));
		if (indexSetNow.yIndex != indexSet.yIndex) {
			// めり込みを排除する方向に移動量を設定する
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(0, -kHeight / 2.0f, 0));
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.move.y = std::min(0.0f, rect.top - worldTransform_.translation_.y + (kHeight / 2.0f + kBlank));
			info.landing = true;
			//ダメージブロックに当たったら
			if (damage) {
				Hit();
			}
		}
	}
}

void Player::ChecMapCollisionRight(CollisionMapInfo& info) { 
	// 右移動あり？
	if (info.move.x <= 0) {
		return;
	}

	std::array<Vector3, kNumCorner> positionNew;

	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真上の当たり判定を行う
	bool hit = false;
	//ダメージ
	bool damage = false;
	// 右上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
	// ブロック
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	} // ダメージブロック
	if (mapChipType == MapChipType::kDamageBlock && mapChipTypeNext != MapChipType::kDamageBlock) {
		hit = true;
		damage = true;
	} // アイテム
	if (mapChipType == MapChipType::kItem) {
		// アイテム取得時の処理
		OnItemPickup();

		// アイテムの当たり判定を消すため、マップチップを空白に設定
		mapChipField_->SetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex, MapChipType::kBlank);

	} // アイテム10個
	if (mapChipType == MapChipType::kItems10) {
		// アイテム取得時の処理
		OnItemPickup10();

		// アイテムの当たり判定を消すため、マップチップを空白に設定
		mapChipField_->SetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex, MapChipType::kBlank);
	} // 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
	// ブロック
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	} // ダメージブロック
	if (mapChipType == MapChipType::kDamageBlock && mapChipTypeNext != MapChipType::kDamageBlock) {
		hit = true;
		damage = true;
	} // アイテム
	if (mapChipType == MapChipType::kItem) {
		// アイテム取得時の処理
		OnItemPickup();

		// アイテムの当たり判定を消すため、マップチップを空白に設定
		mapChipField_->SetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex, MapChipType::kBlank);

	} // アイテム10個
	if (mapChipType == MapChipType::kItems10) {
		// アイテム取得時の処理
		OnItemPickup10();

		// アイテムの当たり判定を消すため、マップチップを空白に設定
		mapChipField_->SetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex, MapChipType::kBlank);
	} // ブロックにヒット？
	if (hit) {
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(+kWidth / 2.0f, 0, 0));
		if (indexSetNow.xIndex != indexSet.xIndex) {
			// めり込みを排除する方向に移動量を設定する
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(+kWidth / 2.0f, 0, 0));
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.move.x = std::max(0.0f, rect.left - worldTransform_.translation_.x - (kWidth / 2.0f + kBlank));
			info.hitWall = true;
			//ダメージブロックに当たったら
			if (damage) {
				Hit();
			}
		}
	}
}

void Player::ChecMapCollisionLeft(CollisionMapInfo& info) {
	// 左移動あり？
	if (info.move.x >= 0) {
		return;
	}

	std::array<Vector3, kNumCorner> positionNew;

	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	bool hit = false;
	//ダメージ
	bool damage = false;
	// 左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
	// ブロック
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	} // ダメージブロック
	if (mapChipType == MapChipType::kDamageBlock && mapChipTypeNext != MapChipType::kDamageBlock) {
		hit = true;
		damage = true;
	} // アイテム
	if (mapChipType == MapChipType::kItem) {
		// アイテム取得時の処理
		OnItemPickup();

		// アイテムの当たり判定を消すため、マップチップを空白に設定
		mapChipField_->SetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex, MapChipType::kBlank);

	} // アイテム10個
	if (mapChipType == MapChipType::kItems10) {
		// アイテム取得時の処理
		OnItemPickup10();

		// アイテムの当たり判定を消すため、マップチップを空白に設定
		mapChipField_->SetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex, MapChipType::kBlank);
	} // 左下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
	// ブロック
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	} // ダメージブロック
	if (mapChipType == MapChipType::kDamageBlock && mapChipTypeNext != MapChipType::kDamageBlock) {
		hit = true;
		damage = true;
	} // アイテム
	if (mapChipType == MapChipType::kItem) {
		// アイテム取得時の処理
		OnItemPickup();

		// アイテムの当たり判定を消すため、マップチップを空白に設定
		mapChipField_->SetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex, MapChipType::kBlank);

	} // アイテム10個
	if (mapChipType == MapChipType::kItems10) {
		// アイテム取得時の処理
		OnItemPickup10();

		// アイテムの当たり判定を消すため、マップチップを空白に設定
		mapChipField_->SetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex, MapChipType::kBlank);
	} // ブロックにヒット？
	if (hit) {
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(-kWidth / 2.0f, 0, 0));
		if (indexSetNow.xIndex != indexSet.xIndex) {
			// めり込みを排除する方向に移動量を設定する
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(-kWidth / 2.0f, 0, 0));
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.move.x = std::min(0.0f, rect.right - worldTransform_.translation_.x + (kWidth / 2.0f + kBlank));
			info.hitWall = true;
			//ダメージブロックに当たったら
			if (damage) {
				Hit();
			}
		}
	}
}

void Player::CheckMapMove(const CollisionMapInfo& info) {
	//移動
	worldTransform_.translation_ += info.move;
}

void Player::CheckMapCelling(const CollisionMapInfo& info) {
	//天井に当たった？
	if (info.ceiling) {
		DebugText::GetInstance()->ConsolePrintf("hit celing\n");
		velocity_.y = 0;
	}
}

void Player::CheckMapHItWall(const CollisionMapInfo& info) {
	// 壁接触による減速
	if (info.hitWall) {
		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

void Player::OneAction() {
	// プレイヤーの現在位置を取得
	Vector3 playerPosition = GetWorldPosition();

	// マップのブロック数を取得
	uint32_t numBlockVertical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	// DIK_1 キーが押されたか確認
	if (Input::GetInstance()->PushKey(DIK_1)) {
		// 画面内にある全てのアイテムを取得
		for (uint32_t y = 0; y < numBlockVertical; ++y) {
			for (uint32_t x = 0; x < numBlockHorizontal; ++x) {
				// 指定のブロックのタイプを取得
				MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(x, y);

				// アイテムかどうかを判定
				if (mapChipType == MapChipType::kItem || mapChipType == MapChipType::kItems10) {
					// アイテムの位置を取得
					Vector3 itemPosition = mapChipField_->GetMapChipPositionByIndex(x, y);

					// アイテムをプレイヤーの方に移動
					MoveItemToPlayer(itemPosition, playerPosition, mapChipType);

					// アイテムがプレイヤーに近づいたら処理
					if (IsCloseToPlayer(itemPosition, playerPosition)) {
						// アイテムを回収
						if (mapChipType == MapChipType::kItem) {
							OnItemPickup(); // スコアを1つ増加
						} else if (mapChipType == MapChipType::kItems10) {
							OnItemPickup10(); // スコアを10増加
						}
						// 取得済みアイテムを消す
						mapChipField_->SetMapChipTypeByIndex(x, y, MapChipType::kBlank);
					}
				}
			}
		}
		// デバッグ出力でアイテムが回収されたことを表示
		DebugText::GetInstance()->ConsolePrintf("Items are gathering towards the player!");
	}
}

// アイテムをプレイヤーの方に移動させる関数
void Player::MoveItemToPlayer(Vector3& itemPosition, const Vector3& playerPosition, MapChipType mapChipType) {
	(void)mapChipType;

	// 移動速度
	float speed = 0.1f;


	// アイテムがプレイヤーに向かって移動するベクトルを計算
	Vector3 direction = playerPosition - itemPosition;
	float distance = Length(direction); // ベクトルの長さを計算

	// 方向を単位ベクトルに正規化
	if (distance > 0.0f) {
		direction /= distance; // 正規化
		// アイテムを移動
		itemPosition += direction * speed;
	}
}

// プレイヤーに近づいたかどうかを判定する関数
bool Player::IsCloseToPlayer(const Vector3& itemPosition, const Vector3& playerPosition) {
	float closeDistance = 0.1f; // プレイヤーとの近接距離
	Vector3 distanceVector = itemPosition - playerPosition;

	// 先ほど作成した Length 関数を使って距離を計算
	return Length(distanceVector) < closeDistance;
}

void Player::UpdateOnGround(const CollisionMapInfo& info) {
	//自キャラが接地状態
	if (onGround_) {
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			// 空中状態に以降
			onGround_ = false;
		} else {
			std::array<Vector3, kNumCorner> positionNew;

			for (uint32_t i = 0; i < positionNew.size(); ++i) {
				positionNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
			}

			// 真下の当たり判定を行う
			bool ground = false;

			MapChipType mapChipType;
			// 左下点の判定
			MapChipField::IndexSet indexSet;

			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom] + Vector3(0, -kGroundSearchHeight, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

			if (mapChipType == MapChipType::kBlock) {
				ground = true;
			}
			if (mapChipType == MapChipType::kDamageBlock) {
				ground = true;
			}
			if (mapChipType == MapChipType::kItem) {
				ground = true;
			}
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom] + Vector3(0, -kGroundSearchHeight, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

			if (mapChipType == MapChipType::kBlock) {
				ground = true;
			}
			if (mapChipType == MapChipType::kDamageBlock) {
				ground = true;
			}
			if (mapChipType == MapChipType::kItem) {
				ground = true;
			}

			if (!ground) {
				//DebugText::GetInstance()->ConsolePrintf("jump");
				onGround_ = false;
			}
		}
	} else {
		// 着地
		if (info.landing) {
			velocity_.x *= (1.0f - kAttenuationLanding);
			velocity_.y = 0.0f;

			//DebugText::GetInstance()->ConsolePrintf("onGround");
			onGround_ = true;
		}
	}
}

void Player::AnimateTurn() {
		// 旋回制御
	if (turnTimer_ > 0.0f) {
		turnTimer_ = std::max(turnTimer_ - (1.0f / 60.0f), 0.0f);

		// 左右の自キャラ角度テーブル
		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};
		// 状態に応じた角度を取得する
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		// 自キャラの角度を設定する
		worldTransform_.rotation_.y = EaseInOut(destinationRotationY, turnFirstRotationY_, turnTimer_ / kTimeTurn);
	}
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {
	/*if (corner == kRightBottom) {
	            return center + {+kWidth / 2.0f, -kHeight / 2.0f, 0};
	        }
	        else if (corner == kLeftBottom) {
	            return center + {-kWidth / 2.0f, -kHeight / 2.0f, 0};
	        }
	        else if (corner == kRightTop) {
	            return center + {+kWidth / 2.0f, -kHeight / 2.0f, 0};
	        }
	        else {
	            return center + {-kWidth / 2.0f, -kHeight / 2.0f, 0};
	        }*/
	Vector3 offsetTable[kNumCorner] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0}, //  kRightBottom
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0}, //  kLeftBottom
	    {+kWidth / 2.0f, +kHeight / 2.0f, 0}, //  kRightTop
	    {-kWidth / 2.0f, +kHeight / 2.0f, 0}  //  kLeftTop
	};

	    return center + offsetTable[static_cast<uint32_t>(corner)];
    }