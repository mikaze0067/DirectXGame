#define NOMINMAX
#include "CameraController.h"
#include "Player.h"
#include <MathUtilityForText.h>


void CameraController::Initialize() {
	// ビュープロジェクションの初期化
	viewProjection_.Initialize();
}

void CameraController::Update() {
	const Vector3& targetVelocity = target_->GetVelocity();

	//追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	// 追従対象とオフセットからカメラの目標座標を計算
	cameraTarget = targetWorldTransform.translation_ + targetOffset_ + targetVelocity * kVelocityBias;

	//座標補間によりゆったり追従
	viewProjection_.translation_ = Leap(viewProjection_.translation_, cameraTarget, kInterpolationRate);

	//追従対象が画面外に出ないように補正
	viewProjection_.translation_.x = std::max(viewProjection_.translation_.x, targetWorldTransform.translation_.x + margin.left);
	viewProjection_.translation_.x = std::min(viewProjection_.translation_.x, targetWorldTransform.translation_.x + margin.right);
	viewProjection_.translation_.y = std::max(viewProjection_.translation_.y, targetWorldTransform.translation_.y + margin.bottom);
	viewProjection_.translation_.y = std::min(viewProjection_.translation_.y, targetWorldTransform.translation_.y + margin.top);
	
	//移動範囲制限
	viewProjection_.translation_.x = std::max(viewProjection_.translation_.x, movableArea_.left);
	viewProjection_.translation_.x = std::min(viewProjection_.translation_.x, movableArea_.right);
	viewProjection_.translation_.y = std::max(viewProjection_.translation_.y, movableArea_.bottom);
	viewProjection_.translation_.y = std::min(viewProjection_.translation_.y, movableArea_.top);
	
	//行列を更新する
	viewProjection_.UpdateMatrix();
}

void CameraController::Reset() {
	//追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	//追従対象とオフセットからカメラの座標を計算
	viewProjection_.translation_ = targetWorldTransform.translation_ + targetOffset_;
}
