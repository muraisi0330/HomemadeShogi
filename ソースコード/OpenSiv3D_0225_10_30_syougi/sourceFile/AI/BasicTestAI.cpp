#include "stdafx.h"
#include "AI/BasicTestAI.h"
#include "Game/Promotion.h"
#include <vector>
#include <limits>
#include <cmath>
#include <algorithm>
#include <chrono> // 時間計測用

	// ---------------------------------------------------------
	// コンフィグ & 定数
	// ---------------------------------------------------------
	namespace BasicTestAIConfig {
		namespace SearchConfig {
			inline int TIME_LIMIT_MS = 1500;
			inline int MAX_DEPTH = 10;                // 最大探索深さ（時間はTIME_LIMITで管理）
			
		
			inline int AVOID_MOVING_INTO_DANGER_PENALTY = 5;
			
		
		
		}
		namespace System {
			inline int AVOID_BACKTRACK_PENALTY = 5000; // 千日手防止
			inline int CHECKMATE_THRESHOLD = 10000;// 詰みと評価関数の閾値
			// 初期値は非常に低くしておく
			inline int INITIAL_CAPTURE_BONUS = 999999;


			// 負け
			inline int LOSS_PENALTY = -100000;
		}
		namespace Evaluation {
			inline int KING_PROXIMITY_BONUS = 10;// 王への接近に対するボーナス
			inline int PROMOTION_BONUS = 150;         // 成りボーナス
			//10は調整用定数
			inline int CAPTURE_BONUS_SCALE = 10;
			// 端にいるほど高評価
			inline int EDGE_PROXIMITY_PENALTY = 150;
			// 中央は危険
			inline int CENTER_PROXIMITY_PENALTY = 100;
			// 中央にいる金銀は少し高評価
			inline int CENTER_GOLD_SILVER_BONUS = 30;
			// 前進も評価
			inline int ADVANCEMENT_BONUS = 5;
			// 3段目/5段目にいる歩は高評価
			inline int ADVANCED_PAWN_BONUS = 80;
			// 味方の王の近くにいる金銀を評価調整用定数
			inline int GOLD_SILVER_KING_PROXIMITY_BONUS = 80;

			inline int DISTANCE_SCALE = 20;
		}
		namespace PieceValue {
			inline PIECE_VALUE VAL = {
				1,
				3,
				3,
				5,
				6,
				8,
				9,
				10000,
				0
			};
			inline PromotedPieceValue PROM_VAL = {
				5,
				5,
				5,
				5,
				9,
				10,
			};
			inline TotalPieceValue TOTAL_VAL{
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
			inline int MOBILITY_BONUS = 10;           // 利きボーナス
			inline int KING_IN_DANGER_PENALTY = 3000; // 王の危険度


			inline int CHECK_BONUS = 1000;             // 王手ボーナス
		}
		namespace HandPiece {
			// 持ち駒は盤上の駒より価値を下げる（調整用定数）
			inline int HAND_PIECE_PENALTY = 5;
		}

		void Load() {

			const TOMLReader toml{ U"example/ExternalConfigurationFile/BasicTestAI.toml" };

			if (!toml.isOpen()) {
				Logger << U"TOML読み込み失敗！";
				return;
			}

			SearchConfig::TIME_LIMIT_MS = toml[U"SearchConfig.TimeLimitMs"].get<int>();
			SearchConfig::MAX_DEPTH = toml[U"SearchConfig.MaxDepth"].get<int>();
			SearchConfig::AVOID_MOVING_INTO_DANGER_PENALTY = toml[U"SearchConfig.AvoidMovingIntoDangerPenalty"].get<int>();

			System::AVOID_BACKTRACK_PENALTY = toml[U"System.AvoidBacktrackPenalty"].get<int>();
			System::CHECKMATE_THRESHOLD = toml[U"System.CheckmateThreshold"].get<int>();
			System::INITIAL_CAPTURE_BONUS = toml[U"System.InitialCaptureBonus"].get<int>();
			System::LOSS_PENALTY = toml[U"System.LossPenalty"].get<int>();

			PieceRisk::MOBILITY_BONUS = toml[U"PieceRisk.MobilityBonus"].get<int>();
			PieceRisk::KING_IN_DANGER_PENALTY = toml[U"PieceRisk.KingInDangerPenalty"].get<int>();
			PieceRisk::CHECK_BONUS = toml[U"PieceRisk.CheckBonus"].get<int>();

			HandPiece::HAND_PIECE_PENALTY = toml[U"HandPiece.HandPiecePenalty"].get<int>();

			Evaluation::KING_PROXIMITY_BONUS = toml[U"Evaluation.KingProximityBonus"].get<int>();
			Evaluation::PROMOTION_BONUS = toml[U"Evaluation.PromotionBonus"].get<int>();
			Evaluation::CAPTURE_BONUS_SCALE = toml[U"Evaluation.CaptureBonusScale"].get<int>();
			Evaluation::EDGE_PROXIMITY_PENALTY = toml[U"Evaluation.EdgeProximityPenalty"].get<int>();
			Evaluation::CENTER_PROXIMITY_PENALTY = toml[U"Evaluation.CenterProximityPenalty"].get<int>();
			Evaluation::CENTER_GOLD_SILVER_BONUS = toml[U"Evaluation.CenterGoldSilverBonus"].get<int>();
			Evaluation::ADVANCEMENT_BONUS = toml[U"Evaluation.AdvancementBonus"].get<int>();
			Evaluation::ADVANCED_PAWN_BONUS = toml[U"Evaluation.AdvancedPawnBonus"].get<int>();

			PieceValue::VAL.PAWN = toml[U"PieceValue.Pawn"].get<int>();
			PieceValue::VAL.LANCE = toml[U"PieceValue.Lance"].get<int>();
			PieceValue::VAL.KNIGHT = toml[U"PieceValue.Knight"].get<int>();
			PieceValue::VAL.SILVER = toml[U"PieceValue.Silver"].get<int>();
			PieceValue::VAL.GOLD = toml[U"PieceValue.Gold"].get<int>();
			PieceValue::VAL.BISHOP = toml[U"PieceValue.Bishop"].get<int>();
			PieceValue::VAL.ROOK = toml[U"PieceValue.Rook"].get<int>();
			PieceValue::VAL.KING = toml[U"PieceValue.King"].get<int>();

			PieceValue::PROM_VAL.PROMOTED_PAWN = toml[U"PieceValue.PromotedPawn"].get<int>();
			PieceValue::PROM_VAL.PROMOTED_LANCE = toml[U"PieceValue.PromotedLance"].get<int>();
			PieceValue::PROM_VAL.PROMOTED_KNIGHT = toml[U"PieceValue.PromotedKnight"].get<int>();
			PieceValue::PROM_VAL.PROMOTED_SILVER = toml[U"PieceValue.PromotedSilver"].get<int>();
			PieceValue::PROM_VAL.PROMOTED_BISHOP = toml[U"PieceValue.PromotedBishop"].get<int>();
			PieceValue::PROM_VAL.PROMOTED_ROOK = toml[U"PieceValue.PromotedRook"].get<int>();

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

	BasicTestAI::BasicTestAI() {
		BasicTestAIConfig::Load();
	}

	// ---------------------------------------------------------
	// 軽量盤面構造体 (FastBoard)
	// Gameクラスを丸ごとコピーする代わりに、これを使用する
	// ---------------------------------------------------------
	

		// Gameから初期化するコンストラクタ
		BasicTestAI::FastBoard::FastBoard(const Game& game)
			: units(game.GetUnitList())
			, currentTeam(game.GetCurrentTeam())
			, mapRef(&game.GetMap())
			, chipDBRef(&game.GetChipDB())
			, unitDBRef(&game.GetUnitDB())
		{

		}
	// 敵の攻撃可能マップを管理する小さなユーティリティ
		void BasicTestAI::AttackMap::Reset()
		{
			for (int y = 0; y < BoardSize; ++y)
				for (int x = 0; x < BoardSize; ++x)
					grid[y][x] = false;
		}

		bool BasicTestAI::AttackMap::IsCovered(const Point& p) const
		{
			if (p.x < 0 || p.y < 0) return false;
			return grid[p.y][p.x];
		}

		int BasicTestAI::AttackMap::CountCovered() const {
			int count = 0;
			for (int y = 0; y < BoardSize; ++y)
				for (int x = 0; x < BoardSize; ++x)
					if (grid[y][x]) count++;
			return count;
		}

		void BasicTestAI::AttackMap::MarkFromUnit(const Unit& u,
			FastBoard fastBoard,
			Team viewTeam)
		{
			if (!u.isOnBoard) return;

			// 高速化: 毎回MoveRangeを確保せず、ローカル変数で済ませる工夫が必要だが、
			// Movementの仕様上、配列を用意する
			int moveRange[BoardSize][BoardSize];
			Movement::Context ctx;
			ctx.moveRange = moveRange;
			ctx.map = fastBoard.mapRef;
			ctx.chipDB = fastBoard.chipDBRef;
			ctx.unitList = &fastBoard.GetUnitList();
			ctx.selectedUnitPosition = Point{ u.pos.x, u.pos.y };
			ctx.moveDirection = (viewTeam == Team::Blue) ? 1 : -1;

			Movement::ComputeMoveRange(moveRange);


			Movement::ComputeMoveRangeImpl(ctx, u.pos.y, u.pos.x, 1, u.type, 0);
			// Movement::ClearMoveRangeForCurrentTeam は利き計算には不要（味方位置も利きに含めるため）

			for (int y = 0; y < BoardSize; ++y) {
				for (int x = 0; x < BoardSize; ++x) {
					if (moveRange[y][x] < 0) continue;
					grid[y][x] = true;
				}
			}
		}
	// ---------------------------------------------------------
	// 内部ロジック実装 (Local Execute)
	// Gameクラスのメソッドを使わず、FastBoard上で手を実行する
	// ---------------------------------------------------------
		void BasicTestAI::ExecuteMoveLocal(FastBoard& board, const Candidate& cand)
		{
			Unit& actor = board.units[cand.unitIndex];

			if (cand.isDrop) {
				// 打ち
				actor.isOnBoard = true;
				actor.pos = cand.target;
				actor.hasMoved = true;
				actor.promoteFlag = false;
				// 手駒インデックスの整理はAI探索上は厳密でなくて良い（isOnBoard判定が主）が、
				// 整合性のため簡易的に扱う
			}
			else {
				// 移動
				// 捕獲判定
				for (auto& target : board.units) {
					if (target.isOnBoard && target.pos == cand.target && target.team != board.currentTeam) {
						// 捕獲発生：相手の駒を自分の手駒にする
						target.isOnBoard = false;
						target.team = board.currentTeam;
						target.type = ToDemoted(target.type); // 成りを解除
						target.hasMoved = false;
						break;
					}
				}

				// 位置更新
				actor.pos = cand.target;
				actor.hasMoved = true;

				// 成り適用
				if (cand.promoteDecision) {
					actor.type = ToPromoted(actor.type);
				}
			}

			// 手番交代
			board.currentTeam = (board.currentTeam == Team::Blue) ? Team::Red : Team::Blue;
		}

	// ---------------------------------------------------------
	// 候補手生成 (Movementモジュール利用)
	// ---------------------------------------------------------
	void BasicTestAI::GenerateMoves(std::vector<Candidate>& out, const FastBoard& board, bool capturesOnly)
	{
		const auto& units = board.units;

		// Movement::ComputeMoveRange 等は Context を要求する
		// FastBoardのデータを使ってContextを作る
		int moveRange[BoardSize][BoardSize];
		Movement::Context ctx;
		ctx.moveRange = moveRange;
		ctx.map = board.mapRef;
		ctx.chipDB = board.chipDBRef;
		ctx.unitList = &board.units;

		for (int i = 0; i < static_cast<int>(units.size()); ++i)
		{
			const Unit& u = units[i];
			if (u.team != board.currentTeam) continue;

			// 盤上の駒
			if (u.isOnBoard) {
				if (u.hasMoved) continue; // (AI探索上は hasMoved=false の駒しか動かせない前提なら) ※通常将棋ならhasMovedは関係ないが、このゲーム仕様に合わせる

				ctx.selectedUnitPosition = u.pos;
				ctx.moveDirection = (board.currentTeam == Team::Blue) ? 1 : -1;
				Movement::ComputeMoveRange(moveRange); // clear

				const auto& info = board.unitDBRef->at(u.type);
				const int cost = board.chipDBRef->at(board.mapRef->at(u.pos.y, u.pos.x)).cost;

				// 移動範囲計算
				Movement::ComputeMoveRangeImpl(ctx, u.pos.y, u.pos.x, info.speed + cost, u.type, 0);
				Movement::ClearMoveRangeForCurrentTeam(moveRange, units, u.team);

				for (int y = 0; y < BoardSize; ++y) {
					for (int x = 0; x < BoardSize; ++x) {
						if (moveRange[y][x] < 0) continue;

						// 捕獲のみモードなら、敵がいない場所はスキップ
						bool isCapture = false;
						int victimVal = 0;
						for (const auto& t : units) {
							if (t.isOnBoard && t.pos == Point{ x, y } && t.team != board.currentTeam) {
								isCapture = true;
								victimVal = PieceValue(t.type, BasicTestAIConfig::PieceValue::TOTAL_VAL);
								break;
							}
						}

						if (capturesOnly && !isCapture) continue;

						Candidate c;
						c.unitIndex = i;
						c.target = Point{ x, y };
						c.from = u.pos;
						c.isDrop = false;
						c.promoteDecision = IsAutoPromotionRequired(u.type, u.team, y) ||
							(ToPromoted(u.type) != u.type && PieceValue(ToPromoted(u.type), BasicTestAIConfig::PieceValue::TOTAL_VAL) > PieceValue(u.type, BasicTestAIConfig::PieceValue::TOTAL_VAL));

						// Move Ordering用スコア
						if (isCapture) c.score += victimVal * 10 - PieceValue(u.type, BasicTestAIConfig::PieceValue::TOTAL_VAL);
						if (c.promoteDecision) c.score += BasicTestAIConfig::Evaluation::PROMOTION_BONUS;

						out.push_back(c);
					}
				}
			}
			// 持ち駒（捕獲のみモードでは生成しないのが一般的）
			else if (!capturesOnly) {
				Drop::ComputeDropMoveRange(moveRange, units, u.type, board.currentTeam);
				for (int y = 0; y < BoardSize; ++y) {
					for (int x = 0; x < BoardSize; ++x) {
						if (moveRange[y][x] < 0) continue;
						Candidate c;
						c.unitIndex = i;
						c.target = Point{ x, y };
						c.isDrop = true;
						c.promoteDecision = false;
						c.score = 0; // 打ちは評価低めスタート
						out.push_back(c);
					}
				}
			}
		}
	}

	// ---------------------------------------------------------
	// 評価関数 (高速版)
	// ---------------------------------------------------------

	// 駒1つあたりのスコアを計算（盤上の位置、囲い、持ち駒補正など）
	inline int BasicTestAI::EvalUnitIndividual(const Unit& u, Team rootAI, Point myKing)
	{
		// 基本価値
		int val = PieceValue(u.type, BasicTestAIConfig::PieceValue::TOTAL_VAL);

		if (u.isOnBoard) {
			// --- A. 位置評価 (PST) ---
			// 前進ボーナス
			int advance = (rootAI == Team::Blue) ? (BoardSize - 1 - u.pos.y) : u.pos.y;

			// 駒種別の位置補正
			if (u.type == UnitType::King || u.type == UnitType::King1) {
				// 王は端が良い、中央は危険
				if (u.pos.x <= 1 || u.pos.x >= 7) val += BasicTestAIConfig::Evaluation::EDGE_PROXIMITY_PENALTY;
				else if (u.pos.x >= 3 && u.pos.x <= 5) val -= BasicTestAIConfig::Evaluation::CENTER_PROXIMITY_PENALTY;
			}
			else if (u.type == UnitType::Gold || u.type == UnitType::Silver) {
				// 金銀は中央が良い
				if (u.pos.x >= 3 && u.pos.x <= 5) val += BasicTestAIConfig::Evaluation::CENTER_GOLD_SILVER_BONUS;
				val += advance * BasicTestAIConfig::Evaluation::ADVANCEMENT_BONUS;// 前進も評価
			}
			else {
				val += advance * BasicTestAIConfig::Evaluation::ADVANCEMENT_BONUS;
			}

			// 歩の特定位置ボーナス（3段目/5段目？）
			if (u.team == rootAI && u.type == UnitType::Pawn) {
				int p = (rootAI == Team::Blue) ? 3 : 5;

				if (u.pos.y == p) val += BasicTestAIConfig::Evaluation::ADVANCED_PAWN_BONUS;// 3段目/5段目にいる歩は高評価
			}

			// --- B. 王の守り (Castle) ---
			// 味方の王の近くにいる金銀を評価
			if (u.team == rootAI && myKing.x >= 0) {
				if (u.type == UnitType::Gold || u.type == UnitType::Silver ||
					u.type == UnitType::Gold || u.type == UnitType::Silver1) {

					int dist = std::abs(u.pos.x - myKing.x) + std::abs(u.pos.y - myKing.y);
					if (dist <= 2) {
						val += (3 - dist) * BasicTestAIConfig::Evaluation::GOLD_SILVER_KING_PROXIMITY_BONUS;
					}
				}
			}
		}
		else {
			// --- C. 持ち駒価値 ---
			int handBonus = val / BasicTestAIConfig::HandPiece::HAND_PIECE_PENALTY;
			val += handBonus;
		}

		// AI視点ならプラス、敵視点ならマイナスで返す
		return (u.team == rootAI ? val : -val);
	}

	// 盤面全体の関係性（利き、王の安全、敵玉への圧力）を計算
	inline int BasicTestAI::EvalBoardGlobal(const FastBoard& board, const Array<Unit>& units, Team rootAI, Point myKing, Point enemyKing)
	{
		int score = 0;
		AttackMap enemyAttackMap;
		AttackMap myAttackMap;
		Team enemyTeam = (rootAI == Team::Blue ? Team::Red : Team::Blue);

		// 利きの計算（AttackMap生成）
		for (const auto& u : units) {
			if (!u.isOnBoard) continue;
			if (u.team == rootAI) myAttackMap.MarkFromUnit(u, board, rootAI);
			else enemyAttackMap.MarkFromUnit(u, board, enemyTeam);
		}

		// 1. 盤面支配率（Mobility）
		int myMobility = myAttackMap.CountCovered();
		int enemyMobility = enemyAttackMap.CountCovered();
		score += (myMobility - enemyMobility) * BasicTestAIConfig::PieceRisk::MOBILITY_BONUS;

		// 2. 自軍の王の安全度 (King Safety)
		if (myKing.x >= 0) {
			// 王手されているか
			if (enemyAttackMap.IsCovered(myKing)) {
				score -= BasicTestAIConfig::PieceRisk::KING_IN_DANGER_PENALTY;
			}
			// 敵駒との距離（近すぎると危険）
			for (const auto& u : units) {
				if (u.isOnBoard && u.team != rootAI) {
					int dist = std::abs(u.pos.x - myKing.x) + std::abs(u.pos.y - myKing.y);
					score -= (BasicTestAIConfig::Evaluation::DISTANCE_SCALE - dist) * BasicTestAIConfig::Evaluation::CAPTURE_BONUS_SCALE;
				}
			}
		}

		// 3. 敵玉への圧力 (Pressure)
		if (enemyKing.x >= 0) {
			// 王手をかけているか
			if (myAttackMap.IsCovered(enemyKing)) {
				score += BasicTestAIConfig::PieceRisk::CHECK_BONUS;
			}
			// 味方駒との距離（近いほど圧力）
			for (const auto& u : units) {
				if (u.isOnBoard && u.team == rootAI) {
					int dist = std::abs(u.pos.x - enemyKing.x) + std::abs(u.pos.y - enemyKing.y);
					score += (BasicTestAIConfig::Evaluation::DISTANCE_SCALE - dist) * BasicTestAIConfig::Evaluation::CAPTURE_BONUS_SCALE;
				}
			}
		}
		
		for (const auto& u : units) {
			if (!u.isOnBoard) continue;
			if (myAttackMap.IsCovered(u.pos)){
				if(u.team != rootAI) continue;
				score += PieceValue(u.type, BasicTestAIConfig::PieceValue::TOTAL_VAL) * BasicTestAIConfig::SearchConfig::AVOID_MOVING_INTO_DANGER_PENALTY;
				continue;
			}
			if (enemyAttackMap.IsCovered(u.pos)) {
				if (u.team == rootAI) continue;
				score -= PieceValue(u.type, BasicTestAIConfig::PieceValue::TOTAL_VAL) * BasicTestAIConfig::SearchConfig::AVOID_MOVING_INTO_DANGER_PENALTY;
				continue;
			}
		}

		return score;
	}
	Point BasicTestAI::FindKing(const Array<Unit>& units, Team team) {
		for (const auto& u : units) if (u.isOnBoard && u.team == team && (u.type == UnitType::King || u.type == UnitType::King1)) return u.pos;
		return { -1, -1 };
	}
	int BasicTestAI::Evaluate(int ID, Point pox, const FastBoard& board, Team rootAI)
	{
		int score = 0;
		const auto& units = board.units;

		// 王の位置特定
		Point myKing = FindKing(units, rootAI);
		Point enemyKing = FindKing(units, (rootAI == Team::Blue ? Team::Red : Team::Blue));

		// 1. 個々の駒の評価積算
		for (const auto& u : units)
		{
			// 基本的な駒の価値計算
			score += EvalUnitIndividual(u, rootAI, myKing);

			// 特定の候補手(ID, pox)に対する特殊ボーナス判定
			// （特定の場所に移動する行動自体への評価）
			if (u.isOnBoard && u.team == rootAI && u.pos == pox) {
				score += PieceValue(u.type, BasicTestAIConfig::PieceValue::TOTAL_VAL) * BasicTestAIConfig::SearchConfig::AVOID_MOVING_INTO_DANGER_PENALTY;
			}
		}

		// 2. 盤面全体（利き・王の安全）の評価加算
		score += EvalBoardGlobal(board, units, rootAI, myKing, enemyKing);

		return score;
	}
	// ---------------------------------------------------------
	// 探索 (Alpha-Beta + Quiescence)
	// ---------------------------------------------------------
	int32 BasicTestAI::CaptureScore(Array<Unit> units, Team rootAI, int32 ID, Point pox) {
		int32 score = 0;
		for (const auto& u : units)
		{
			if (u.team != rootAI)  continue;
			if (u.pos != pox)  continue;
			score += PieceValue(u.type, BasicTestAIConfig::PieceValue::TOTAL_VAL) * BasicTestAIConfig::SearchConfig::AVOID_MOVING_INTO_DANGER_PENALTY;
			return score;
		}
		return 0;
	}
	// 静止探索
	int BasicTestAI::Quiescence(int ID, Point pox, FastBoard& board, Team rootAI, int alpha, int beta, int depthLimit)
	{
		int stand_pat = Evaluate(ID, pox, board, rootAI);
		if (stand_pat >= beta) return beta;
		if (stand_pat > alpha) alpha = stand_pat;

		if (depthLimit <= 0) return alpha;

		std::vector<Candidate> moves;
		GenerateMoves(moves, board, true); // 捕獲のみ

		// ソート
		std::sort(moves.begin(), moves.end(), [](const Candidate& a, const Candidate& b) {
			return a.score > b.score;
		});

		for (auto& mv : moves) {
			FastBoard nextBoard = board; // 軽量コピー
			ExecuteMoveLocal(nextBoard, mv);

			int score = -Quiescence(mv.unitIndex, mv.target, nextBoard, rootAI, -beta, -alpha, depthLimit - 1);
			if (score >= beta) return beta;
			if (score > alpha) alpha = score;
		}
		return alpha;
	}

	// 通常探索
	int BasicTestAI::AlphaBeta(int ID, Point pox, FastBoard& board, Team rootAI, int depth, int alpha, int beta)
	{
		// 時間切れチェック（数千ノードに1回程度行うのが理想だが、ここでは簡易的に毎回チェック）
		if (g_timeUp) return 0;
		if ((std::chrono::steady_clock::now() - g_startTime) > std::chrono::milliseconds(BasicTestAIConfig::SearchConfig::TIME_LIMIT_MS)) {
			g_timeUp = true;
			return 0;
		}

		if (depth <= 0) {
			return Quiescence(ID, pox, board, rootAI, alpha, beta);
		}

		std::vector<Candidate> moves;
		GenerateMoves(moves, board, false);

		if (moves.empty()) return BasicTestAIConfig::System::LOSS_PENALTY; // 負け

		// ソート
		std::sort(moves.begin(), moves.end(), [](const Candidate& a, const Candidate& b) {
			return a.score > b.score;
		});

		int bestVal = std::numeric_limits<int>::min();

		for (auto& mv : moves) {
			FastBoard nextBoard = board; // 軽量コピー
			ExecuteMoveLocal(nextBoard, mv);

			int val = -AlphaBeta(mv.unitIndex, mv.target, nextBoard, rootAI, depth - 1, -beta, -alpha);

			// 時間切れなら中断
			if (g_timeUp) return 0;

			if (val > bestVal) bestVal = val;
			if (val > alpha) alpha = val;
			if (alpha >= beta) break;
		}
		return bestVal;
	}


	// ルート探索（反復深化）
	Candidate BasicTestAI::IterativeDeepening(Game& game, Team aiTeam, AIState& state)
	{
		g_startTime = std::chrono::steady_clock::now();
		g_timeUp = false;

		FastBoard rootBoard(game);
		Candidate bestMove;
		bestMove.score = -BasicTestAIConfig::System::INITIAL_CAPTURE_BONUS;

		// 最初の1手が見つからない場合の保険
		std::vector<Candidate> firstMoves;
		GenerateMoves(firstMoves, rootBoard, false);
		if (firstMoves.empty()) return bestMove;
		bestMove = firstMoves[0]; // とりあえず先頭を入れておく

		// 深さ1から徐々に深くしていく
		for (int depth = 1; depth <= BasicTestAIConfig::SearchConfig::MAX_DEPTH; ++depth)
		{
			int alpha = std::numeric_limits<int>::min();
			constexpr int beta = std::numeric_limits<int>::max();
			Candidate currentDepthBest = bestMove;
			int currentDepthMaxScore = std::numeric_limits<int>::min();

			// ルートノードの展開
			std::vector<Candidate> moves;
			GenerateMoves(moves, rootBoard, false);

			// 前回の最善手を優先的に探索するようにソート（Move Orderingの強化）
			std::sort(moves.begin(), moves.end(), [&](const Candidate& a, const Candidate& b) {
				bool aIsBest = (a.from == bestMove.from && a.target == bestMove.target);
				bool bIsBest = (b.from == bestMove.from && b.target == bestMove.target);

				// どちらか片方だけが最善手の場合
				if (aIsBest != bIsBest) {
					return aIsBest; // aが最善なら先に(true)、bが最善なら後に(false)
				}
				// 両方最善、あるいは両方違う場合はスコアで比較
				return a.score > b.score;
	});
			bool completeSearch = true;

			for (auto& mv : moves) {
				FastBoard nextBoard = rootBoard;
				ExecuteMoveLocal(nextBoard, mv);

				// 千日手ペナルティ（ルートのみ）
				int penalty = 0;
				if (mv.from == state.lastMoveFrom
					&& mv.unitIndex == state.ID) penalty = BasicTestAIConfig::System::AVOID_BACKTRACK_PENALTY;

				int score = -AlphaBeta(mv.unitIndex, mv.target, nextBoard, aiTeam, depth - 1, -beta, -alpha);
				score -= penalty;

				if (g_timeUp) {
					completeSearch = false;
					break;
				}

				if (score > currentDepthMaxScore) {
					currentDepthMaxScore = score;
					currentDepthBest = mv;
					currentDepthBest.score = score;
				}
				if (score > alpha) alpha = score;
			}

			if (!completeSearch) {
				break;
			}

			// この深さでの探索完了。結果を採用。
			bestMove = currentDepthBest;

			// 詰みが見つかったら（非常に高いスコア）そこで終了しても良い
			if (bestMove.score > BasicTestAIConfig::System::CHECKMATE_THRESHOLD) break;
		}

		return bestMove;
	}

// ---------------------------------------------------------
// Public API
// ---------------------------------------------------------
// 最良候補を実行する（Game API を使う）
// 成りの実行（Game API を利用）
void BasicTestAI::CarryOutPromotionIfNeeded(Game& game, const Candidate& cand)
{
	if (game.GetPendingPromotionIndex() < 0) return;
	if (!cand.promoteDecision) { game.CancelPromotionByIndex(cand.unitIndex); return; }
	game.ApplyPromotionByIndex(cand.unitIndex);
}
void BasicTestAI::ExecuteCandidate(Game& game, const Candidate& cand)
{
	if (cand.unitIndex < 0) return;
	game.SelectUnitByIndex(cand.unitIndex);
	game.ExecuteMoveOrCaptureSelected(cand.target.x, cand.target.y);
	CarryOutPromotionIfNeeded(game, cand);
	game.TurnEndDetermination();
}
void BasicTestAI::StartThinking(Game& game, Team aiTeam, AIState& state) {
	if (IsThinking()) return;
	Game& g = game;
	// 非同期で実行開始
	aiTask = Async([=, &g, &state]() {
		return IterativeDeepening(g, aiTeam, state);
	});
}

void BasicTestAI::MakeMove(Game& game, Team aiTeam)
{
	if (!game.IsGameActive()) return;
	if (aiTask.isReady()) {
		Candidate best = aiTask.get();
		if (best.unitIndex >= 0) {
			aiState.lastMoveFrom = best.from;
			aiState.ID = best.unitIndex;
			ExecuteCandidate(game, best);
		}
	}
	else
	{
		StartThinking(game, aiTeam, aiState);
	}
}
