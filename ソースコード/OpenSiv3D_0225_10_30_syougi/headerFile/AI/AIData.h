#pragma once
#include <Siv3D.hpp>
#include "Types/types.h"
#include "Game/Game.h"



struct Candidate {
	int unitIndex = -1;
	Point target{ -1, -1 };
	Point from{ -1, -1 };
	bool isDrop = false;
	bool promoteDecision = false;
	int score = 0;
	bool givesCheck = false; // 追加: 軽量な王手フラグ（GenerateMoves で設定）
};

enum class HashFlag : uint8 {
	None = 0, // 0 を「データなし」にする
	Exact,
	Alpha,
	Beta
};

struct TTEntry {
	uint64 hash = 0;
	int32 score = 0;
	int16 depth = -1;
	HashFlag flag = HashFlag::None;
	uint8 age = 0;
	int32 bestUnitIndex = -1;
	Point bestTarget = { -1, -1 };
	bool bestPromote = false;
};

//駒の評価
struct PIECE_VALUE
{
	int PAWN = 100;// 歩
	int LANCE = 300;// 香車
	int KNIGHT = 300;// 桂馬
	int SILVER = 500;// 銀
	int GOLD = 600;// 金
	int BISHOP = 800;// 角
	int ROOK = 900;// 飛車
	int KING = 10000;// 王
	int PROMOTION_BONUS = 50;// 成りのボーナス
};

//成り駒の評価
struct PromotedPieceValue
{
	int PROMOTED_PAWN = 150;// と金
	int PROMOTED_LANCE = 350;// 成香
	int PROMOTED_KNIGHT = 350;// 成桂
	int PROMOTED_SILVER = 550;// 成銀
	int PROMOTED_BISHOP = 850;// 馬
	int PROMOTED_ROOK = 950;// 龍
};

//駒の合計評価
struct TotalPieceValue
{
	int TOTAL_PAWN = 250;// 歩
	int TOTAL_LANCE = 650;// 香車
	int TOTAL_KNIGHT = 650;// 桂馬
	int TOTAL_SILVER = 1050;// 銀
	int TOTAL_GOLD = 1200;// 金
	int TOTAL_BISHOP = 1650;// 角
	int TOTAL_ROOK = 1950;// 飛車
	int TOTAL_KING = 10000;// 王
	int TOTAL_PROMOTED_PAWN = 150;// と金
	int TOTAL_PROMOTED_LANCE = 350;// 成香
	int TOTAL_PROMOTED_KNIGHT = 350;// 成桂
	int TOTAL_PROMOTED_SILVER = 550;// 成銀
	int TOTAL_PROMOTED_BISHOP = 850;// 馬
	int TOTAL_PROMOTED_ROOK = 950;// 龍
};
static inline int PieceValue(UnitType t, TotalPieceValue TPV) {
	switch (t) {
	case UnitType::Pawn:    return TPV.TOTAL_PAWN;
	case UnitType::Lance:   return TPV.TOTAL_LANCE;
	case UnitType::Knight:  return TPV.TOTAL_KNIGHT;
	case UnitType::Silver:  return TPV.TOTAL_SILVER;
	case UnitType::Gold:    return TPV.TOTAL_GOLD;
	case UnitType::Bishop:  return TPV.TOTAL_BISHOP;
	case UnitType::Rook:    return TPV.TOTAL_ROOK;
	case UnitType::King:    return TPV.TOTAL_KING;
	case UnitType::King1:   return TPV.TOTAL_KING;
	// 成駒
	case UnitType::Pawn1:   return TPV.TOTAL_PROMOTED_PAWN;
	case UnitType::Lance1:  return TPV.TOTAL_PROMOTED_LANCE;
	case UnitType::Knight1: return TPV.TOTAL_PROMOTED_KNIGHT;
	case UnitType::Silver1: return TPV.TOTAL_PROMOTED_SILVER;
	case UnitType::Bishop1: return TPV.TOTAL_PROMOTED_BISHOP;
	case UnitType::Rook1:   return TPV.TOTAL_PROMOTED_ROOK;
	default: return 0;
	}
}

//チームの数
constexpr int TEAM_COUNT = 2;

constexpr int KILLER_TABLE_PLY = 32;// 反復深化の最大深さに合わせる
constexpr int ZOBRIST_HAND_SLOTS = 20;// 持ち駒用ハッシュスロット数





