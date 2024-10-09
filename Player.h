#include <WorldTransform.h>
#include <Model.h>
#include <Vector3.h>
#include <numbers>
#include <Input.h>
#include <MathUtilityForText.h>
#include <algorithm>
#include <AABB.h>

class MapChipField;
class Enemy;

/// <summary>
/// 自キャラ
/// </summary>
class Player{
public:

	// 左右
	enum class LRDirection {
		kRight,
		kLeft,
	};

	// 角
	enum Corner {
		kRightBottom, // 右下
		kLeftBottom,  // 左下
		kRightTop,    // 右上
		kLeftTop,     // 左上

		kNumCorner // 要素数
	};

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="textureHandle">テクスチャハンドル</param>
	void Initialize(Model* model, ViewProjection* viewProjection, const Vector3& position);

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	const WorldTransform& GetWorldTransform() const { return worldTransform_; }

	const Vector3& GetVelocity() const { return velocity_; }

	Vector3 GetWorldPosition();

	// AABBを取得
	AABB GetAABB();

	//衝突応答
	void OnCollision(const Enemy* enemy);

	//デスフラグgetter
	bool IsDead() const { return isDead_; }

	private:

	static inline const float kAcceleration = 0.1f;
	static inline const float kAttenuation = 0.05f;
	// ジャンプ初速（上方向）
	static inline const float kJumpAcceleration = 20.0f;
	// 重力加速度（下方向）
	static inline const float kGravityAccleration = 0.98f;
	static inline const float kAttenuationWall = 0.2f;
	// 着地時の速度減衰率
	static inline const float kAttenuationLanding = 0.5f;
	// 最大落下速度
	static inline const float kLimitFallSpeed = 0.5f;
	static inline const float kLimitRunSpeed = 0.5f;
	// 旋回時間<秒>
	static inline const float kTimeTurn = 0.3f;
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;
	static inline const float kBlank = 0.04f;
	static inline const float kGroundSearchHeight = 0.06f;

	struct CollisionMapInfo {
		bool ceiling = false; // 天井
		bool landing = false; // 着陸
		bool hitWall = false; // 壁
		Vector3 move;
	};

	ViewProjection* viewProjection_ = nullptr;
	// ワールドトランスフォーム
	WorldTransform worldTransform_;
	// 3Dモデル
	Model* model_ = nullptr;
	
	Vector3 velocity_ = {};

	//デスフラグ
	bool isDead_ = false;


	// 接地状態フラグ
	bool onGround_ = true;

	LRDirection lrDirection_ = LRDirection::kRight;

	// 旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;
	// 旋回タイマー
	float turnTimer_ = 0.0f;
	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;

	void inputMove();

	void ChecMapCollision(CollisionMapInfo& info);
	void ChecMapCollisionUp(CollisionMapInfo& info);
	void ChecMapCollisionDown(CollisionMapInfo& info);
	void ChecMapCollisionRight(CollisionMapInfo& info);
	void ChecMapCollisionLeft(CollisionMapInfo& info);

	void CheckMapMove(const CollisionMapInfo& info);

	void CheckMapCelling(const CollisionMapInfo& info);

	void UpdateOnGround(const CollisionMapInfo& info);

	void CheckMapHItWall(const CollisionMapInfo& info);

	void AnimateTurn();

	Vector3 CornerPosition(const Vector3& center, Corner corner);

};