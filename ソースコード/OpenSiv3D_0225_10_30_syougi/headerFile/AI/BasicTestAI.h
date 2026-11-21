#pragma once
#include "AI/AI.h"

class BasicTestAI : public IShogiAI {
public:
	BasicTestAI();
	virtual ~BasicTestAI() {
		// AIが思考中にオブジェクトが破棄されるのを防ぐ
		if (aiTask.isValid()) {
			aiTask.wait();
		}
	}
	void MakeMove(Game& game, Team aiTeam) override;
	String getName() const override { return U"テストAI"; }

	void ExecuteCandidate(Game& game, const Candidate& cand);
	
private:
	struct FastBoard {
		Array<Unit> units;
		Team currentTeam;
		uint64 hash;
		const Grid<int32>* mapRef;
		const HashTable<int32, ChipInfo>* chipDBRef;
		const HashTable<UnitType, UnitInfo>* unitDBRef;



		// 持ち駒カウント [type][teamIdx]
		int handCount[UnitTypeArraySize][TEAM_COUNT];
		// 盤上占有マップ: -1 = 空, それ以外 = units のインデックス
		int occ[BoardSize][BoardSize];
		// 王の位置キャッシュ: index 0 = Blue, 1 = Red
		Point kingPos[TEAM_COUNT];

		// --- 追加: ビットボード表現（差分更新用） ---
		Bitboard occBB; // 全占有ビットボード
		std::array<Bitboard, UnitTypeArraySize> pieceBB; // 駒種別占有（インデックスは UnitType の列挙値に対応）
		Bitboard teamBB[TEAM_COUNT]; // チーム別占有: [0]=Blue, [1]=Red

		// --- 追加: PST 合計値 (チーム別) ---
		int pstScore[TEAM_COUNT] = { 0, 0 }; // pstScore[0]=Blue 合計, pstScore[1]=Red 合計
		int handScore[TEAM_COUNT] = { 0,0 };
		int materialScore[TEAM_COUNT] = { 0,0 };// 駒の価値合計（持ち駒も含む）
		//盤面の駒の数（持ち駒は含まない）
		int pieceCount = 0; // pieceCount[0]=Blue 合計, pieceCount[1]=Red 合計

		// 呼び出し側(ShidenAI)のメンバ関数を使えるようにコンストラクタを修正
		FastBoard(const Game& game, BasicTestAI& ai);
		// occ を作るユーティリティ（DoMove/UndoMove で差分更新可能にする）
		void BuildOcc();
		// handCount を再集計するユーティリティ（実装は AI.cpp に移動）
		void RecountHand();
	};
	struct AttackMap {
		Bitboard attackBB;
		void Reset();
		bool IsCovered(const Point& p) const;
		void MarkFromUnit(const Unit& u, FastBoard& fastBoard, Team viewTeam);
		// 盤面全体からビット演算で利きを合成する（完全ビットボード版）
		void BuildFromBoard(const FastBoard& board, Team enemyTeam);
		Bitboard AttacksFromUnit(const FastBoard& board, const Unit& u);
	};
	struct PiecesID {
		Array<int> kingID;
		Array<int> goldID;
		Array<int> silverID;
		Array<int> bishopID;
		Array<int> pawnID;
		Array<int> rookID;
		Array<int> lanceID;
		Array<int> knightID;

		void PiecesIDSearch(const Array<Unit>& units);
	};
	struct MoveInfo {
		int unitIdx;
		Point from;
		Point to;
		UnitType capturedType; // 取った駒（なければNone）
		bool promoted;         // この手で成ったか
		uint64 oldHash;        // 指す前のハッシュ（戻す用）

		// 追加：捕獲駒の正確な復元用フィールド
		bool captured = false;    // この手で駒を取ったか
		int capturedIndex = -1;   // 取られた駒の units 配列内インデックス
		Unit capturedBefore;      // 取られる前の駒の完全な状態（復元用）

		// 追加: PST 差分を Undo のために保持
		int pstDelta[TEAM_COUNT] = { 0, 0 }; // pstDelta[0]=Blue 増分, pstDelta[1]=Red 増分
		int handScore[TEAM_COUNT] = { 0,0 };
		int materialScore[TEAM_COUNT] = { 0,0 };
		int pieceCount = 0; // 駒数の増減（持ち駒は含まない）
	};

	// --- 思考用ヘルパー関数 (メンバ関数) ---

	Array<TTEntry> transpositionTable;
	uint8 currentAge = 0;
	uint64 zobristTable[UnitTypeArraySize][TEAM_COUNT][NumTiles];
	uint64 zobristHand[UnitTypeArraySize][TEAM_COUNT][ZOBRIST_HAND_SLOTS];
	uint64 zobristSide;
	Candidate killerMoves[KILLER_TABLE_PLY][TEAM_COUNT];
	PiecesID piecesID;
	SlidingTable g_slidingTable;
	bool g_slidingTableBuilt = false;

	// --- 追加: デバッグ用文字列化 ---
	Array<String> forDebugging;

	int historyTable[TEAM_COUNT][UnitTypeArraySize][NumTiles];
	std::chrono::steady_clock::time_point startTime;
	std::chrono::steady_clock::time_point mateStartTime;
	bool timeUp = false;
	Array<uint64> searchStack;
	AsyncTask<Candidate> aiTask;

	// --- 追加メンバ: 移動候補プール ---
	std::vector<std::vector<Candidate>> movePool; // 深さごとに再利用するバッファ群


	// プール取得ヘルパ（実装は cpp 側）
	std::vector<Candidate>& GetMoveListForDepth(int depth, bool quies = false);

