#pragma once
#include "Game/Movement.h" // Contextを使うため
#include "Board/Bitboard.h"
#include <Siv3D.hpp>

// すべての移動戦略の親となるインターフェース
class IMoveStrategy
{
public:
	virtual ~IMoveStrategy() = default;// 仮想デストラクタ
	virtual Bitboard Execute(const Movement::Context& ctx, int y, int x, int n, UnitType u) const = 0;// 純粋仮想関数: すべての戦略はこれを実装する必要がある

};

class BaseMoveStrategy : public IMoveStrategy
{
protected:
	

	// 1マス移動（歩、金、王、桂馬など用）: 再帰は行わず到達マスビットを返す
	Bitboard TryStep(const Movement::Context& ctx, int y, int x, int n, const Array<Point>& offsets, UnitType /*u*/, int stepMult = 1) const
	{
		Bitboard out;
		out.clear();
		if (!ctx.map) return out;
		for (const auto& d : offsets)
		{
			int ny = y + d.y * stepMult;
			int nx = x + d.x * stepMult;

			// 範囲外は無視
			if (!Movement::IsInsideMap(*ctx.map, nx, ny)) continue;

			// そのマスは到達可能（空地 or 敵がいる等は到達として含める）
			out.set(SQ(nx, ny));
		}
		return out;
	}

	// スライド移動（香車、飛車、角行用）: レイを飛ばして hit するまでビットを立てる。占有で止める（占有マスは含める）
	Bitboard TrySlide(const Movement::Context& ctx, int y, int x, int n,
		const Array<Point>& offsets, UnitType /*u*/) const
	{
		Bitboard out;
		out.clear();
		if (!ctx.map) return out;

		for (const auto& d : offsets)
		{
			for (int step = 1; step < BoardSize; ++step)
			{
				int ny = y + d.y * step;
				int nx = x + d.x * step;
				if (!Movement::IsInsideMap(*ctx.map, nx, ny))
				{
					break;
				}

				out.set(SQ(nx, ny));

				// ここは Context ベースの高速な通行判定を使う（占有ならそこで停止）
				if (!Movement::IsTilePassable(ctx, ctx.selectedUnitPosition, nx, ny))
				{
					break;
				}
			}
		}
		return out;
	}
};
