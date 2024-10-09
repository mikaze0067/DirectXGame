#include <ViewProjection.h>

class Player;

class CameraController {
public: // メンバ関数
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update();

	void SetTarget(Player* target) { target_ = target; }

	void Reset();

	const ViewProjection& GetViewProjection() const { return viewProjection_; }

	// 矩形
	struct Rect {
		float left = 0.0f;
		float right = 1.0f;
		float bottom = 0.0f;
		float top = 1.0f;
	};

	void SetMovableArea(Rect area) { movableArea_ = area; }

	//カメラの目標座標
	Vector3 cameraTarget;

	private: // メンバ変数

	// ビュープロジェクション
	ViewProjection viewProjection_;

	Player* target_ = nullptr;

	Vector3 targetOffset_ = {0, 0, -15.0f};

	//カメラ移動範囲
	Rect movableArea_ = {0, 100, 0, 100};

	//座標補間割合
	static inline const float kInterpolationRate = 0.1f;
	//速度掛け率
	static inline const float kVelocityBias = 30.0f;
	//追従対象の各方向へのカメラ移動範囲
	static inline const Rect margin = {-9.0f, 9.0f, -5.0f, 5.0f};
};