	void TickNode();
	bool IsEndgame(const FastBoard& board) const;// 終盤判定

	int GetDynamicKingPST(const FastBoard& board, Team team, int y, int x, AttackMap& enemyAttackMap);// 動的王位置点を取得
	Point FindKing(const Array<Unit>& units, Team team);// 王の位置を見つける
	void InitZobrist();// Zobristハッシュ初期化
	uint64 CalculateHash(const Array<Unit>& units, Team currentTeam);// ハッシュ計算
	int DistanceScore(const Point& a, const Point& b);
	int GetUnitIndexAt(const FastBoard& board, int x, int y);
	int GetMultiplierPercent(UnitType t, const FastBoard& board);

	void ComputePinnedPieces(const FastBoard& board, Array<int>& outPinnedIndices, Array<Point>& outAttackerPos);// 鋭角に並んでいるか
	bool IsOnSameLine(const Point& a, const Point& b, const Point& p) const;// 3点が一直線上にあるか

	int StaticExchangeEvaluation(const FastBoard& board, int tx, int ty, int attackerIndex);// 静的交換評価
	int StaticExchangeEvaluationForDrop(const FastBoard& board, int tx, int ty, UnitType dropType, Team dropTeam);
	int GetPSTValue(UnitType type, int y, int x, Team team);// 駒ごとの位置点を取得
	int ComputeUnitPST(const FastBoard& board, const Unit& u);
	int EvalBasicMaterial(const FastBoard& board, Team rootAI);// 基本的な駒の価値評価
	double CalculatePhase(const FastBoard& board);// ゲームの進行度を計算
	int EvalTacticalStability(const FastBoard& board, const Unit& u, Team rootAI, AttackMap& myAttacks, AttackMap& enemyAttacks);// 戦術的安定性評価

	inline int PawnStructure(const Unit& u, Team rootAI, const Array<Unit>& units);// 歩の構造評価
	inline int EvalUnitIndividual(const Unit& u, Team rootAI, Point myKing, Point enKing, const Array<Unit>& units);// 駒個別評価

	bool IsSquareAttacked(FastBoard& board, Point p, Team enemyTeam);// 指定マスが攻撃されているか
	int CountEnemyAttacksAroundKing(FastBoard& board, Team myTeam, Point myKingPos);// 王の周囲の敵の攻撃数を数える
	int CountKingLiberty(Point kingPos, const AttackMap& enemyAttacks);// 王の逃げ道を数える
	int CountSafeKingLiberties(const FastBoard& board, Point kingPos, Team kingTeam);//
	int EvaluateDetailed(FastBoard& board, Team rootAI, bool heavy);// 詳細評価関数
	int FloatingPiecePenalty(const Unit& u, Team aiTeam, AttackMap& attacBkyMyself, AttackMap& enemyByAttack);
	int FindLeastValuableAttacker(const FastBoard& board, int tx, int ty, Team attackerTeam);
	void ExecuteMoveLocal(FastBoard& board, const Candidate& cand);// ローカル盤面で手を実行

	void GenerateMoves(std::vector<Candidate>& out, const FastBoard& board, bool capturesOnly);// 手の生成
	void GenerateTacticalMoves(std::vector<Candidate>& out, const FastBoard& board, Point lastCapturedPos = Point{ -1, -1 });
	void GeneratePieceCandidates(std::vector<Candidate>& out, FastBoard& root, Team aiTeam);// 駒の移動候補を生成
	void DoMove(FastBoard& board, const Candidate& cand, MoveInfo& info);// 手を実行
	void UndoMove(FastBoard& board, const MoveInfo& info);// 手を戻す
	void SortMoves(std::vector<Candidate>& moves, const FastBoard& board, int depth, int ttIdx, Point ttTarget);// 手のソート
	void ClearHeuristics();// ヒューリスティクスのクリア
	void StoreTT(uint64 hash, int depth, int score, HashFlag flag, const Candidate& move);// トランスポジションテーブルに保存
	TTEntry* LookupTT(uint64 hash);// トランスポジションテーブルの検索


	bool GivesCheckAfterMove(BasicTestAI& ai, FastBoard& board, const Candidate& cand, Team mover);// 手を指した後にチェックを与えるか
	bool MateDFS(BasicTestAI& ai, BasicTestAI::FastBoard& board, Team sideToMove, Team attacker, int depth);// 深さ優先探索で詰みを探す
	bool FindMateAtRoot(BasicTestAI& ai, BasicTestAI::FastBoard& root, Team aiTeam, int maxDepth, Candidate& outMove);// ルートで詰みを探す

	int Quiescence(FastBoard& board, Team rootAI, int alpha, int beta, int q_depth, bool heavy);// クイースセンス探索
	int AlphaBeta(FastBoard& board, Team rootAI, int depth, int alpha, int beta, int dist);// アルファベータ探索
	Candidate IterativeDeepening(Game& game, Team aiTeam, AIState& state);// 反復深化探索
	void UpdateHandHash(FastBoard& board, UnitType type, int teamIdx, int delta);// 持ち駒ハッシュの更新
	void XORPieceHash(FastBoard& board, int unitIndex);// 駒ハッシュのXOR更新
	void XORSide(FastBoard& board);// 手番ハッシュのXOR更新

	void StartThinking(Game& game, Team aiTeam, AIState& state);// 思考開始
	bool IsThinking() const { return aiTask.isValid() && !aiTask.isReady(); }// 思考中か

	// --- デバッグ用ヘルパー関数 ---
	void DisplayAttackMap(Game& game, Team team);
};
