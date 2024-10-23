#include "Clear.h"


ClearScene::~ClearScene() {}

void ClearScene::Initialize() {
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

}

void ClearScene::Update() {
	time--;
	if (time==0) {
		finished_ = true;
	}
}

void ClearScene::Draw() {
	// 各桁のスコアをスプライトで描画
	scoreSprite100_ = Sprite::Create(num[score_100], {260, 80});
	scoreSprite10_ = Sprite::Create(num[score_10], {360, 80});
	scoreSprite1_ = Sprite::Create(num[score_1], {460, 80});

	// スプライトを描画
	scoreSprite100_->Draw();
	scoreSprite10_->Draw();
	scoreSprite1_->Draw();
}

void ClearScene::SetScore(uint32_t score100, uint32_t score10, uint32_t score1) {
	score_100 = score100;
	score_10 = score10;
	score_1 = score1;
}
