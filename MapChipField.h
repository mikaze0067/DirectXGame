#pragma once

#include <cstdint>
#include <vector>
#include <Vector3.h>
#include <string>

enum class MapChipType {
	kBlank, //空白
	kBlock, //ブロック
	kDamageBlock, // ダメージブロック
	kItem,        // アイテム
	kItems10,     ///複数アイテム（10）
};

struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};

class MapChipField {
public:
	//1ブロックのサイズ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

	void ResetMapChipData();

	void LoadMapChipCsv(const std::string& filePath);

	uint32_t GetNumBlockVirtical();

	uint32_t GetNumBlockHorizontal();

	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);

	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	// 新しく追加するメソッドの宣言
	void SetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex, MapChipType newType); // 追加

	struct IndexSet {
		uint32_t xIndex;
		uint32_t yIndex;
	};

	IndexSet GetMapChipIndexSetByPosition(const Vector3& position);
	
	//範囲矩形
	struct Rect {
		float left;
		float right;
		float bottom;
		float top;
	};

	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);

	private:

	// ブロックの個数
	static inline const uint32_t kNumBlockVirtical = 30;
	static inline const uint32_t kNumBlockHorizontal = 75;

	MapChipData mapChipData_;
};