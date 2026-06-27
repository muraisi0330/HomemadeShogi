#include "stdafx.h"
#include "Game/Movement.h"
#include "Logic/MoveStrategy.h"
#include "Logic/Strategies.h"
#include "Board/Bitboard.h"
#include <unordered_map>
#include <memory>

namespace Movement
{
	// 対応表（ルックアップテーブル）
	static std::unordered_map<UnitType, std::unique_ptr<IMoveStrategy>> g_strategies;

	// 初期化関数（Game起動時に一度だけ呼ぶ、または初回アクセス時に初期化）
	void InitStrategies()
	{
		if (!g_strategies.empty()) return;

		
		g_strategies[UnitType::King] = std::make_unique<KingStrategy>();
		g_strategies[UnitType::King1] = std::make_unique<KingStrategy>(); // 王も玉も同じ
		g_strategies[UnitType::Pawn] = std::make_unique<PawnStrategy>();
		g_strategies[UnitType::Rook] = std::make_unique<RookStrategy>();
		g_strategies[UnitType::Gold] = std::make_unique<GoldStrategy>();
		g_strategies[UnitType::Bishop] = std::make_unique<BishopStrategy>();
		g_strategies[UnitType::Knight] = std::make_unique<KnightStrategy>();
		g_strategies[UnitType::Lance] = std::make_unique<LanceStrategy>();
		g_strategies[UnitType::Silver] = std::make_unique<SilverStrategy>();
		g_strategies[UnitType::Pawn1] = std::make_unique<GoldStrategy>();
		g_strategies[UnitType::Rook1] = std::make_unique<DragonKingStrategy>();
		g_strategies[UnitType::Bishop1] = std::make_unique<RyomaStrategy>();
		g_strategies[UnitType::Knight1] = std::make_unique<GoldStrategy>();
		g_strategies[UnitType::Lance1] = std::make_unique<GoldStrategy>();
		g_strategies[UnitType::Silver1] = std::make_unique<GoldStrategy>();
	}

	// 戦略を取得するヘルパー
	const IMoveStrategy* GetStrategy(UnitType type)
	{
		if (g_strategies.empty()) InitStrategies();

		auto it = g_strategies.find(type);
		if (it != g_strategies.end()) {
			return it->second.get();
		}
		return nullptr; // 未登録の駒
	}


	bool IsInsideMap(const Grid<int32>& map, int x, int y) noexcept
	{
		if (x < 0) return false;
		if (y < 0) return false;
		if (x >= map.width()) return false;
		if (y >= map.height()) return false;
		return true;
	}

	// 既存のリストベースの通行判定（互換性のため残す）
	bool IsTilePassable(const Array<Unit>& unitList, const Point& selectedPos, int x, int y) noexcept
	{
		for ( auto& u : unitList)
		{
			if (!u.isOnBoard) continue;
			if (u.pos.x != x) continue;
			if (u.pos.y != y) continue;
			if (selectedPos.x == x && selectedPos.y == y) continue;
			return false;
		}
		return true;
	}

	// 新しい Context ベースの通行判定：occBB を使う（高速・一貫）
	bool IsTilePassable(const Context& ctx, const Point& selectedPos, int x, int y) noexcept
	{
		// 範囲外は不可
		if (x < 0 || x >= BoardSize || y < 0 || y >= BoardSize) return false;
		int sq = SQ(x, y);
		// 空なら通行可能
		if (!ctx.occBB.test(sq)) return true;
		// もし選択ユニットの元位置なら通行可能（移動先に自分が見えているケース）
		if (selectedPos.x == x && selectedPos.y == y) return true;
		return false;
	}

	// ビットボードを Context から一度構築するヘルパ
	static void BuildBitboardsIfNeeded(Context& ctx)
	{
		if (ctx.bitboardsBuilt) return;
		ctx.occBB.clear();
		for (auto& b : ctx.pieceBB) b.clear();
		ctx.teamBB[0].clear(); ctx.teamBB[1].clear();

		if (!ctx.unitList) { ctx.bitboardsBuilt = true; return; }
		for (int i = 0; i < static_cast<int>(ctx.unitList->size()); ++i) {
			const Unit& u = (*ctx.unitList)[i];
			if (!u.isOnBoard) continue;
			int sq = SQ(u.pos.x, u.pos.y);
			if (sq < 0 || sq >= BoardSize * BoardSize) continue;
			ctx.occBB.set(sq);
			int tIdx = static_cast<int>(u.type);
			if (tIdx >= 0 && tIdx < static_cast<int>(ctx.pieceBB.size())) ctx.pieceBB[tIdx].set(sq);
			int teamIdx = (u.team == Team::Blue) ? 0 : 1;
			ctx.teamBB[teamIdx].set(sq);
		}
		ctx.bitboardsBuilt = true;
	}

