#include "stdafx.h"
#include "AI/SolidDefenseAI.h"
#include "Game/Promotion.h"
#include <vector>
#include <limits>
#include <cmath>

//テスト用AI
	namespace SolidDefenseAIConfig {
		namespace SearchConfig {
			inline  int SEARCH_DEPTH = 2;//AIの探索深さ
			inline  int MOVE_RANDOM_RANGE = 1;//手のスコアに加えるランダム値の範囲
		}
		namespace Evaluation {
			inline  int PROMOTION_BONUS = 5;//成る手に対するボーナス

			inline  int AVOID_BACKTRACK_PENALTY = 500;//直前の位置に戻る手を避けるペナルティ


			inline  int KING_PROXIMITY_BONUS = 10;// 王への接近に対するボーナス
			inline  int DISTANCE_SCALE = 20;

		};
		namespace PieceValue {
			inline  PIECE_VALUE VAL = {
				1,
				3,
				3,
				5,
				6,
				8,
				9,
				1000,
				0
			};
			inline  PromotedPieceValue PROM_VAL = {
				5,
				5,
				5,
				5,
				9,
				10,
			};
			inline  TotalPieceValue TOTAL_VAL{
			 VAL.PAWN,
		     VAL.LANCE,
		     VAL.KNIGHT,
		     VAL.SILVER,
	    	 VAL.GOLD,
	    	 VAL.BISHOP,
		     VAL.ROOK,
		     VAL.KING,
	         PROM_VAL.PROMOTED_PAWN,
		     PROM_VAL.PROMOTED_LANCE,
		     PROM_VAL.PROMOTED_KNIGHT,
		     PROM_VAL.PROMOTED_SILVER,
		     PROM_VAL.PROMOTED_BISHOP,
		     PROM_VAL.PROMOTED_ROOK
			};
		
		};
		namespace PieceRisk {
			inline  int AVOID_MOVING_INTO_DANGER_PENALTY = 30;// 敵の攻撃範囲に入る手を避けるペナルティ
			inline  int CAPTURE_BONUS = 15;//駒を取る手に対するボーナス
			inline  int PIECE_PICKING_BONUS = 5;// 駒取りの期待値を加算
			inline  int CHECK_BONUS = 5;	//王手をかけられているなら加点
			inline  int STABILITY_PENALTY = 2;//自駒が攻撃される数に対するペナルティ
			inline  int KING_IN_DANGER_PENALTY = 500;//王が危険な時の減点
		};

		void Load() {
			const TOMLReader toml{ U"example/ExternalConfigurationFile/SolidDefenseAI.toml" };


			SearchConfig::MOVE_RANDOM_RANGE = toml[U"SearchConfig"][U"MOVE_RANDOM_RANGE"].get<int>();
			SearchConfig::SEARCH_DEPTH = toml[U"SearchConfig"][U"SEARCH_DEPTH"].get<int>();

			PieceRisk::AVOID_MOVING_INTO_DANGER_PENALTY = toml[U"PieceRisk"][U"AVOID_MOVING_INTO_DANGER_PENALTY"].get<int>();
			PieceRisk::CAPTURE_BONUS = toml[U"PieceRisk"][U"CAPTURE_BONUS"].get<int>();
			PieceRisk::PIECE_PICKING_BONUS = toml[U"PieceRisk"][U"PIECE_PICKING_BONUS"].get<int>();
			PieceRisk::CHECK_BONUS = toml[U"PieceRisk"][U"CHECK_BONUS"].get<int>();
			PieceRisk::STABILITY_PENALTY = toml[U"PieceRisk"][U"STABILITY_PENALTY"].get<int>();
			PieceRisk::KING_IN_DANGER_PENALTY = toml[U"PieceRisk"][U"KING_IN_DANGER_PENALTY"].get<int>();

			Evaluation::AVOID_BACKTRACK_PENALTY = toml[U"Evaluation"][U"AVOID_BACKTRACK_PENALTY"].get<int>();
			Evaluation::PROMOTION_BONUS = toml[U"Evaluation"][U"PROMOTION_BONUS"].get<int>();
			Evaluation::KING_PROXIMITY_BONUS = toml[U"Evaluation"][U"KING_PROXIMITY_BONUS"].get<int>();
			Evaluation::DISTANCE_SCALE = toml[U"Evaluation"][U"DISTANCE_SCALE"].get<int>();

			PieceValue::VAL.PAWN = toml[U"PieceValue.PAWN"].get<int>();
			PieceValue::VAL.LANCE = toml[U"PieceValue.LANCE"].get<int>();
			PieceValue::VAL.KNIGHT = toml[U"PieceValue.KNIGHT"].get<int>();
			PieceValue::VAL.SILVER = toml[U"PieceValue.SILVER"].get<int>();
			PieceValue::VAL.GOLD = toml[U"PieceValue.GOLD"].get<int>();
			PieceValue::VAL.BISHOP = toml[U"PieceValue.BISHOP"].get<int>();
			PieceValue::VAL.ROOK = toml[U"PieceValue.ROOK"].get<int>();
			PieceValue::VAL.KING = toml[U"PieceValue.KING"].get<int>();
			PieceValue::VAL.PROMOTION_BONUS = toml[U"PieceValue.PROMOTION_BONUS"].get<int>();

			PieceValue::PROM_VAL.PROMOTED_PAWN = toml[U"PromotedPieceValue.PROMOTED_PAWN"].get<int>();
			PieceValue::PROM_VAL.PROMOTED_LANCE = toml[U"PromotedPieceValue.PROMOTED_LANCE"].get<int>();
			PieceValue::PROM_VAL.PROMOTED_KNIGHT = toml[U"PromotedPieceValue.PROMOTED_KNIGHT"].get<int>();
			PieceValue::PROM_VAL.PROMOTED_SILVER = toml[U"PromotedPieceValue.PROMOTED_SILVER"].get<int>();
			PieceValue::PROM_VAL.PROMOTED_BISHOP = toml[U"PromotedPieceValue.PROMOTED_BISHOP"].get<int>();
			PieceValue::PROM_VAL.PROMOTED_ROOK = toml[U"PromotedPieceValue.PROMOTED_ROOK"].get<int>();


			// --- TOTAL_VAL の更新 ---
			PieceValue::TOTAL_VAL = {
				PieceValue::VAL.PAWN,
				PieceValue::VAL.LANCE,
				PieceValue::VAL.KNIGHT,
				PieceValue::VAL.SILVER,
				PieceValue::VAL.GOLD,
				PieceValue::VAL.BISHOP,
				PieceValue::VAL.ROOK,
				PieceValue::VAL.KING,
				PieceValue::PROM_VAL.PROMOTED_PAWN,
				PieceValue::PROM_VAL.PROMOTED_LANCE,
				PieceValue::PROM_VAL.PROMOTED_KNIGHT,
				PieceValue::PROM_VAL.PROMOTED_SILVER,
				PieceValue::PROM_VAL.PROMOTED_BISHOP,
				PieceValue::PROM_VAL.PROMOTED_ROOK
			};
		}

	}

	SolidDefenseAI::SolidDefenseAI()
	{
		SolidDefenseAIConfig::Load();
	}
	
	// 敵の攻撃可能マップを管理する小さなユーティリティ
	
		void SolidDefenseAI::AttackMap::Reset()
		{
			for (int y = 0; y < BoardSize; ++y)
				for (int x = 0; x < BoardSize; ++x)
					grid[y][x] = false;
		}

		bool SolidDefenseAI::AttackMap::IsCovered(const Point& p) const
		{
			if (p.x < 0 || p.y < 0) return false;
			return grid[p.y][p.x];
		}

		// 指定ユニットの移動範囲を計算して grid を埋める
		void SolidDefenseAI::AttackMap::MarkFromUnit(const Unit& u,
						  const Array<Unit>& units,
						  const Grid<int32>& map,
						  const HashTable<int32, ChipInfo>& chipDB,
						  const HashTable<UnitType, UnitInfo>& unitDB,
						  Team aiTeam)
		{
			if (!u.isOnBoard) return;
			if (u.team == aiTeam) return; // 敵ユニットのみを使って攻撃範囲を作る
			int moveRange[BoardSize][BoardSize];
			Movement::Context ctx;
			ctx.moveRange = moveRange;
			ctx.map = &map;
			ctx.chipDB = &chipDB;
			ctx.unitList = &units;
			ctx.selectedUnitPosition = Point{ u.pos.x, u.pos.y };
			ctx.moveDirection = (aiTeam == Team::Blue) ? -1 : 1;
			Movement::ComputeMoveRange(moveRange);
			const auto& info = unitDB.at(u.type);
			const int cost = chipDB.at(map[u.pos.y][u.pos.x]).cost;
			Movement::ComputeMoveRangeImpl(ctx, u.pos.y, u.pos.x, info.speed + cost, u.type, 0);
			Movement::ClearMoveRangeForCurrentTeam(moveRange, units, u.team);

			for (int y = 0; y < BoardSize; ++y) {
				for (int x = 0; x < BoardSize; ++x) {
					if (moveRange[y][x] < 0) continue;
					grid[y][x] = true;
				}
			}
		}
	





	// 距離評価（マンハッタン距離の負）
	int SolidDefenseAI::DistanceScore(const Point& a, const Point& b)
	{
		return -(std::abs(a.x - b.x) + std::abs(a.y - b.y));
	}

	

	// 敵王の位置を返す（見つからなければ {-1,-1}）
	Point SolidDefenseAI::FindEnemyKing(const Array<Unit>& units, Team aiTeam)
	{
		for (const auto& v : units)
		{
			if (!v.isOnBoard) continue;
			if (v.team == aiTeam) continue;
			if (v.type == UnitType::King
				|| v.type == UnitType::King1)
			{
				return v.pos;
			}
		}
		return Point{ -1, -1 };
	}
	// 自軍王の位置を返す（見つからなければ {-1,-1}）
	Point SolidDefenseAI::FindMyKing(const Array<Unit>& units, Team aiTeam)
	{
		for (const auto& v : units)
		{
			if (!v.isOnBoard) continue;
			if (v.team != aiTeam) continue;
			if (v.type == UnitType::King
				|| v.type == UnitType::King1)
			{
				return v.pos;
			}
		}
		return Point{ -1, -1 };
	}

	// 盤上駒の候補生成（移動・捕獲・成りの簡易判断含む）
	void SolidDefenseAI::GeneratePieceCandidates(std::vector<Candidate>& out,
								 const Array<Unit>& units,
								 const Grid<int32>& map,
								 const HashTable<int32, ChipInfo>& chipDB,
								 const HashTable<UnitType, UnitInfo>& unitDB,
								 Team aiTeam, AttackMap attackMap)
	{
		attackMap.Reset();
		// まず敵の攻撃マップを作る（一度だけ）
		for (const auto& u : units)
		{
			attackMap.MarkFromUnit(u, units, map, chipDB, unitDB, aiTeam);
		}

		for (int i = 0; i < static_cast<int>(units.size()); ++i)
		{
			const Unit& u = units[i];
			if (u.team != aiTeam) continue;
			if (!u.isOnBoard) continue;
			if (u.hasMoved) continue;

			int moveRange[BoardSize][BoardSize];
			Movement::Context ctx;
			ctx.moveRange = moveRange;
			ctx.map = &map;
			ctx.chipDB = &chipDB;
			ctx.unitList = &units;
			ctx.selectedUnitPosition = Point{ u.pos.x, u.pos.y };
			ctx.moveDirection = (aiTeam == Team::Blue) ? 1 : -1;
			Movement::ComputeMoveRange(moveRange);

			const auto& info = unitDB.at(u.type);
			const int cost = chipDB.at(map[u.pos.y][u.pos.x]).cost;
			Movement::ComputeMoveRangeImpl(ctx, u.pos.y, u.pos.x, info.speed + cost, u.type, 0);
			Movement::ClearMoveRangeForCurrentTeam(moveRange, units, u.team);

			for (int y = 0; y < BoardSize; ++y)
			{
				for (int x = 0; x < BoardSize; ++x)
				{
					if (moveRange[y][x] < 0) continue;
					Candidate c;
					c.unitIndex = i;
					c.target = Point{ x, y };
					c.isDrop = false;
					c.from = Point{ u.pos.x, u.pos.y };

					// 成り判定（簡易）
					if (IsAutoPromotionRequired(u.type, u.team, y))
					{
						c.promoteDecision = true;
					}
					else
					{
						UnitType promoted = ToPromoted(u.type);
						c.promoteDecision = (promoted != u.type) && (PieceValue(promoted, SolidDefenseAIConfig::PieceValue::TOTAL_VAL) > PieceValue(u.type, SolidDefenseAIConfig::PieceValue::TOTAL_VAL));
					}

					out.push_back(c);
				}
			}
		}
	}

	// 手駒（ドロップ）候補生成
	void SolidDefenseAI::GenerateDropCandidates(std::vector<Candidate>& out,
								const Array<Unit>& units,
								Team aiTeam)
	{
		for (int i = 0; i < static_cast<int>(units.size()); ++i)
		{
			const Unit& u = units[i];
			if (u.team != aiTeam) continue;
			if (u.isOnBoard) continue; // 手駒のみ
			int moveRange[BoardSize][BoardSize];
			Drop::ComputeDropMoveRange(moveRange, units, u.type, aiTeam);
			for (int y = 0; y < BoardSize; ++y)
			{
				for (int x = 0; x < BoardSize; ++x)
				{
					if (moveRange[y][x] < 0) continue;
					Candidate c;
					c.unitIndex = i;
					c.target = Point{ x, y };
					c.isDrop = true;
					c.promoteDecision = false; // ドロップは成らない
					out.push_back(c);
				}
			}
		}
	}



	// 単純な盤面評価（局所用途）
	int SolidDefenseAI::EvaluateBoard(const Array<Unit>& units, Team aiTeam)
	{
		int score = 0;
		for (const auto& u : units)
		{
			if (!u.isOnBoard) continue;
			score += (u.team == aiTeam ? +1 : -1) * PieceValue(u.type, SolidDefenseAIConfig::PieceValue::TOTAL_VAL);
		}
		return score;
	}

	// 候補をスコアリングする（捕獲・成り・王への接近・危険度など）
	int SolidDefenseAI::ScoreCandidate(const Candidate& cand,
					   const Array<Unit>& units,
					   Team aiTeam,
					   const Point& enemyKing)
	{
		int score = Random(-SolidDefenseAIConfig::SearchConfig::MOVE_RANDOM_RANGE, SolidDefenseAIConfig::SearchConfig::MOVE_RANDOM_RANGE);

		Point myKing = FindMyKing(units, aiTeam);

		// 捕獲評価
		for (const auto& v : units)
		{
			if (!v.isOnBoard) continue;
			if (v.pos.x != cand.target.x) continue;
			if (v.pos.y != cand.target.y) continue;
			if (v.team == aiTeam) continue;
			score += (PieceValue(v.type, SolidDefenseAIConfig::PieceValue::TOTAL_VAL) * SolidDefenseAIConfig::PieceRisk::CAPTURE_BONUS);// 駒を取る手にボーナス
			score += DistanceScore(v.pos, myKing) * SolidDefenseAIConfig::Evaluation::KING_PROXIMITY_BONUS;// 王手をかけられているなら加点
		}
		// 前回移動元に戻す手を嫌う（反復移動抑制）
		if (g_lastMoveFrom == cand.target &&
			g_ID == cand.unitIndex)
		{
			score -= SolidDefenseAIConfig::Evaluation::AVOID_BACKTRACK_PENALTY;
		}

		// 成り評価
		if (cand.promoteDecision && !cand.isDrop)
		{
			score += SolidDefenseAIConfig::Evaluation::PROMOTION_BONUS;
		}

		// 自駒が攻撃される数に対するペナルティ
		if (g_enemyAttackMap.IsCovered(myKing))
		{
			score -= SolidDefenseAIConfig::PieceRisk::KING_IN_DANGER_PENALTY;
		}
		// 自分の王に近づけられているか（敵ユニットからの距離の脅威）
		if (myKing.x >= 0)
		{
			for (const auto& v : units)
			{
				if (v.team != aiTeam && v.isOnBoard)
					score -= DistanceScore(v.pos, myKing) * SolidDefenseAIConfig::Evaluation::KING_PROXIMITY_BONUS;

			}
		}
		// 敵王への接近評価
		score += std::max(0, SolidDefenseAIConfig::Evaluation::DISTANCE_SCALE + DistanceScore(cand.target, enemyKing)) * SolidDefenseAIConfig::Evaluation::KING_PROXIMITY_BONUS;
		return score;
	}

	// 成りの実行（Game API を利用）
	void SolidDefenseAI::CarryOutPromotionIfNeeded(Game& game, const Candidate& cand)
	{
		if (game.GetPendingPromotionIndex() < 0) return;
		if (!cand.promoteDecision) { game.CancelPromotionByIndex(cand.unitIndex); return; }
		game.ApplyPromotionByIndex(cand.unitIndex);
	}



	// 指定チーム（aiTeam）の駒が相手の攻撃範囲に入っている数を数える
	int SolidDefenseAI::CountOwnPiecesUnderAttack(const Array<Unit>& units,
								  Team aiTeam,
								  const Grid<int32>& map,
								  const HashTable<int32, ChipInfo>& chipDB,
								  const HashTable<UnitType, UnitInfo>& unitDB)
	{
		AttackMap localMap;
		// 敵ユニット（aiTeam と異なるチーム）の攻撃範囲をマーク
		for (const auto& u : units)
		{
			if (!u.isOnBoard) continue;
			if (u.team == aiTeam) continue; // 敵のみ
			localMap.MarkFromUnit(u, units, map, chipDB, unitDB, aiTeam);
		}

		// 自駒がその攻撃範囲に入っているか数える
		int count = 0;
		for (const auto& u : units)
		{
			if (!u.isOnBoard) continue;
			if (u.team != aiTeam) continue; // 自駒のみ
			if (!localMap.IsCovered(u.pos)) continue;
			count += PieceValue(u.type, SolidDefenseAIConfig::PieceValue::TOTAL_VAL);
		}
		return count;
	}
	// 最良候補を実行する（Game API を使う）
	void SolidDefenseAI::ExecuteCandidate(Game& game, const Candidate& cand)
	{
		if (cand.unitIndex < 0) return;
		game.SelectUnitByIndex(cand.unitIndex);
		game.ExecuteMoveOrCaptureSelected(cand.target.x, cand.target.y);
		CarryOutPromotionIfNeeded(game, cand);
		game.TurnEndDetermination();
	}

	// 与えた局面で自手を適用した後、相手の最良応手を評価して返す
	int SolidDefenseAI::EvaluateAfterMove(Game gameCopy, Candidate myMove, const Array<Unit>& originalUnits,
						  const HashTable<int32, ChipInfo>& chipDB,
						  const Grid<int32>& map,
						  const HashTable<UnitType, UnitInfo>& unitDB,
						  Team aiTeam, const Point& enemyKing)
	{
		if (myMove.score < 0)return myMove.score;
		int Score = 0;
		// gameCopy に自手を適用（ローカルコピーで安全にシミュレーション）
		ExecuteCandidate(gameCopy, myMove);
		Team enemy = (aiTeam == Team::Blue ? Team::Red : Team::Blue);// 敵チーム
		std::vector<Candidate> Moves;
		const auto& gameUnits = gameCopy.GetUnitList();
		AttackMap attackMap;
		attackMap.Reset();
		GeneratePieceCandidates(Moves, gameUnits, gameCopy.GetMap(), gameCopy.GetChipDB(), gameCopy.GetUnitDB(), aiTeam, attackMap);
		Score += (int)Moves.size() * SolidDefenseAIConfig::PieceRisk::PIECE_PICKING_BONUS;
		// シミュレーション後の盤面で相手候補を生成
		std::vector<Candidate> enemyMoves;
		GeneratePieceCandidates(enemyMoves, gameUnits, gameCopy.GetMap(), gameCopy.GetChipDB(), gameCopy.GetUnitDB(), enemy, attackMap);
		// ドロップも考慮
		GenerateDropCandidates(enemyMoves, gameUnits, enemy);

		int bestEnemyScore = std::numeric_limits<int>::min();// 敵の最良応手スコア
		AttackMap OwnAircraftMap;// 自軍攻撃マップ
		OwnAircraftMap.Reset();
		// 自軍攻撃マップを作成
		for (const auto& e : gameUnits)
		{
			if (e.type == UnitType::King)continue;
			if (e.type == UnitType::King1) continue;
			OwnAircraftMap.MarkFromUnit(e, gameUnits, gameCopy.GetMap(), gameCopy.GetChipDB(), gameCopy.GetUnitDB(), enemy);
		}
		// 王手をかけられているなら加点
		if (OwnAircraftMap.IsCovered(enemyKing)) { Score += SolidDefenseAIConfig::PieceRisk::CHECK_BONUS; }
		// 敵から見た「相手（＝AI）の王」の位置（評価に使用）
		Point myKingAfter = FindEnemyKing(gameUnits, enemy); // enemy の視点から見た「相手の王」
		for (const auto& e : enemyMoves)
		{
			int s = ScoreCandidate(e, gameUnits, enemy, myKingAfter);
			if (s > bestEnemyScore) bestEnemyScore = s;
		}

		// --- 安定性評価（自駒が相手の攻撃にさらされる数の増減） ---
		const int beforeAttacked = CountOwnPiecesUnderAttack(originalUnits, aiTeam, map, chipDB, unitDB);
		const int afterAttacked = CountOwnPiecesUnderAttack(gameUnits, aiTeam, gameCopy.GetMap(), gameCopy.GetChipDB(), gameCopy.GetUnitDB());

		const int attackedDelta = afterAttacked - beforeAttacked;
		// ペナルティ重みは経験則で調整可能。ここでは 20 を使用（1 駒増えるごとに -20）。
		const int stabilityPenaltyPerPiece = SolidDefenseAIConfig::PieceRisk::CAPTURE_BONUS;
		// --- ここまで ---
		Score -= attackedDelta * stabilityPenaltyPerPiece;
		// 移動する駒が敵の攻撃範囲に入る場合は減点、守られているなら少し加点
		if (attackMap.IsCovered(myMove.target))
		{
			if (myMove.unitIndex >= 0)
				Score -= PieceValue(originalUnits[myMove.unitIndex].type, SolidDefenseAIConfig::PieceValue::TOTAL_VAL) * SolidDefenseAIConfig::PieceRisk::AVOID_MOVING_INTO_DANGER_PENALTY;
		}

		// 自手の期待値から敵の最良応手を差し引く（単純な Minimax 風の調整）
		return myMove.score - (bestEnemyScore / 2) + Score;
	}

	Candidate SolidDefenseAI::IterativeDeepening(Game& game, Team aiTeam) {
		const auto& units = game.GetUnitList();
		const auto& map = game.GetMap();
		const auto& chipDB = game.GetChipDB();
		const auto& unitDB = game.GetUnitDB(); // コピーして constness 問題を回避

		// 敵攻撃マップを初期化
		g_enemyAttackMap.Reset();

		std::vector<Candidate> candidates;
		candidates.reserve(256);

		// 古い出力を消去
		ClearPrint();

		// 候補生成
		GeneratePieceCandidates(candidates, units, map, chipDB, unitDB, aiTeam, g_enemyAttackMap);
		GenerateDropCandidates(candidates, units, aiTeam);

		if (candidates.empty()) return Candidate{};

		// 敵王位置（現在盤面の情報）
		Point enemyKing = FindEnemyKing(units, aiTeam);

		// スコアリングと最良選択
		Candidate best;
		best.score = std::numeric_limits<int>::min();
		/*	const int searchDepth = Config::AI::SEARCH_DEPTH;
			best = IterativeDeepeningSearch(game, aiTeam, searchDepth);*/

		for (auto& c : candidates)
		{
			// まず局所的なスコア（捕獲・成り・危険度など）
			c.score = ScoreCandidate(c, units, aiTeam, enemyKing);
			// 相手の最善応手を考慮した評価（シミュレーション）
			c.score = EvaluateAfterMove(game, c, units, chipDB, map, unitDB, aiTeam, enemyKing);
			if (c.score > best.score)
			{
				best = c;
			}
		}
		g_lastMoveFrom = units[best.unitIndex].pos;
		g_ID = best.unitIndex;
		return best;
	}
	void SolidDefenseAI::StartThinking(Game& game, Team aiTeam) {
		if (IsThinking()) return;
		Game& g = game;
		// 非同期で実行開始
		aiTask = Async([=, &g]() {
			return IterativeDeepening(g, aiTeam);
		});
	}

// Public API
void SolidDefenseAI::MakeMove(Game& game, Team aiTeam)
{
	if (!game.IsGameActive()) return;
	if (aiTask.isReady()) {
		Candidate best = aiTask.get();
		if (best.unitIndex >= 0) {
			ExecuteCandidate(game, best);
		}
	}
	else
	{
		StartThinking(game, aiTeam);
	}
}
