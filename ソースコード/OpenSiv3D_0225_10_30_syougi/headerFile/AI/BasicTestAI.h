#pragma once
#include "AI/AI.h"

// --- テスト用AI (簡易) ---
class BasicTestAI : public IShogiAI {
public:
	BasicTestAI();
	void MakeMove(Game& game, Team aiTeam) override;
	String getName() const override { return U"テストAI"; }
private:
	struct FastBoard
	{
		// 探索に必要なデータのみを持つ
		Array<Unit> units;
		Team currentTeam;

		// 参照用データ（コピーしない！）
		const Grid<int32>* mapRef = nullptr;
		const HashTable<int32, ChipInfo>* chipDBRef = nullptr;
		const HashTable<UnitType, UnitInfo>* unitDBRef = nullptr;

		// Gameから初期化するコンストラクタ
		FastBoard(const Game& game);
		// コピーコンストラクタ（参照ポインタはそのままコピー）
		FastBoard(const FastBoard& other) = default;

		// ユニットリストへのアクセサ
		const Array<Unit>& GetUnitList() const { return units; }
	};
	struct AttackMap
	{
		bool grid[BoardSize][BoardSize] = {};

		void Reset();
		bool IsCovered(const Point& p) const;

		int CountCovered() const;

		void MarkFromUnit(const Unit& u,
			FastBoard fastBoard,
			Team viewTeam);
	};
	// 時間管理用グローバル変数（探索中に参照）
	std::chrono::steady_clock::time_point g_startTime;
	bool g_timeUp = false;
	AsyncTask<Candidate> aiTask;

	// 基本的なヘルパー関数
	Point FindKing(const Array<Unit>& units, Team team);// 王の位置を見つける

	// 候補手生成用ヘルパー関数
	void ExecuteMoveLocal(FastBoard& board, const Candidate& cand);// ローカル盤面で手を実行
	void GenerateMoves(std::vector<Candidate>& out, const FastBoard& board, bool capturesOnly = false);// 移動候補を生成

	// 盤面評価用ヘルパー関数
	inline int EvalUnitIndividual(const Unit& u, Team rootAI, Point myKing);// 個別ユニット評価
	inline int EvalBoardGlobal(const FastBoard& board, const Array<Unit>& units, Team rootAI, Point myKing, Point enemyKing);// 盤面全体評価
	int Evaluate(int ID, Point pox, const FastBoard& board, Team rootAI);// 盤面評価関数
	int32 CaptureScore(Array<Unit> units, Team rootAI, int32 ID, Point pox);// 捕獲スコア計算

	// 探索用ヘルパー関数
	int Quiescence(int ID, Point pox, FastBoard& board, Team rootAI, int alpha, int beta, int depthLimit = 4);// 静止探索
	int AlphaBeta(int ID, Point pox, FastBoard& board, Team rootAI, int depth, int alpha, int beta);// αβ探索
	Candidate IterativeDeepening(Game& game, Team aiTeam, AIState& state);// 反復深化探索

	// AIの手続き
	void CarryOutPromotionIfNeeded(Game& game, const Candidate& cand);// 必要に応じて成りを実行
	void ExecuteCandidate(Game& game, const Candidate& cand);// 候補手を実行
	void StartThinking(Game& game, Team aiTeam, AIState& state);// 思考開始
	bool IsThinking() const { return aiTask.isValid() && !aiTask.isReady(); }// 思考中か

};