	void ComputeMoveRange(int moveRange[BoardSize][BoardSize]) noexcept
	{
		for (int y = 0; y < BoardSize; ++y)
		{
			for (int x = 0; x < BoardSize; ++x)
			{
				moveRange[y][x] = ResetSelectedIndex;
			}
		}
	}

	void ClearMoveRangeForCurrentTeam(int moveRange[BoardSize][BoardSize], const Array<Unit>& unitList, Team currentTeam) noexcept
	{
		for (auto& u : unitList)
		{
			if (u.team != currentTeam) continue;
			if (!u.isOnBoard) continue;
			moveRange[u.pos.y][u.pos.x] = ResetSelectedIndex;
		}
	}

	int ComputeMoveRangeImpl(Context& ctx, int y, int x, int n, UnitType u, int calcno)
	{
		// 一度だけビットボードを構築（fastBoard があればそれを参照してコピーする）
		BuildBitboardsIfNeeded(ctx);

		// ガード：範囲外
		if (!IsInsideMap(*ctx.map, x, y))
		{
			++calcno;
			return calcno;
		}

		// 既により良い値が入っているなら終わり
		if (ctx.moveRange[y][x] >= n) return calcno;

		// このマスに入るコストを引く
		n -= 1;
		ctx.moveRange[y][x] = n;

		// ガード：別の駒がいて移動できない場合（Context ベースの高速判定を使用）
		if (!IsTilePassable(ctx, ctx.selectedUnitPosition, x, y))
		{
			++calcno;
			return calcno;
		}

		// ガード：残り移動力が負なら終了
		if (n < 0)
		{
			++calcno;
			return calcno;
		}

		const IMoveStrategy* strategy = GetStrategy(u);
		if (strategy)
		{
			// その駒専用の動きを実行せよ
			Bitboard targets = strategy->Execute(ctx, y, x, n, u);
			Bitboard tmp = targets;
			int sq;
			while ((sq = pop_lsb(tmp)) != -1) {
				int nx = sq % BoardSize;
				int ny = sq / BoardSize;
				// 再帰呼び出し
				calcno = ComputeMoveRangeImpl(ctx, ny, nx, n, u, calcno);
			}
		}

		return calcno;
	}

