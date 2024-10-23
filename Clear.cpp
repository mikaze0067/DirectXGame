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
	score_100 = Sprite::Create(num[score100], {260, 80});
	score_10 = Sprite::Create(num[score10], {560, 80});
	score_1 = Sprite::Create(num[score1], {360, 80});
}
