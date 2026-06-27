#pragma once
#include "Logic/MoveStrategy.h"

// 各 Execute は Movement::Context を読み取り（ビットボード等を参照）
// そして「この位置から次に到達可能なマスの Bitboard」を返す実装に変更しました。

// 歩の戦略
class PawnStrategy : public BaseMoveStrategy
{
public:
	Bitboard Execute(const Movement::Context& ctx, int y, int x, int n, UnitType u) const override
	{
		// 歩は「前方に1つ」進む
		return TryStep(ctx, y, x, n, Array<Point>{ Point{0, -1 * ctx.moveDirection} }, u);
	}
};

class KnightStrategy : public BaseMoveStrategy
{
public:
	Bitboard Execute(const Movement::Context& ctx, int y, int x, int n, UnitType u) const override
	{
		int dy = -2 * ctx.moveDirection;
		return TryStep(ctx, y, x, n, Array<Point>{ {1,dy}, {-1,dy} }, u);
	}
};

class LanceStrategy : public BaseMoveStrategy
{
public:
	Bitboard Execute(const Movement::Context& ctx, int y, int x, int n, UnitType u) const override
	{
		return TrySlide(ctx, y, x, n, Array<Point>{ {0, -1 * ctx.moveDirection} }, u);
	}
};

// 王の戦略
class KingStrategy : public BaseMoveStrategy
{
public:
	Bitboard Execute(const Movement::Context& ctx, int y, int x, int n, UnitType u) const override
	{
		return TryStep(ctx, y, x, n, Array<Point>{
			{1,-1}, {1,0}, {1,1},
			{0,-1},        {0,1},
			{-1,-1},{-1,0},{-1,1}
		}, u);
	}
};

// 飛車の戦略
class RookStrategy : public BaseMoveStrategy
{
public:
	Bitboard Execute(const Movement::Context& ctx, int y, int x, int n, UnitType u) const override
	{
		return TrySlide(ctx, y, x, n, Array<Point>{ {0,-1},{0,1},{-1,0},{1,0} }, u);
	}
};

class BishopStrategy : public BaseMoveStrategy
{
public:
	Bitboard Execute(const Movement::Context& ctx, int y, int x, int n, UnitType u) const override
	{
		return TrySlide(ctx, y, x, n, Array<Point>{ {1,-1},{1,1},{-1,1},{-1,-1} }, u);
	}
};

// 金の戦略（成駒などもこれを使い回せる）
class GoldStrategy : public BaseMoveStrategy
{
public:
	Bitboard Execute(const Movement::Context& ctx, int y, int x, int n, UnitType u) const override
	{
		int dy = -1 * ctx.moveDirection;
		return TryStep(ctx, y, x, n, Array<Point>{
			{0, dy}, {-1, dy}, {1, dy}, // 前
			{1, 0}, {-1, 0},            // 左右
			{0, -dy}                    // 真後ろ
		}, u);
	}
};

// 銀の戦略
class SilverStrategy : public BaseMoveStrategy
{
public:
	Bitboard Execute(const Movement::Context& ctx, int y, int x, int n, UnitType u) const override
	{
		int dy = -1 * ctx.moveDirection;
		return TryStep(ctx, y, x, n, Array<Point>{
			{0, dy}, {-1, dy}, {1, dy}, // 前
			{1, -dy}, {-1, -dy},        // 真後ろ
		}, u);
	}
};


class DragonKingStrategy : public BaseMoveStrategy
{
public:
	Bitboard Execute(const Movement::Context& ctx, int y, int x, int n, UnitType u) const override
	{
		Bitboard a = TrySlide(ctx, y, x, n, Array<Point>{ {0,-1},{0,1},{-1,0},{1,0} }, u);
		Bitboard b = TryStep(ctx, y, x, n, Array<Point>{ {1,-1},{1,1},{-1,1},{-1,-1} }, u);
		return a | b;
	}
};

class RyomaStrategy : public BaseMoveStrategy
{
public:
	Bitboard Execute(const Movement::Context& ctx, int y, int x, int n, UnitType u) const override
	{
		Bitboard a = TrySlide(ctx, y, x, n, Array<Point>{ {1,-1},{1,1},{-1,1},{-1,-1} }, u);
		Bitboard b = TryStep(ctx, y, x, n, Array<Point>{ {0,-1},{0,1},{-1,0},{1,0} }, u);
		return a | b;
	}
};




