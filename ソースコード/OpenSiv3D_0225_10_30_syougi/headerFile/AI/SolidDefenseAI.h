#pragma once
#include "AI/AI.h"

// --- 金剛AI (守備重視) ---
class SolidDefenseAI : public IShogiAI {
public:
	SolidDefenseAI();
	void MakeMove(Game& game, Team aiTeam) override;
	String getName() const override { return U"金剛AI"; }
private:

	struct AttackMap {
		bool grid[BoardSize][BoardSize];
		void Reset();
		bool IsCovered(const Point& p) const;
		void MarkFromUnit(const Unit& u,
					  const Array<Unit>& units,
					  const Grid<int32>& map,
					  const HashTable<int32, ChipInfo>& chipDB,
					  const HashTable<UnitType, UnitInfo>& unitDB,
					  Team aiTeam);
	};
	// 思考用グローバル変数
	AttackMap g_enemyAttackMap;
	Point g_lastMoveFrom{ -1, -1 };
	int g_ID = 0;
	AsyncTask<Candidate> aiTask;

	// --- 基本的なヘルパー関数 ---
	int DistanceScore(const Point& a, const Point& b);//2点間のマンハッタン距離に基づくスコアを計算
	Point FindEnemyKing(const Array<Unit>& units, Team aiTeam);// 敵王の位置を見つける
	Point FindMyKing(const Array<Unit>& units, Team aiTeam);// 自軍王の位置を見つける

	// --- 候補手生成用ヘルパー関数 ---
	void GeneratePieceCandidates(std::vector<Candidate>& out,
								const Array<Unit>& units,
								const Grid<int32>& map,
								const HashTable<int32, ChipInfo>& chipDB,
								const HashTable<UnitType, UnitInfo>& unitDB,
								Team aiTeam, AttackMap attackMap);// 駒の移動候補を生成
	void GenerateDropCandidates(std::vector<Candidate>& out,
							   const Array<Unit>& units,
							   Team aiTeam);// 手駒の打ち候補を生成

	// --- 盤面評価用ヘルパー関数 ---
	int EvaluateBoard(const Array<Unit>& units, Team aiTeam);// 盤面評価関数
	int ScoreCandidate(const Candidate& cand,
					  const Array<Unit>& units,
					  Team aiTeam,
					  const Point& enemyKing);// 候補手のスコアを計算
	int EvaluateAfterMove(Game gameCopy, Candidate myMove, const Array<Unit>& originalUnits,
						const HashTable<int32, ChipInfo>& chipDB,
						const Grid<int32>& map,
						const HashTable<UnitType, UnitInfo>& unitDB,
						Team aiTeam, const Point& enemyKing);// 手を実行した後の盤面を評価
	int CountOwnPiecesUnderAttack(const Array<Unit>& units,
								Team aiTeam,
								const Grid<int32>& map,
								const HashTable<int32, ChipInfo>& chipDB,
								const HashTable<UnitType, UnitInfo>& unitDB);// 自軍の攻撃されている駒の数を数える
	Candidate IterativeDeepening(Game& game, Team aiTeam);// 反復深化探索

	// --- AIの手続き ---
	void CarryOutPromotionIfNeeded(Game& game, const Candidate& cand);// 必要に応じて成りを実行
	void ExecuteCandidate(Game& game, const Candidate& cand);// 候補手を実行
	void StartThinking(Game& game, Team aiTeam);// 思考開始
	bool IsThinking() const { return aiTask.isValid() && !aiTask.isReady(); }// 思考中か

};
