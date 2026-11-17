#pragma once
#include "Types/types.h"
#include "Game/Game.h"
#include "Board/Bitboard.h"
#include "Board/SlidingAttacks.h"
#include <array>


// Movement モジュール：移動範囲計算ロジックを Game から切り出す。
// - Context に参照を渡し、moveRange 配列を副作用で更新する。 
// - computeMoveRange は再帰的に呼ばれる。

namespace Movement
{
	
	struct Context
	{
		// pointer to the 2D array: int[BoardSize][BoardSize]
		int (*moveRange)[BoardSize] = nullptr;// 2D 配列へのポインタ
		const Grid<int32>* map = nullptr;// マップ参照
		const HashTable<int32, ChipInfo>* chipDB = nullptr;// チップ DB 参照
		const Array<Unit>* unitList = nullptr;// ユニットリスト参照
		Point selectedUnitPosition = Point{ 0, 0 };// 選択ユニット座標
		int moveDirection = 1;// 進行方向（1:青, -1:赤）

		// --- ビットボード一元化用 --- 
		// これらは ComputeMoveRangeImpl の最初で一度だけ構築して再利用する想定
		Bitboard occBB;
		std::array<Bitboard, 30> pieceBB;
		Bitboard teamBB[2];
		bool bitboardsBuilt = false;

		// --- 追加: 外部 FastBoard (ShidenAI::FastBoard 等) のアドレスを渡せる汎用ポインタ
		//     設定されていればビルド処理はそれを参照して内部のビットボードをコピーします。
		const void* fastBoard = nullptr;
	};
	

	// moveRange を -1 で初期化する 
	void ComputeMoveRange(int moveRange[BoardSize][BoardSize]) noexcept;
	// moveRange を現在手の駒だけ残す（元の clearMoveRangeForCurrentTeam 相当）
	void ClearMoveRangeForCurrentTeam(int moveRange[BoardSize][BoardSize], const Array<Unit>& unitList, Team currentTeam) noexcept;
	bool IsInsideMap(const Grid<int32>& map, int x, int y) noexcept;

	// 旧 IsTilePassable と併存。新しいオーバーロードは Context を参照してビットボードを使う。
	bool IsTilePassable(const Array<Unit>& unitList, const Point& selectedPos, int x, int y) noexcept;
	bool IsTilePassable(const Context& ctx, const Point& selectedPos, int x, int y) noexcept;

	// compute move range (戻り値は元実装に合わせて calcno を返します)
	int ComputeMoveRangeImpl(Context& ctx, int y, int x, int n, UnitType u, int calcno = 0);
	void AttachBitboardsFromExternal(Context& ctx, const Bitboard& occ, const Bitboard* pieceBBArray, int pieceCount, const Bitboard teamBB[2]) noexcept;

	// Strategy を取得して実行 const IMoveStrategy* strat = Movement::GetStrategy(u.type);if (strat)
	void PopulateAIContext(Context& ctx, const Bitboard& occ, const Bitboard teamBB[2], int direction);// AI用の Context を構築するヘルパ
//AI 用の戦略インターフェースと実装例
	Bitboard ComputeMoveBitboard(Context& ctx, int y, int x, int n, UnitType u);

	Bitboard BuildAttackersMask(Team side, int tsq, const Bitboard& occMask, const std::array<Bitboard, UnitTypeArraySize>& pieceBB, const Bitboard teamBB[2], const SlidingTable& sliding);

}
