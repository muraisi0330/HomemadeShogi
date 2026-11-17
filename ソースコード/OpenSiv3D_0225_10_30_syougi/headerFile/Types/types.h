#pragma once
#include <Siv3D.hpp>
// マップチップ情報
struct ChipInfo
{
	String name;//名前
	ColorF color;//チップの色
	int cost;//移動コスト（このチップに入るために必要な移動量)
};

// チーム
enum class Team : int32
{
	Neutral,//無所属
	Blue,//青チーム
	Red,//赤チーム
};

// ユニット種別
enum class UnitType : int32
{
	King,//王将
	Rook,//飛車
	Bishop,//角行
	Gold,//金将
	Silver,//銀将
	Knight,//桂馬
	Lance,//香車
	Pawn,//歩兵
	Pawn1,//成り歩兵
	Lance1,//成り香車
	Knight1,//成り桂馬
	Silver1,//成り銀将
	Rook1,//龍王
	Bishop1,//龍馬
	King1,//玉将
};

// ユニット定義（軽量）
struct Unit
{
	 UnitType type = UnitType::Pawn; // 初期値を追加
    Point pos = { 0, 0 };          // 初期値を追加
    Team team = Team::Neutral;     // 初期値を追加
    bool hasMoved = false;
    bool promoteFlag = false;
    bool isOnBoard = true;
    int32 handIndex = 0;
};

// ユニット情報
struct UnitInfo
{
	String name; // 名前
	int speed = 1;// 移動力
};
// 初期データを返すファクトリ関数
static Array<Unit> CreateInitialUnitList()
{
	Array<Unit> list = {
		{UnitType::Pawn,{0,6},Team::Blue},
		{UnitType::Pawn,{1,6},Team::Blue},
		{UnitType::Pawn,{2,6},Team::Blue},
		{UnitType::Pawn,{3,6},Team::Blue},
		{UnitType::Pawn,{4,6},Team::Blue},
		{UnitType::Pawn,{5,6},Team::Blue},
		{UnitType::Pawn,{6,6},Team::Blue},
		{UnitType::Pawn,{7,6},Team::Blue},
		{UnitType::Pawn,{8,6},Team::Blue},
		{UnitType::Lance,{0,8},Team::Blue},
		{UnitType::Knight,{1,8},Team::Blue},
		{UnitType::Silver,{2,8},Team::Blue},
		{UnitType::Gold,{3,8},Team::Blue},
		{UnitType::Lance,{8,8},Team::Blue},
		{UnitType::Knight,{7,8},Team::Blue},
		{UnitType::Silver,{6,8},Team::Blue},
		{UnitType::Gold,{5,8},Team::Blue},
		{UnitType::Bishop,{1,7},Team::Blue},
		{UnitType::Rook,{7,7},Team::Blue},
		{UnitType::King,{4,8},Team::Blue},
		{UnitType::Pawn,{0,2},Team::Red},
		{UnitType::Pawn,{1,2},Team::Red},
		{UnitType::Pawn,{2,2},Team::Red},
		{UnitType::Pawn,{3,2},Team::Red},
		{UnitType::Pawn,{4,2},Team::Red},
		{UnitType::Pawn,{5,2},Team::Red},
		{UnitType::Pawn,{6,2},Team::Red},
		{UnitType::Pawn,{7,2},Team::Red},
		{UnitType::Pawn,{8,2},Team::Red},
		{UnitType::Lance,{0,0},Team::Red},
		{UnitType::Knight,{1,0},Team::Red},
		{UnitType::Silver,{2,0},Team::Red},
		{UnitType::Gold,{3,0},Team::Red},
		{UnitType::Lance,{8,0},Team::Red},
		{UnitType::Knight,{7,0},Team::Red},
		{UnitType::Silver,{6,0},Team::Red},
		{UnitType::Gold,{5,0},Team::Red},
		{UnitType::Bishop,{7,1},Team::Red},
		{UnitType::Rook,{1,1},Team::Red},
		{UnitType::King1,{4,0},Team::Red}
	};
	return list;
}

// 初期マップデータを返すファクトリ関数
static HashTable<int32, ChipInfo> CreateDefaultChipDB()
{
	return HashTable<int32, ChipInfo>{
		{0, { U"空", ColorF{0,0,1.0}, 99 }},
		{ 1, {U"盤", ColorF{0.9,0.9,0.6,0}, 1} }
	};
}

// 初期ユニットデータベースを返すファクトリ関数
static HashTable<UnitType, UnitInfo> CreateDefaultUnitDB()
{
	return HashTable<UnitType, UnitInfo>{
		{UnitType::Pawn, { U"歩兵",1 }},
		{ UnitType::Lance,{U"香車",1} },
		{ UnitType::Knight,{U"桂馬",1} },
		{ UnitType::Silver,{U"銀将",1} },
		{ UnitType::Gold,{U"金将",1} },
		{ UnitType::Bishop,{U"角行",1} },
		{ UnitType::Rook,{U"飛車",1} },
		{ UnitType::King,{U"王将",1} },
		{ UnitType::King1,{U"玉将",1} },
		{ UnitType::Pawn1,{U"と金",1} },
		{ UnitType::Lance1,{U"成香",1} },
		{ UnitType::Knight1,{U"成桂",1} },
		{ UnitType::Silver1,{U"成銀",1} },
		{ UnitType::Bishop1,{U"龍馬",1} },
		{ UnitType::Rook1,{U"龍王",1} }
	};
}
// 成り駒かどうかを判定する関数
constexpr bool IsPromoted(UnitType t) noexcept
{
	switch (t)
	{
	case UnitType::Pawn1:
	case UnitType::Lance1:
	case UnitType::Knight1:
	case UnitType::Silver1:
	case UnitType::Bishop1:
	case UnitType::Rook1:
		return true;
	default:
		return false;
	}
}
// 成り駒かどうかを判定する関数
constexpr bool IsDemoted(UnitType t) noexcept
{
	switch (t)
	{
	case UnitType::Pawn:
	case UnitType::Lance:
	case UnitType::Knight:
	case UnitType::Silver:
	case UnitType::Bishop:
	case UnitType::Rook:
		return true;
	default:
		return false;
	}
}

enum class GameEvent
{
	PieceMoved,     // 駒が移動した
	PieceDropped,   // 駒を打った
	Promoted,       // 成った
	GameWon,        // 勝利した
	GameStart,      // ゲーム開始
};
// UnitType の列挙値の最大値 + 1
constexpr int UnitTypeArraySize = static_cast<int>(UnitType::King1) + 1;