	// --- 新規実装: 外部ビットボードを Context に注入するヘルパ ---
	void AttachBitboardsFromExternal(Context& ctx, const Bitboard& occ, const Bitboard* pieceBBArray, int pieceCount, const Bitboard teamBB[2]) noexcept
	{
		// コピーする前に内部クリア
		ctx.occBB.clear();
		for (auto& b : ctx.pieceBB) b.clear();
		ctx.teamBB[0].clear(); ctx.teamBB[1].clear();

		// occ をコピー
		ctx.occBB = occ;

		// pieceBB 配列をコピー（安全な上限で）
		int copyCount = std::min<int>(static_cast<int>(ctx.pieceBB.size()), std::max(0, pieceCount));
		for (int i = 0; i < copyCount; ++i)
		{
			ctx.pieceBB[i] = pieceBBArray[i];
		}

		// teamBB コピー
		ctx.teamBB[0] = teamBB[0];
		ctx.teamBB[1] = teamBB[1];

		ctx.bitboardsBuilt = true;
	}
	void PopulateAIContext(Context& ctx, const Bitboard& occ, const Bitboard teamBB[2], int direction) {
		ctx.occBB = occ;
		ctx.teamBB[0] = teamBB[0];
		ctx.teamBB[1] = teamBB[1];
		ctx.moveDirection = direction;
		ctx.bitboardsBuilt = true; // 構築済みフラグを立てる
		// AIでは map や chipDB を参照しないよう、Strategy側で null チェックが必要
	}
	Bitboard ComputeMoveBitboard(Context& ctx, int y, int x, int n, UnitType u) {
		// 一度だけビットボードを構築（fastBoard があればそれを参照してコピーする）
		BuildBitboardsIfNeeded(ctx);
		const IMoveStrategy* strategy = GetStrategy(u);
		if (strategy)
		{
			return strategy->Execute(ctx, y, x, n, u);
		}
		return Bitboard(); // 戦略がない場合は空のビットボードを返す
	}
	Bitboard BuildAttackersMask(Team side, int tsq, const Bitboard& occMask, const std::array<Bitboard, UnitTypeArraySize>& pieceBB, const Bitboard teamBB[2], const SlidingTable& sliding)
	{
		Bitboard attackers; attackers.clear();
		const int sideIdx = (side == Team::Blue) ? 0 : 1;
		const int forward = (side == Team::Blue) ? -1 : 1;
		int tx_local = tsq % BoardSize, ty_local = tsq / BoardSize;

		// 歩
		{
			int ay = ty_local - forward;
			if (ay >= 0 && ay < BoardSize) {
				int asq_local = SQ(tx_local, ay);
				if (teamBB[sideIdx].test(asq_local) && pieceBB[static_cast<int>(UnitType::Pawn)].test(asq_local)) attackers.set(asq_local);
			}
		}
		// 桂馬
		{
			int ny = ty_local - 2 * forward;
			if (ny >= 0 && ny < BoardSize) {
				for (int dx : { -1, 1 }) {
					int nx = tx_local + dx;
					if (nx < 0 || nx >= BoardSize) continue;
					int nsq = SQ(nx, ny);
					if (teamBB[sideIdx].test(nsq) &&
						(pieceBB[static_cast<int>(UnitType::Knight)].test(nsq) || pieceBB[static_cast<int>(UnitType::Knight1)].test(nsq)))
						attackers.set(nsq);
				}
			}
		}
		// 近接駒
		for (int dy = -1; dy <= 1; ++dy) for (int dx = -1; dx <= 1; ++dx) {
			if (dx == 0 && dy == 0) continue;
			int nx = tx_local + dx, ny = ty_local + dy;
			if (nx < 0 || nx >= BoardSize || ny < 0 || ny >= BoardSize) continue;
			int nsq = SQ(nx, ny);
			if (!teamBB[sideIdx].test(nsq)) continue;
			if (pieceBB[static_cast<int>(UnitType::King)].test(nsq) ||
				pieceBB[static_cast<int>(UnitType::Gold)].test(nsq) ||
				pieceBB[static_cast<int>(UnitType::Pawn1)].test(nsq) ||
				pieceBB[static_cast<int>(UnitType::Silver1)].test(nsq) ||
				pieceBB[static_cast<int>(UnitType::Knight1)].test(nsq) ||
				pieceBB[static_cast<int>(UnitType::Lance1)].test(nsq)) {
				attackers.set(nsq); continue;
			}
			if (pieceBB[static_cast<int>(UnitType::Silver)].test(nsq)) {
				if (!(dx == 0 && dy == -forward)) attackers.set(nsq);
			}
		}
		// 飛車系 (rook-like)
		{
			Bitboard rookMask = sliding.rook_attacks_from(tsq, occMask);
			Bitboard rookLike = (pieceBB[static_cast<int>(UnitType::Rook)] | pieceBB[static_cast<int>(UnitType::Rook1)]);
			Bitboard rookCandidates = rookMask & teamBB[sideIdx] & rookLike;
			if (rookCandidates.any()) {
				Bitboard tmp = rookCandidates;
				int sq;
				while ((sq = pop_lsb(tmp)) != -1) attackers.set(sq);
			}
			// 香車 (Lance)
			Bitboard lanceCandidates = (sliding.rook_attacks_from(tsq, occMask) & teamBB[sideIdx] & pieceBB[static_cast<int>(UnitType::Lance)]);
			if (lanceCandidates.any()) {
				Bitboard tmp = lanceCandidates;
				int sq;
				while ((sq = pop_lsb(tmp)) != -1) {
					int lx = sq % BoardSize, ly = sq / BoardSize;
					if (side == Team::Blue) {
						if (ly > ty_local) attackers.set(sq);
					}
					else {
						if (ly < ty_local) attackers.set(sq);
					}
				}
			}
			// 龍の付加利き (斜め1)
			for (int dy : { -1, 1 }) for (int dx : { -1, 1 }) {
				int nx = tx_local + dx, ny = ty_local + dy;
				if (nx < 0 || nx >= BoardSize || ny < 0 || ny >= BoardSize) continue;
				int nsq = SQ(nx, ny);
				if (teamBB[sideIdx].test(nsq) && pieceBB[static_cast<int>(UnitType::Rook1)].test(nsq)) attackers.set(nsq);
			}
		}
		// 角系 (bishop-like)
		{
			Bitboard bishopMask = sliding.bishop_attacks_from(tsq, occMask);
			Bitboard bishopLike = (pieceBB[static_cast<int>(UnitType::Bishop)] | pieceBB[static_cast<int>(UnitType::Bishop1)]);
			Bitboard bishopCandidates = bishopMask & teamBB[sideIdx] & bishopLike;
			if (bishopCandidates.any()) {
				Bitboard tmp = bishopCandidates;
				int sq;
				while ((sq = pop_lsb(tmp)) != -1) attackers.set(sq);
			}
			// 馬の orthogonal 1 マス利き
			const std::array<std::pair<int, int>, 4> orth = { std::pair<int,int>{1,0}, {-1,0}, {0,1}, {0,-1} };
			for (const auto& d : orth) {
				int nx = tx_local + d.first, ny = ty_local + d.second;
				if (nx < 0 || nx >= BoardSize || ny < 0 || ny >= BoardSize) continue;
				int nsq = SQ(nx, ny);
				if (teamBB[sideIdx].test(nsq) && pieceBB[static_cast<int>(UnitType::Bishop1)].test(nsq)) attackers.set(nsq);
			}
		}

		return attackers;
	}
} // namespace Movement


