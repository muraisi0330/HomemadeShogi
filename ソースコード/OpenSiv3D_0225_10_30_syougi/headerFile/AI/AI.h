#pragma once
#include "AI/AIData.h"
#include "Game/Game.h"
#include "Game/Movement.h"
#include "Game/Drop.h"
#include "Board/Bitboard.h" // 追加
#include "Board/BitboardTables.h"
#include "Board/SlidingAttacks.h"
#include <array>

// 最後に AI が動かした駒の出発地点（以前の実装の a に相当）
struct AIState
{
	Point lastMoveFrom{ -1, -1 };
	int ID = 0;
	std::vector<uint64> gameHistory; // 対局開始時からのハッシュ履歴を保持
};

class IShogiAI {
public:
	virtual ~IShogiAI() = default;

	// AIに手番を回すための純粋仮想関数
	virtual void MakeMove(Game& game, Team aiTeam) = 0;

	// 必要に応じて、名前を取得するなどの共通メソッド
	virtual String getName() const = 0;
	AIState aiState;
private:

};





