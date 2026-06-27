#include "stdafx.h"
#include "AI/ShidenAI.h"
#include "Game/Promotion.h"
#include "Logic/MoveStrategy.h" // 追加: 駒戦略インターフェースを利用するため
#include <vector>
#include <limits>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <unordered_map>
#include <random>


namespace ShidenAIConfig {
	namespace SearchConfig {

		inline  int TIME_LIMIT_MS = 3000;// ミリ秒
		inline  int MAX_DEPTH = 12;// 探索深さの最大値
		inline  int MAX_QUIESCENCE_DEPTH = 8;// 静止探索の最大深さ
		inline  int MAX_MOVES_PER_POSITION = 128;// 1局面あたりの最大候補手数（枝刈りのため）

		inline  int FALLBACK_TOP_N = 3;     // デフォルト: 上位3件を再評価
		inline  int FALLBACK_DEEP_DEPTH = 3; // デフォルト: AlphaBeta を depth=3

		//Null Move Pruningの減少深さ
		inline  int NULL_MOVE_REDUCTION = 3;// デフォルト: 3

		
		inline  int MATE_TRY_DEPTH = 6;// 詰み探索を試みる深さ
	
		inline  int MATE_SEARCH_DECAY_PER_DEPTH = 4;

		inline  int MATE_SEARCH_EXTENSION = 3;// 詰み探索時の深さ延長
		inline  int MATE_SEARCH_MIN_MOVES = 5;// 詰み探索を開始する最小手数
	};
	namespace System {

		constexpr  int UNIT_TYPE_MAX = UnitTypeArraySize;
		constexpr  int BOARD_SQUARES = NumTiles;

		inline  int SCORE_INF = 10000000;// 無限大スコア（十分大きい値）

		inline  int REPETITION_DRAW_SCORE = 0;// 千日手の引き分けスコア

		inline  int PERCENT_BASE = 100;// パーセント表記の基数（100で100%を表す）

		inline  uint64_t RANDOM_SEED = 123456789ULL;// ランダムシード（固定値で再現性を確保）

		inline  int SEE_SCALE = 100;// SEEのスケーリング係数（捕獲評価の値を調整するため）

		inline  int MIN_HAND_MATERIAL_VALUERATE = 70;// 持ち駒の価値率の最小値（SEEで捕獲評価を行うかの基準）

		inline  int NODES_PER_TIME_CHECK = 4096u;

		inline  int BASE_CAPTURE_SCORE = 2000000;

	}
	namespace TTConfig {
		constexpr size_t TT_SIZE = 1 << 20;// 1Mエントリ
		constexpr size_t TT_MASK = TT_SIZE - 1;// マスク値
	};
	namespace Evaluation {

		inline  int CHECKMATE_TO_ENEMY_KING = 1500;// 敵王に詰みをかける手の評価値
		inline  int BOTTOM_STEP_BONUS = 20;// 盤面の底に近いほど高評価
		inline  int ROUND_HEADED_PIECE_BONUS = 40;// 盤面の中心に近いほど高評価
		inline  int KING_TRAPPED_PENALTY = 99999999;

		inline  int DROOPING_STEP_BONUS = 30;// 盤面の端に近いほど高評価
		inline  int CASTLE_NEAR_KING_BONUS = 600;// 王の近くにいる駒を評価
		inline  int ACTIVITY_BONUS = 50;// 駒の活動度に対するボーナス

		inline  int ADVANCE_BONUS = 15;// 前進に対するボーナス
		inline  int KING_SHIELD_BONUS = 300;// 王の前にいる駒を評価

		inline  int DRAGON_HORSE_ACTIVITY_BONUS = 50;// 馬の活動度に対するボーナス

		inline  int SUPPORT_BONUS = 100;// 駒を支えている駒がある場合のボーナス
		inline  int NO_ENEMY_ATTACK_BONUS = 600;// 敵の攻撃がない駒に対するボーナス

		inline  int PROMOTION_ONLY_BONUS = 50;// 成る手であること自体のボーナス
		inline  int PROMOTION_BONUS = 200;// 成りのボーナス（駒の価値に対して相対的なもの）

		inline  int PAWN_ADVANCE_BONUS = 20;// 歩の前進に対するボーナス
		inline  double PAWN_PHASE_DIVISOR = 18.0;// 歩の前進ボーナスの段階的減衰のための除数

		inline  int CAPTURE_BASE_VALUE_FACTOR = 10;// 捕獲基本価値倍率
		inline  int CAPTURE_BONUS_MULTIPLIER = 50;// 捕獲ボーナス倍率

	}
	namespace Endgame {

		inline  int ENDGAME_PIECE_COUNT_THRESHOLD = 6;// 終盤とみなす駒の総数の閾値

		inline  int ENDGAME_CENTER_WEIGHT = 30;// 盤面の中心にいる駒の評価を上げるための重み
		inline  int ENDGAME_ENEMY_ZONE_BONUS = 40;// 敵陣にいる駒の評価を上げるためのボーナス
		inline  int ENDGAME_LIBERTY_WEIGHT = 80;// 駒の自由度（逃げ道）の重み

	}
	// 駒の価値や評価関数の定数
	namespace MoveOrdering {

		inline  int MOVE_ORDER_KILLER_SCORE = 1500000;
		inline  int MOVE_ORDER_CAPTURE_SCORE = 2000000;
		inline  int MOVE_ORDER_TT_SCORE = 3000000;

		inline  int BECOMING_CAPTURE_BONUS = 500;// 成る手で相手の駒を取る手に対するボーナス
		inline  int POSSIBLE_HAND_BONUSES = 1000;// 持ち駒にできる手に対するボーナス

		inline  int BECOMING_CAPTURE_3POINTS_BONUS =
			MOVE_ORDER_CAPTURE_SCORE / 3;

		inline  int PROXIMITY_BONUS_UNIT =
			MOVE_ORDER_CAPTURE_SCORE / 6;

		inline  int PROXIMITY_BONUS_MAX_DISTANCE = 2;
		inline  int PROXIMITY_BONUS_MAX = 3;// 王に近いほど高いボーナスの最大値

		inline  int PROXIMITY_BONUS_BASE =
			MOVE_ORDER_TT_SCORE / 4;
		inline  int DROP_BONUS = 20;// 持ち駒を打つ手に対するボーナス

	}
	namespace PieceValue {
		// 評価定数（100倍スケール）
		inline  PIECE_VALUE VAL = {
		100,// 歩
		500,// 香
		550,// 桂
		800,// 銀
		900,// 金
		1500,// 角
		2000,// 飛
		100000,// 王
		0// 成りのボーナス（駒の価値に対して相対的なもの）
		};
		// 成り駒の価値（捕獲評価用）
		inline  PromotedPieceValue PROM_VAL = {
			700,// と金
			300,// 成香
			250,// 成桂
			100,// 成銀
			250,// 馬
			250// 龍
		};
		inline  TotalPieceValue TOTAL_VAL = {
		 VAL.PAWN,
		 VAL.LANCE,
		 VAL.KNIGHT,
		 VAL.SILVER,
		 VAL.GOLD,
		 VAL.BISHOP,
		 VAL.ROOK,
		 VAL.KING,
		 VAL.PAWN + PROM_VAL.PROMOTED_PAWN,
		 VAL.LANCE + PROM_VAL.PROMOTED_LANCE,
		 VAL.KNIGHT + PROM_VAL.PROMOTED_KNIGHT,
		 VAL.SILVER + PROM_VAL.PROMOTED_SILVER,
		 VAL.BISHOP + PROM_VAL.PROMOTED_BISHOP,
		 VAL.ROOK + PROM_VAL.PROMOTED_ROOK

		};
	};
	namespace PieceRisk {
		//狙われている駒の価値
		inline  PIECE_VALUE UNDER_ATTACK_PENALTY
			= {
				40,// 歩
				200,// 香
				220,// 桂
				320,// 銀
				350,// 金
				500,// 角
				700,// 飛
				30000,// 王
				300// 成りのボーナス
		};
		inline  TotalPieceValue TOTAL_UNDER_ATTACK_PENALTY = {
			UNDER_ATTACK_PENALTY.PAWN,
			UNDER_ATTACK_PENALTY.LANCE,
			UNDER_ATTACK_PENALTY.KNIGHT,
			UNDER_ATTACK_PENALTY.SILVER,
			UNDER_ATTACK_PENALTY.GOLD,
			UNDER_ATTACK_PENALTY.BISHOP,
			UNDER_ATTACK_PENALTY.ROOK,
			UNDER_ATTACK_PENALTY.KING,
			UNDER_ATTACK_PENALTY.PAWN + UNDER_ATTACK_PENALTY.PROMOTION_BONUS,
			UNDER_ATTACK_PENALTY.LANCE + UNDER_ATTACK_PENALTY.PROMOTION_BONUS,
			UNDER_ATTACK_PENALTY.KNIGHT + UNDER_ATTACK_PENALTY.PROMOTION_BONUS,
			UNDER_ATTACK_PENALTY.SILVER + UNDER_ATTACK_PENALTY.PROMOTION_BONUS,
			UNDER_ATTACK_PENALTY.BISHOP + UNDER_ATTACK_PENALTY.PROMOTION_BONUS,
			UNDER_ATTACK_PENALTY.ROOK + UNDER_ATTACK_PENALTY.PROMOTION_BONUS
		};


	};
	namespace HandPiece {
		inline  PIECE_VALUE HAND_MULT
			= {
				150,// 歩の持ち駒は1.5倍
				100,// 香の持ち駒は1.0倍
				100,// 桂の持ち駒は1.0倍
				100,// 銀の持ち駒は1.0倍
				100,// 金の持ち駒は1.0倍
				90,// 角の持ち駒は0.9倍
				90,  // 飛車の持ち駒は0.9倍
				10000,// 王の持ち駒は存在しないが、念のため元値を維持
				0// 成りのボーナスは持ち駒には適用しない
		};

		inline  int HAND_PIECE_BONUS_PERCENT = 10;

		inline  int PAWN_HAND_OPENING_EXTRA_PERCENT = 30;

	};

	namespace PST {
		using Table = std::array<std::array<int, 9>, 9>;

		// 歩のテーブル (先手用)
		// 敵陣（上部）に行くほど高く、中央ほど高い
		constexpr Table PAWN = { {
			{  0,   0,   0,   0,   0,   0,   0,   0,   0}, // 1段目(成)
			{ 50,  60,  70,  80,  90,  80,  70,  60,  50}, // 2段目
			{ 40,  50,  60,  70,  80,  70,  60,  50,  40}, // 3段目
			{ 20,  30,  40,  50,  60,  50,  40,  30,  20},
			{ 10,  15,  20,  25,  30,  25,  20,  15,  10},
			{  5,  10,  10,  10,  10,  10,  10,  10,   5},
			{  0,   0,   0,   0,   0,   0,   0,   0,   0},
			{  0,   0,   0,   0,   0,   0,   0,   0,   0},
			{  0,   0,   0,   0,   0,   0,   0,   0,   0}
		} };

		// 玉のテーブル (先手用: 左側への囲いを推奨する例)
		constexpr Table KING = { {
		{-150, -150, -150, -150, -150, -150, -150, -150, -150},
		{-120, -120, -120, -120, -120, -120, -120, -120, -120},
		{-100, -100, -100, -100, -100, -100, -100, -100, -100},
		{ -80,  -80,  -80,  -80,  -80,  -80,  -80,  -80,  -80},
		{ -50,  -50,  -80, -150, -150, -150,  -80,  -50,  -50}, // 5段目：中央への進出を拒否
		{ -20,  -20,  -50, -200, -250, -200,  -50,  -20,  -20}, // 6段目：同上
		{  20,   20,    0, -250, -300, -250,    0,   20,   20}, // 7段目：同上
		{  80,  120,   50, -300, -400, -300,   50,  120,   80}, // 8段目：中央3列を「地獄」化
		{ 150,  250,  100, -400, -600, -400,  100,  250,  150}  // 9段目：隅(0,1,7,8列)を「聖域」化
	} };

		// 桂馬：中央への跳ね出しを高く、1〜2段目（行き止まり）と端を極端に低く
		constexpr Table KNIGHT = { {
		   {-100, -80, -70, -70, -70, -70, -70, -80,-100}, // 1段目(詰み)
		   {-50, -20,   0,   0,   0,   0,   0, -20, -50}, // 2段目(ほぼ詰み)
		   { 10,  40,  60,  60,  60,  60,  60,  40,  10}, // 3段目(攻めの拠点)
		   {  0,  20,  30,  40,  50,  40,  30,  20,   0},
		   {  0,  10,  20,  25,  30,  25,  20,  10,   0},
		   {  0,   0,   0,   0,   0,   0,   0,   0,   0},
		   {  0,   0,   0,   0,   0,   0,   0,   0,   0},
		   {  0,   0,   0,   0,   0,   0,   0,   0,   0},
		   {  0,   0,   0,   0,   0,   0,   0,   0,   0}
		} };

		// 金・成駒：玉の周辺と中央を高く。守備力を数値化
		constexpr Table GOLD = { {
			{  0,  10,  20,  30,  30,  30,  20,  10,   0}, // 敵陣
			{ 10,  20,  30,  40,  40,  40,  30,  20,  10},
			{ 15,  25,  35,  45,  50,  45,  35,  25,  15},
			{ 10,  20,  30,  40,  45,  40,  30,  20,  10},
			{  0,  10,  20,  30,  35,  30,  20,  10,   0},
			{  0,   5,  15,  20,  25,  20,  15,   5,   0},
			{ 10,  20,  30,  20,  10,  20,  30,  20,  10}, // 自陣：守備
			{ 30,  50,  60,  40,  20,  40,  60,  50,  30}, // 玉の囲い想定位置を高く
			{ 20,  40,  50,  30,  10,  30,  50,  40,  20}
		} };

		// 飛車：2段目（龍の横利き）と、中央の縦ラインを重視
		constexpr Table ROOK = { {
			{ 20,  20,  20,  20,  20,  20,  20,  20,  20},
			{ 50,  50,  60,  80,  90,  80,  60,  50,  50}, // 2段目(龍)
			{ 20,  30,  40,  50,  60,  50,  40,  30,  20},
			{  0,  10,  20,  30,  40,  30,  20,  10,   0},
			{  0,  10,  20,  30,  40,  30,  20,  10,   0},
			{  0,   0,  10,  20,  20,  20,  10,   0,   0},
			{  0,   0,   0,   0,   0,   0,   0,   0,   0},
			{  0,   0,   0,   0,   0,   0,   0,   0,   0},
			{  0,   0,   0,   0,   0,   0,   0,   0,   0}
		} };
		// --- 銀将 (SILVER) ---
	  // 中央への進出と、2段目での攻守の切り替えを評価
		constexpr Table SILVER = { {
			{ 0, 10, 15, 20, 20, 20, 15, 10,  0}, // 敵陣深く
			{10, 30, 40, 50, 60, 50, 40, 30, 10}, // 攻めの拠点
			{ 5, 20, 35, 45, 50, 45, 35, 20,  5},
			{ 0, 10, 25, 35, 40, 35, 25, 10,  0},
			{ 0,  5, 15, 25, 30, 25, 15,  5,  0},
			{ 0,  0, 10, 15, 20, 15, 10,  0,  0},
			{ 5, 15, 15,  5,  0,  5, 15, 15,  5}, // 自陣：守備
			{15, 35, 25, 15,  0, 15, 25, 35, 15}, // 囲いの横
			{ 0, 10, 10,  0,  0,  0, 10, 10,  0}
		} };

		// --- 香車 (LANCE) ---
		// 前方への利きを重視。1段目は成らなければ動けないため評価を下げ、2~3段目を高く
		constexpr Table LANCE = { {
			{ 0,  0,  0,  0,  0,  0,  0,  0,  0}, // 1段目（強制成りのためPSTは0）
			{40, 50, 60, 60, 60, 60, 60, 50, 40}, // 2段目（最強）
			{30, 40, 45, 45, 45, 45, 45, 40, 30},
			{20, 30, 35, 35, 35, 35, 35, 30, 20},
			{10, 20, 25, 25, 25, 25, 25, 20, 10},
			{ 5, 10, 15, 15, 15, 15, 15, 10,  5},
			{ 0,  5, 10, 10, 10, 10, 10,  5,  0},
			{ 0,  0,  5,  5,  5,  5,  5,  0,  0},
			{ 0,  0,  0,  0,  0,  0,  0,  0,  0}
		} };

		// --- 角行 (BISHOP) ---
		// 盤面中央付近にいるほど、斜めの利きが最大化されるため高く評価
		constexpr Table BISHOP = { {
			{ 0, 15, 15, 15, 15, 15, 15, 15,  0},
			{15, 30, 30, 30, 30, 30, 30, 30, 15},
			{15, 30, 50, 55, 55, 55, 50, 30, 15},
			{15, 30, 55, 75, 80, 75, 55, 30, 15},
			{15, 30, 55, 80, 100, 80, 55, 30, 15}, // 中央(55)が最強
			{15, 30, 55, 75, 80, 75, 55, 30, 15},
			{15, 30, 50, 55, 55, 55, 50, 30, 15},
			{10, 20, 20, 20, 20, 20, 20, 20, 10},
			{ 0,  0,  0,  0,  0,  0,  0,  0,  0}
		} };

	};
	void Load() {
		const TOMLReader toml{ U"example/ExternalConfigurationFile/ShidenAI.toml" };
		if (not toml) {
			Print << U"TOML読み込み失敗！";
			return;
		}

		SearchConfig::TIME_LIMIT_MS = toml[U"SearchConfig.TIME_LIMIT_MS"].get<int>();
		SearchConfig::MAX_DEPTH = toml[U"SearchConfig.MAX_DEPTH"].get<int>();
		SearchConfig::MAX_QUIESCENCE_DEPTH = toml[U"SearchConfig.MAX_QUIESCENCE_DEPTH"].get<int>();
		SearchConfig::MAX_MOVES_PER_POSITION = toml[U"SearchConfig.MAX_MOVES_PER_POSITION"].get<int>();
		SearchConfig::FALLBACK_TOP_N = toml[U"SearchConfig.FALLBACK_TOP_N"].get<int>();
		SearchConfig::FALLBACK_DEEP_DEPTH = toml[U"SearchConfig.FALLBACK_DEEP_DEPTH"].get<int>();
		SearchConfig::NULL_MOVE_REDUCTION = toml[U"SearchConfig.NULL_MOVE_REDUCTION"].get<int>();
		SearchConfig::MATE_TRY_DEPTH = toml[U"SearchConfig.MATE_TRY_DEPTH"].get<int>();
		SearchConfig::MATE_SEARCH_DECAY_PER_DEPTH = toml[U"SearchConfig.MATE_SEARCH_DECAY_PER_DEPTH"].get<int>();
		SearchConfig::MATE_SEARCH_EXTENSION = toml[U"SearchConfig.MATE_SEARCH_EXTENSION"].get<int>();
		SearchConfig::MATE_SEARCH_MIN_MOVES = toml[U"SearchConfig.MATE_SEARCH_MIN_MOVES"].get<int>();

		System::SCORE_INF = toml[U"System.SCORE_INF"].get<int>();
		System::REPETITION_DRAW_SCORE = toml[U"System.REPETITION_DRAW_SCORE"].get<int>();
		System::PERCENT_BASE = toml[U"System.PERCENT_BASE"].get<int>();
		System::SEE_SCALE = toml[U"System.SEE_SCALE"].get<int>();
		System::MIN_HAND_MATERIAL_VALUERATE = toml[U"System.MIN_HAND_MATERIAL_VALUERATE"].get<int>();
		System::BASE_CAPTURE_SCORE = toml[U"System.BASE_CAPTURE_SCORE"].get<int>();

		Evaluation::CHECKMATE_TO_ENEMY_KING = toml[U"Evaluation.CHECKMATE_TO_ENEMY_KING"].get<int>();
		Evaluation::BOTTOM_STEP_BONUS = toml[U"Evaluation.BOTTOM_STEP_BONUS"].get<int>();
		Evaluation::ROUND_HEADED_PIECE_BONUS = toml[U"Evaluation.ROUND_HEADED_PIECE_BONUS"].get<int>();
		Evaluation::KING_TRAPPED_PENALTY = toml[U"Evaluation.KING_TRAPPED_PENALTY"].get<int>();
		Evaluation::DROOPING_STEP_BONUS = toml[U"Evaluation.DROOPING_STEP_BONUS"].get<int>();
		Evaluation::CASTLE_NEAR_KING_BONUS = toml[U"Evaluation.CASTLE_NEAR_KING_BONUS"].get<int>();
		Evaluation::ACTIVITY_BONUS = toml[U"Evaluation.ACTIVITY_BONUS"].get<int>();
		Evaluation::ADVANCE_BONUS = toml[U"Evaluation.ADVANCE_BONUS"].get<int>();
		Evaluation::KING_SHIELD_BONUS = toml[U"Evaluation.KING_SHIELD_BONUS"].get<int>();
		Evaluation::DRAGON_HORSE_ACTIVITY_BONUS = toml[U"Evaluation.DRAGON_HORSE_ACTIVITY_BONUS"].get<int>();
		Evaluation::SUPPORT_BONUS = toml[U"Evaluation.SUPPORT_BONUS"].get<int>();
		Evaluation::NO_ENEMY_ATTACK_BONUS = toml[U"Evaluation.NO_ENEMY_ATTACK_BONUS"].get<int>();
		Evaluation::PROMOTION_ONLY_BONUS = toml[U"Evaluation.PROMOTION_ONLY_BONUS"].get<int>();
		Evaluation::PROMOTION_BONUS = toml[U"Evaluation.PROMOTION_BONUS"].get<int>();
		Evaluation::PAWN_ADVANCE_BONUS = toml[U"Evaluation.PAWN_ADVANCE_BONUS"].get<int>();
		Evaluation::PAWN_PHASE_DIVISOR = toml[U"Evaluation.PAWN_PHASE_DIVISOR"].get<double>();
		Evaluation::CAPTURE_BASE_VALUE_FACTOR = toml[U"Evaluation.CAPTURE_BASE_VALUE_FACTOR"].get<int>();
		Evaluation::CAPTURE_BONUS_MULTIPLIER = toml[U"Evaluation.CAPTURE_BONUS_MULTIPLIER"].get<int>();

		Endgame::ENDGAME_PIECE_COUNT_THRESHOLD = toml[U"Endgame.ENDGAME_PIECE_COUNT_THRESHOLD"].get<int>();
		Endgame::ENDGAME_CENTER_WEIGHT = toml[U"Endgame.ENDGAME_CENTER_WEIGHT"].get<int>();
		Endgame::ENDGAME_ENEMY_ZONE_BONUS = toml[U"Endgame.ENDGAME_ENEMY_ZONE_BONUS"].get<int>();
		Endgame::ENDGAME_LIBERTY_WEIGHT = toml[U"Endgame.ENDGAME_LIBERTY_WEIGHT"].get<int>();

		MoveOrdering::MOVE_ORDER_KILLER_SCORE = toml[U"MoveOrdering.MOVE_ORDER_KILLER_SCORE"].get<int>();
		MoveOrdering::MOVE_ORDER_CAPTURE_SCORE = toml[U"MoveOrdering.MOVE_ORDER_CAPTURE_SCORE"].get<int>();
		MoveOrdering::MOVE_ORDER_TT_SCORE = toml[U"MoveOrdering.MOVE_ORDER_TT_SCORE"].get<int>();
		MoveOrdering::BECOMING_CAPTURE_BONUS = toml[U"MoveOrdering.BECOMING_CAPTURE_BONUS"].get<int>();
		MoveOrdering::POSSIBLE_HAND_BONUSES = toml[U"MoveOrdering.POSSIBLE_HAND_BONUSES"].get<int>();
		MoveOrdering::PROXIMITY_BONUS_MAX_DISTANCE = toml[U"MoveOrdering.PROXIMITY_BONUS_MAX_DISTANCE"].get<int>();
		MoveOrdering::PROXIMITY_BONUS_MAX = toml[U"MoveOrdering.PROXIMITY_BONUS_MAX"].get<int>();
		MoveOrdering::DROP_BONUS = toml[U"MoveOrdering.DROP_BONUS"].get<int>();

		int captureBonusDivisor = toml[U"MoveOrdering.BECOMING_CAPTURE_3POINTS_BONUS"].get<int>();
		MoveOrdering::BECOMING_CAPTURE_3POINTS_BONUS =
			MoveOrdering::MOVE_ORDER_CAPTURE_SCORE / captureBonusDivisor;

		int proximityBonusUnitDivisor = toml[U"MoveOrdering.PROXIMITY_BONUS_UNIT"].get<int>();
		MoveOrdering::PROXIMITY_BONUS_UNIT =
			MoveOrdering::MOVE_ORDER_CAPTURE_SCORE / proximityBonusUnitDivisor;

		int proximityBonusBaseDivisor = toml[U"MoveOrdering.PROXIMITY_BONUS_BASE"].get<int>();
		MoveOrdering::PROXIMITY_BONUS_BASE =
			MoveOrdering::MOVE_ORDER_CAPTURE_SCORE / proximityBonusBaseDivisor;
	

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
			PieceValue::VAL.PAWN + PieceValue::PROM_VAL.PROMOTED_PAWN,
			PieceValue::VAL.LANCE + PieceValue::PROM_VAL.PROMOTED_LANCE,
			PieceValue::VAL.KNIGHT + PieceValue::PROM_VAL.PROMOTED_KNIGHT,
			PieceValue::VAL.SILVER + PieceValue::PROM_VAL.PROMOTED_SILVER,
			PieceValue::VAL.BISHOP + PieceValue::PROM_VAL.PROMOTED_BISHOP,
			PieceValue::VAL.ROOK + PieceValue::PROM_VAL.PROMOTED_ROOK
		};

		PieceRisk::UNDER_ATTACK_PENALTY.PAWN = toml[U"PieceRisk.PAWN"].get<int>();
		PieceRisk::UNDER_ATTACK_PENALTY.LANCE = toml[U"PieceRisk.LANCE"].get<int>();
		PieceRisk::UNDER_ATTACK_PENALTY.KNIGHT = toml[U"PieceRisk.KNIGHT"].get<int>();
		PieceRisk::UNDER_ATTACK_PENALTY.SILVER = toml[U"PieceRisk.SILVER"].get<int>();
		PieceRisk::UNDER_ATTACK_PENALTY.GOLD = toml[U"PieceRisk.GOLD"].get<int>();
		PieceRisk::UNDER_ATTACK_PENALTY.BISHOP = toml[U"PieceRisk.BISHOP"].get<int>();
		PieceRisk::UNDER_ATTACK_PENALTY.ROOK = toml[U"PieceRisk.ROOK"].get<int>();
		PieceRisk::UNDER_ATTACK_PENALTY.KING = toml[U"PieceRisk.KING"].get<int>();
		PieceRisk::UNDER_ATTACK_PENALTY.PROMOTION_BONUS = toml[U"PieceRisk.PROMOTION_BONUS"].get<int>();

		PieceRisk::TOTAL_UNDER_ATTACK_PENALTY = {
			PieceRisk::UNDER_ATTACK_PENALTY.PAWN,
			PieceRisk::UNDER_ATTACK_PENALTY.LANCE,
			PieceRisk::UNDER_ATTACK_PENALTY.KNIGHT,
			PieceRisk::UNDER_ATTACK_PENALTY.SILVER,
			PieceRisk::UNDER_ATTACK_PENALTY.GOLD,
			PieceRisk::UNDER_ATTACK_PENALTY.BISHOP,
			PieceRisk::UNDER_ATTACK_PENALTY.ROOK,
			PieceRisk::UNDER_ATTACK_PENALTY.KING,
			PieceRisk::UNDER_ATTACK_PENALTY.PAWN + PieceRisk::UNDER_ATTACK_PENALTY.PROMOTION_BONUS,
			PieceRisk::UNDER_ATTACK_PENALTY.LANCE + PieceRisk::UNDER_ATTACK_PENALTY.PROMOTION_BONUS,
			PieceRisk::UNDER_ATTACK_PENALTY.KNIGHT + PieceRisk::UNDER_ATTACK_PENALTY.PROMOTION_BONUS,
			PieceRisk::UNDER_ATTACK_PENALTY.SILVER + PieceRisk::UNDER_ATTACK_PENALTY.PROMOTION_BONUS,
			PieceRisk::UNDER_ATTACK_PENALTY.BISHOP + PieceRisk::UNDER_ATTACK_PENALTY.PROMOTION_BONUS,
			PieceRisk::UNDER_ATTACK_PENALTY.ROOK + PieceRisk::UNDER_ATTACK_PENALTY.PROMOTION_BONUS
		};

		HandPiece::HAND_MULT.PAWN = toml[U"HandPiece.PAWN"].get<int>();
		HandPiece::HAND_MULT.LANCE = toml[U"HandPiece.LANCE"].get<int>();
		HandPiece::HAND_MULT.KNIGHT = toml[U"HandPiece.KNIGHT"].get<int>();
		HandPiece::HAND_MULT.SILVER = toml[U"HandPiece.SILVER"].get<int>();
		HandPiece::HAND_MULT.GOLD = toml[U"HandPiece.GOLD"].get<int>();
		HandPiece::HAND_MULT.BISHOP = toml[U"HandPiece.BISHOP"].get<int>();
		HandPiece::HAND_MULT.ROOK = toml[U"HandPiece.ROOK"].get<int>();
		HandPiece::HAND_MULT.KING = toml[U"HandPiece.KING"].get<int>();
		HandPiece::HAND_MULT.PROMOTION_BONUS = toml[U"HandPiece.PROMOTION_BONUS"].get<int>();

		HandPiece::HAND_PIECE_BONUS_PERCENT = toml[U"HandPiece.HAND_PIECE_BONUS_PERCENT"].get<int>();
		HandPiece::PAWN_HAND_OPENING_EXTRA_PERCENT = toml[U"HandPiece.PAWN_HAND_OPENING_EXTRA_PERCENT"].get<int>();
		
	}

}; ShidenAI::ShidenAI() {
	ShidenAIConfig::Load();
	std::memset(zobristTable, 0, sizeof(zobristTable));
	std::memset(zobristHand, 0, sizeof(zobristHand));
	std::memset(historyTable, 0, sizeof(historyTable));
	zobristSide = 0;
	currentAge = 0;
	// killerMoves も初期化
	for (int i = 0; i < KILLER_TABLE_PLY; ++i) {
		killerMoves[i][0] = Candidate{};
		killerMoves[i][1] = Candidate{};
	}
}


static inline int GetHandMultiplierPercent(UnitType t) {
	using namespace ShidenAIConfig::HandPiece;
	switch (t) {
	case UnitType::Pawn:    return HAND_MULT.PAWN;
	case UnitType::Lance:   return HAND_MULT.LANCE;
	case UnitType::Knight:  return HAND_MULT.KNIGHT;
	case UnitType::Silver:  return HAND_MULT.SILVER;
	case UnitType::Gold:    return HAND_MULT.GOLD;
	case UnitType::Bishop:  return HAND_MULT.BISHOP;
	case UnitType::Rook:    return HAND_MULT.ROOK;
	default:
		// 既存の互換性を保つ：従来の HAND_PIECE_BONUS_PERCENT をデフォルトに
		return (ShidenAIConfig::System::PERCENT_BASE + ShidenAIConfig::HandPiece::HAND_PIECE_BONUS_PERCENT);
	}
}
int ShidenAI::GetMultiplierPercent(UnitType t, const FastBoard& board) {
	int base = GetHandMultiplierPercent(t);

	// 盤上駒数（玉以外）を数える（フェーズ計算用）
	int pieceCount = 0;
	for (const auto& u : board.units) {
		if (!u.isOnBoard) continue;
		if (u.type == UnitType::King || u.type == UnitType::King1) continue;
		++pieceCount;
	}
	// フェーズ: 1.0 = 序盤/中盤（駒が多い）、0.0 = 終盤（駒が少ない）
	double phase = static_cast<double>(pieceCount) / ShidenAIConfig::Evaluation::PAWN_PHASE_DIVISOR;
	if (phase < 0.0) phase = 0.0;
	if (phase > 1.0) phase = 1.0;

	// 終盤判定（既存の互換性を保つ）
	if (!IsEndgame(board)) {
		// 序盤/中盤：特定の駒種（歩）の手駒価値を上乗せ
		if (t == UnitType::Pawn) {
			int extra = static_cast<int>((ShidenAIConfig::HandPiece::PAWN_HAND_OPENING_EXTRA_PERCENT * phase));
			int adjusted = base + extra;
			return adjusted;
		}
		return base;
	}

	// 終盤：既存ロジック（減衰）
	// 盤上駒数（玉以外）を数える（末尾ロジックの互換性保持）
	int pc = 0;
	for (const auto& u : board.units) {
		if (!u.isOnBoard) continue;
		if (u.type == UnitType::King || u.type == UnitType::King1) continue;
		++pc;
	}
	const int threshold = ShidenAIConfig::Endgame::ENDGAME_PIECE_COUNT_THRESHOLD;
	int depth = std::max(0, threshold - pc);
	const int reduceUnit = ShidenAIConfig::SearchConfig::MATE_SEARCH_DECAY_PER_DEPTH;
	int reducePercent = depth * reduceUnit;
	int minPercent = (base * ShidenAIConfig::System::MIN_HAND_MATERIAL_VALUERATE) / ShidenAIConfig::System::PERCENT_BASE;
	int adjusted = base - reducePercent;
	if (adjusted < minPercent) adjusted = minPercent;
	return adjusted;
}

std::vector<Candidate>& ShidenAI::GetMoveListForDepth(int depth, bool quies) {
	// 遅延初期化
	if (movePool.empty()) {
		int total = ShidenAIConfig::SearchConfig::MAX_DEPTH + ShidenAIConfig::SearchConfig::MAX_QUIESCENCE_DEPTH;
		movePool.resize(total);
		for (auto& v : movePool) v.reserve(ShidenAIConfig::SearchConfig::MAX_MOVES_PER_POSITION);
	}

	// depth をそのまま ply として使う呼び出しもあるため対応。
	// だが推奨は searchStack.size() を渡すこと（呼び出し側で設定）。
	int ply = depth;
	if (ply < 0) ply = static_cast<int>(searchStack.size());

	int idx = 0;
	if (quies) {
		idx = ShidenAIConfig::SearchConfig::MAX_DEPTH + std::max(0, ply);
	}
	else {
		idx = std::max(0, ply);
	}
	if (idx < 0) idx = 0;
	if (idx >= static_cast<int>(movePool.size())) idx = static_cast<int>(movePool.size()) - 1;
	return movePool[idx];
}

// 指定した王の周囲8マスに敵の利きがいくつあるかを数える
int ShidenAI::CountEnemyAttacksAroundKing(FastBoard& board, Team myTeam, Point myKingPos) {
	if (myKingPos.x == -1) return 0;
	int attackCount = 0;
	AttackMap enemyAttackMap;
	enemyAttackMap.Reset();
	Team enemyTeam = (myTeam == Team::Blue ? Team::Red : Team::Blue);
	for (const auto& u : board.units) {
		if (u.isOnBoard && u.team == enemyTeam) enemyAttackMap.MarkFromUnit(u, board, enemyTeam);
	}
	for (int dy = -1; dy <= 1; ++dy) {
		for (int dx = -1; dx <= 1; ++dx) {
			if (dx == 0 && dy == 0) continue;
			if (enemyAttackMap.IsCovered({ myKingPos.x + dx, myKingPos.y + dy })) attackCount++;
		}
	}
	return attackCount;
}


Point ShidenAI::FindKing(const Array<Unit>& units, Team team) {
	for (const auto& ID : piecesID.kingID) {
		if (ID < 0 || ID >= static_cast<int>(units.size())) continue;
		const Unit& u = units[ID];
		if (u.isOnBoard && u.team == team && (u.type == UnitType::King || u.type == UnitType::King1)) return u.pos;
	}
	return { -1, -1 };
}
int ShidenAI::ComputeUnitPST(const FastBoard& board, const Unit& u) {
	if (!u.isOnBoard) return 0;
	return GetPSTValue(u.type, u.pos.y, u.pos.x, u.team);
}


ShidenAI::FastBoard::FastBoard(const Game& game, ShidenAI& ai)
	: units(game.GetUnitList())
	, currentTeam(game.GetCurrentTeam())
	, mapRef(&game.GetMap())
	, chipDBRef(&game.GetChipDB())
	, unitDBRef(&game.GetUnitDB())
{
	ai.InitZobrist();
	std::memset(handCount, 0, sizeof(handCount));
	// ビットボードをクリア
	occBB.clear();
	teamBB[0].clear(); teamBB[1].clear();
	for (auto& b : pieceBB) b.clear();

	for (auto& u : units) {
		if (!u.isOnBoard) {
			UnitType dem = ToDemoted(u.type);
			u.type = dem; // 内部表現を統一
			int t = (int)dem;
			int ti = (u.team == Team::Blue ? 0 : 1);
			if (t >= 0 && t < UnitTypeArraySize) handCount[t][ti]++;
		}
	}
	// hash を計算
	hash = ai.CalculateHash(units, currentTeam);
	// occ と kingPos を構築（BuildOcc がビットボードも構築）
	BuildOcc();

	// --- PST 合計を初期化 ---
	pstScore[0] = pstScore[1] = 0;
	handScore[0] = handScore[1] = 0;
	materialScore[0] = materialScore[1] = 0;
	pieceCount = 0;
	for (const auto& u : units) {

		int teamIdx = (u.team == Team::Blue) ? 0 : 1;
		Point myKing = kingPos[teamIdx];
		Point enKing = kingPos[1 - teamIdx];
		if (u.isOnBoard){
			materialScore[teamIdx] += PieceValue(u.type, ShidenAIConfig::PieceValue::TOTAL_VAL);
			materialScore[teamIdx] += ai.EvalUnitIndividual(u, u.team, myKing, enKing, units);
			pieceCount++;
			if (u.type == UnitType::King || u.type == UnitType::King1) {

			}
			else {
				pstScore[teamIdx] += ai.GetPSTValue(u.type, u.pos.y, u.pos.x, u.team);
			}
	     }
		else
		{
			handScore[teamIdx] += (PieceValue(u.type, ShidenAIConfig::PieceValue::TOTAL_VAL) * ai.GetMultiplierPercent(u.type, *this)) / ShidenAIConfig::System::PERCENT_BASE;
		}
	}
	

}


void ShidenAI::FastBoard::BuildOcc() {
	for (int y = 0; y < BoardSize; ++y)
		for (int x = 0; x < BoardSize; ++x)
			occ[y][x] = -1;
	kingPos[0] = { -1, -1 };
	kingPos[1] = { -1, -1 };

	// ビットボードをクリア
	occBB.clear();
	teamBB[0].clear(); teamBB[1].clear();
	for (auto& b : pieceBB) b.clear();

	for (int i = 0; i < static_cast<int>(units.size()); ++i) {
		const auto& u = units[i];
		if (!u.isOnBoard) continue;
		if (u.pos.y >= 0 && u.pos.y < BoardSize && u.pos.x >= 0 && u.pos.x < BoardSize) {
			occ[u.pos.y][u.pos.x] = i;
			const int sq = u.pos.y * BoardSize + u.pos.x;
			// 範囲チェック
			if (sq >= 0 && sq < NumTiles) {
				occBB.set(sq);
				int tIdx = static_cast<int>(u.type);
				if (tIdx >= 0 && tIdx < static_cast<int>(pieceBB.size())) pieceBB[tIdx].set(sq);
				int teamIdx = (u.team == Team::Blue) ? 0 : 1;
				teamBB[teamIdx].set(sq);
			}
		}
		if (u.type == UnitType::King || u.type == UnitType::King1) {
			int idx = (u.team == Team::Blue) ? 0 : 1;
			kingPos[idx] = u.pos;
		}
	}
}


void ShidenAI::PiecesID::PiecesIDSearch(const Array<Unit>& units) {
	kingID.clear();
	pawnID.clear();
	rookID.clear();
	lanceID.clear();
	knightID.clear();
	goldID.clear();
	silverID.clear();
	bishopID.clear();
	for (const auto& [i, u] : IndexedRef(units)) {
		if (!u.isOnBoard) continue;
		if (u.type == UnitType::Pawn ||
			u.type == UnitType::Pawn1) {
			pawnID << i; continue;
		}
		if (u.type == UnitType::Knight ||
			u.type == UnitType::Knight1) {
			knightID << i; continue;
		}
		if (u.type == UnitType::Lance ||
			u.type == UnitType::Lance1) {
			lanceID << i; continue;
		}
		if (u.type == UnitType::Gold) { goldID << i; continue; }
		if (u.type == UnitType::King ||
			u.type == UnitType::King1) {
			kingID << i; continue;
		}
		if (u.type == UnitType::Bishop ||
			u.type == UnitType::Bishop1) {
			bishopID << i; continue;
		}
		if (u.type == UnitType::Rook ||
			u.type == UnitType::Rook1) {
			rookID << i; continue;
		}
		if (u.type == UnitType::Silver ||
			u.type == UnitType::Silver1) {
			silverID << i; continue;
		}
	}
}



void ShidenAI::InitZobrist() {
	if (!transpositionTable.empty()) return;
	transpositionTable.resize(ShidenAIConfig::TTConfig::TT_SIZE);
	std::mt19937_64 engine(ShidenAIConfig::System::RANDOM_SEED);// 固定シードで初期化
	std::uniform_int_distribution<uint64> dist;
	for (int i = 0; i < ShidenAIConfig::System::UNIT_TYPE_MAX; ++i)
		for (int j = 0; j < 2; ++j) {
			for (int k = 0; k < ShidenAIConfig::System::BOARD_SQUARES; ++k) zobristTable[i][j][k] = dist(engine);
			for (int n = 0; n < ZOBRIST_HAND_SLOTS; ++n) zobristHand[i][j][n] = dist(engine);
		}
	zobristSide = dist(engine);
}



uint64 ShidenAI::CalculateHash(const Array<Unit>& units, Team currentTeam) {
	uint64 h = 0;
	int handCountLocal[ShidenAIConfig::System::UNIT_TYPE_MAX][2] = {};
	for (const auto& u : units) {
		int teamIdx = (u.team == Team::Blue ? 0 : 1);
		if (u.isOnBoard) {
			int typeIdx = (int)u.type;
			h ^= zobristTable[typeIdx][teamIdx][u.pos.y * BoardSize + u.pos.x];
		}
		else {
			// 持ち駒は非成に正規化してカウント
			UnitType dem = ToDemoted(u.type);
			int typeIdx = (int)dem;
			if (typeIdx >= 0 && typeIdx < ShidenAIConfig::System::UNIT_TYPE_MAX) handCountLocal[typeIdx][teamIdx]++;
		}
	}
	for (int i = 0; i < ShidenAIConfig::System::UNIT_TYPE_MAX; ++i)
		for (int j = 0; j < 2; ++j)
			if (handCountLocal[i][j] > 0 && handCountLocal[i][j] < ZOBRIST_HAND_SLOTS) h ^= zobristHand[i][j][handCountLocal[i][j]];
	if (currentTeam == Team::Red) h ^= zobristSide;
	return h;
}

// ヒューリスティクス情報をクリア
void ShidenAI::ClearHeuristics() {
	std::memset(killerMoves, 0, sizeof(killerMoves));
	std::memset(historyTable, 0, sizeof(historyTable));
}


void ShidenAI::StoreTT(uint64 hash, int depth, int score, HashFlag flag, const Candidate& move) {
	auto& entry = transpositionTable[hash & ShidenAIConfig::TTConfig::TT_MASK];
	if (entry.age != currentAge || depth >= entry.depth) {
		entry = { hash, score, (int16)depth, flag, currentAge, move.unitIndex, move.target, move.promoteDecision };
	}
}

TTEntry* ShidenAI::LookupTT(uint64 hash) {
	auto& entry = transpositionTable[hash & ShidenAIConfig::TTConfig::TT_MASK];
	return (entry.hash == hash && entry.flag != HashFlag::None) ? &entry : nullptr;
}




void ShidenAI::UpdateHandHash(FastBoard& board, UnitType type, int teamIdx, int delta)
{
	UnitType dem = ToDemoted(type);
	int tIdx = (int)dem;
	if (tIdx < 0 || tIdx >= ShidenAIConfig::System::UNIT_TYPE_MAX) return;
	int cur = board.handCount[tIdx][teamIdx];
	if (cur > 0 && cur < ZOBRIST_HAND_SLOTS) board.hash ^= zobristHand[tIdx][teamIdx][cur]; // 既存カウントを除去
	int next = cur + delta;
	board.handCount[tIdx][teamIdx] = next;
	if (next > 0 && next < ZOBRIST_HAND_SLOTS) board.hash ^= zobristHand[tIdx][teamIdx][next]; // 新カウントを追加
}



void ShidenAI::XORPieceHash(FastBoard& board, int unitIndex)
{
	// 範囲外アクセスを完全に遮断する
	if (unitIndex < 0 || unitIndex >= static_cast<int>(board.units.size())) {
		return;
	}
	const Unit& u = board.units[unitIndex];
	if (!u.isOnBoard) return;
	int teamIdx = (u.team == Team::Blue ? 0 : 1);
	board.hash ^= zobristTable[(int)u.type][teamIdx][u.pos.y * BoardSize + u.pos.x];
}
void ShidenAI::XORSide(FastBoard& board)
{
	board.hash ^= zobristSide;
	board.currentTeam = (board.currentTeam == Team::Blue ? Team::Red : Team::Blue);
}


void ShidenAI::FastBoard::RecountHand() {
	std::memset(handCount, 0, sizeof(handCount));
	for (auto& u : units) {
		if (!u.isOnBoard) {
			// 持ち駒は内部表現を「非成駒」で統一する
			UnitType dem = ToDemoted(u.type);
			u.type = dem;
			int t = (int)dem;
			int ti = (u.team == Team::Blue ? 0 : 1);
			if (t >= 0 && t < UnitTypeArraySize) handCount[t][ti]++;
		}
	}
}

// ヘルパ: ビットボードから square index を安全に計算
static inline int PosToSqSafe(const Point& p) {
	if (p.x < 0 || p.y < 0 || p.x >= BoardSize || p.y >= BoardSize) return -1;
	return p.y * BoardSize + p.x;
}



void ShidenAI::DoMove(FastBoard& board, const Candidate& cand, MoveInfo& info)
{
	// 既存の先頭処理（状態保存）はそのまま
	info.unitIdx = cand.unitIndex;
	info.from = cand.from;
	info.to = cand.target;
	info.oldHash = board.hash;
	info.promoted = cand.promoteDecision;
	info.capturedType = UnitType::King;
	info.captured = false;
	info.capturedIndex = -1;
	info.pstDelta[0] = board.pstScore[0];
	info.pstDelta[1] = board.pstScore[1];
	info.handScore[0] = board.handScore[0];
	info.handScore[1] = board.handScore[1];
	info.materialScore[0] = board.materialScore[0];
	info.materialScore[1] = board.materialScore[1];
	info.pieceCount = board.pieceCount;

	Unit& u = board.units.at(cand.unitIndex);
	int teamIdx = (board.currentTeam == Team::Blue ? 0 : 1);
	int oppTeamIdx = 1 - teamIdx;
	Point myKing = board.kingPos[teamIdx];
	Point enKing = board.kingPos[oppTeamIdx];

	board.materialScore[teamIdx] -= EvalUnitIndividual(u, board.currentTeam, myKing, enKing, board.units);

	// --- PST 差分: 移動対象（と捕獲対象）の PST を事前に計算 ---
	int pstBeforeMover = 0;
	if (u.isOnBoard) pstBeforeMover = ComputeUnitPST(board, u);
	int pstBeforeVictim = 0;
	int victimIndex = -1;
	if (!cand.isDrop) {
		for (int i = 0; i < static_cast<int>(board.units.size()); ++i) {
			const Unit& target = board.units[i];
			if (target.isOnBoard && target.pos == cand.target && target.team != board.currentTeam) {
				victimIndex = i;
				pstBeforeVictim = ComputeUnitPST(board, target);
				break;
			}
		}
	}

	// 1) 移動駒の現在位置ハッシュを除去（盤上なら位置、手駒なら手駒カウントを減らす）
	if (u.isOnBoard) {
		XORPieceHash(board, cand.unitIndex);
		// occ は差分でクリア
		if (u.pos.x >= 0 && u.pos.x < BoardSize && u.pos.y >= 0 && u.pos.y < BoardSize) {
			int sq = u.pos.y * BoardSize + u.pos.x;
			board.occ[u.pos.y][u.pos.x] = -1;
			if (sq >= 0 && sq < NumTiles) {
				board.occBB.reset(sq);
				int tIdx = static_cast<int>(u.type);
				if (tIdx >= 0 && tIdx < static_cast<int>(board.pieceBB.size())) board.pieceBB[tIdx].reset(sq);
				board.teamBB[teamIdx].reset(sq);
			}
		}
	}
	else {
		UpdateHandHash(board, u.type, teamIdx, -1);
		board.handScore[teamIdx] -= (PieceValue(u.type, ShidenAIConfig::PieceValue::TOTAL_VAL) * GetMultiplierPercent(u.type, board)) / ShidenAIConfig::System::PERCENT_BASE;
	}

	// 2) 捕獲処理（存在すれば）
	if (!cand.isDrop) {
		for (int i = 0; i < static_cast<int>(board.units.size()); ++i) {
			Unit& target = board.units[i];
			if (target.isOnBoard && target.pos == cand.target && target.team != board.currentTeam) {
				// 情報保存（Undo 用）
				info.captured = true;
				info.capturedIndex = i;
				info.capturedBefore = target;
				info.capturedType = target.type;

				// 捕獲駒の位置ハッシュを除去
				XORPieceHash(board, i);

				board.handScore[teamIdx] += (PieceValue(target.type, ShidenAIConfig::PieceValue::TOTAL_VAL) * GetMultiplierPercent(target.type, board)) / ShidenAIConfig::System::PERCENT_BASE;
				board.pieceCount--; // 盤上駒が減る
				board.materialScore[teamIdx] += PieceValue(target.type, ShidenAIConfig::PieceValue::TOTAL_VAL);
				board.materialScore[oppTeamIdx] -= PieceValue(target.type, ShidenAIConfig::PieceValue::TOTAL_VAL);

				// 捕獲駒は非成に戻して持ち駒へ
				UnitType demoted = ToDemoted(target.type);
				UpdateHandHash(board, demoted, teamIdx, +1);

				// 盤上から取り除く
				target.isOnBoard = false;
				// ビットボード差分
				if (target.pos.x >= 0 && target.pos.x < BoardSize && target.pos.y >= 0 && target.pos.y < BoardSize) {
					int tsq = target.pos.y * BoardSize + target.pos.x;
					if (tsq >= 0 && tsq < NumTiles) {
						board.occBB.reset(tsq);
						int tidx = static_cast<int>(target.type);
						if (tidx >= 0 && tidx < static_cast<int>(board.pieceBB.size())) board.pieceBB[tidx].reset(tsq);
						int oppTeamIdxLocal = (target.team == Team::Blue ? 0 : 1);
						board.teamBB[oppTeamIdxLocal].reset(tsq);
					}
					board.occ[target.pos.y][target.pos.x] = -1;
				}

				// 捕獲駒のチーム/タイプを書き換え（持ち駒化）
				target.team = board.currentTeam;
				target.type = demoted;
				break;
			}
		}
	}

	// 3) 駒を配置
	if (cand.isDrop) {
		u.isOnBoard = true;
		u.pos = cand.target;
	}
	else {
		u.pos = cand.target;
		if (cand.promoteDecision) u.type = ToPromoted(u.type);
	}
	// occ 到着地をセット
	if (u.pos.x >= 0 && u.pos.x < BoardSize && u.pos.y >= 0 && u.pos.y < BoardSize) {
		board.occ[u.pos.y][u.pos.x] = cand.unitIndex;
		int nsq = u.pos.y * BoardSize + u.pos.x;
		if (nsq >= 0 && nsq < NumTiles) {
			board.occBB.set(nsq);
			int tIdx = static_cast<int>(u.type);
			if (tIdx >= 0 && tIdx < static_cast<int>(board.pieceBB.size())) board.pieceBB[tIdx].set(nsq);
			board.teamBB[teamIdx].set(nsq);
		}
	}
	if (u.type == UnitType::King || u.type == UnitType::King1) {
		for (const auto& ID : piecesID.goldID) {
			if (ID < 0 || ID >= static_cast<int>(board.units.size())) continue;
			const Unit& t = board.units[ID];
			board.materialScore[teamIdx] -= EvalUnitIndividual(u, board.currentTeam, myKing, enKing, board.units);
		}
		for (const auto& ID : piecesID.silverID) {
			if (ID < 0 || ID >= static_cast<int>(board.units.size())) continue;
			const Unit& t = board.units[ID];
			board.materialScore[teamIdx] -= EvalUnitIndividual(u, board.currentTeam, myKing, enKing, board.units);
		}
		for (const auto& ID : piecesID.knightID) {
			if (ID < 0 || ID >= static_cast<int>(board.units.size())) continue;
			const Unit& t = board.units[ID];
			board.materialScore[teamIdx] -= EvalUnitIndividual(u, board.currentTeam, myKing, enKing, board.units);
		}
	}

	// 4) 移動後の駒ハッシュを追加
	if (u.isOnBoard) XORPieceHash(board, cand.unitIndex);
	board.materialScore[teamIdx] += EvalUnitIndividual(u, board.currentTeam, myKing, enKing, board.units);
	// --- PST 差分の計算（移動後の PST を再計算し、差分を board.pstScore に反映） ---
	int pstAfterMover = 0;
	if (u.isOnBoard) pstAfterMover = ComputeUnitPST(board, u);

	// 移動駒のチームに対する delta
	int moverDelta = pstAfterMover - pstBeforeMover;
	board.pstScore[teamIdx] += moverDelta;
	// 捕獲があれば被捕獲駒の PST を相手チームから引く
	if (victimIndex != -1) {
		// 被捕獲駒は既に盤上から取り除かれているが pstBeforeVictim holds its previous PST
		board.pstScore[oppTeamIdx] -= pstBeforeVictim;

	}

	// 5) 王位置キャッシュ更新    
	if (u.type == UnitType::King || u.type == UnitType::King1) {
		board.kingPos[teamIdx] = u.pos;
		for (const auto& ID : piecesID.goldID) {
			if (ID < 0 || ID >= static_cast<int>(board.units.size())) continue;
			const Unit& t = board.units[ID];
			board.materialScore[teamIdx] += EvalUnitIndividual(u, board.currentTeam, myKing, enKing, board.units);
		}
		for (const auto& ID : piecesID.silverID) {
			if (ID < 0 || ID >= static_cast<int>(board.units.size())) continue;
			const Unit& t = board.units[ID];
			board.materialScore[teamIdx] += EvalUnitIndividual(u, board.currentTeam, myKing, enKing, board.units);
		}
		for (const auto& ID : piecesID.knightID) {
			if (ID < 0 || ID >= static_cast<int>(board.units.size())) continue;
			const Unit& t = board.units[ID];
			board.materialScore[teamIdx] += EvalUnitIndividual(u, board.currentTeam, myKing, enKing, board.units);
		}
	}

	// 6) 手番を切替え
	XORSide(board);
}

/* --- UndoMove: pstDelta を用いて PST 合計を復元 --- */
void ShidenAI::UndoMove(FastBoard& board, const MoveInfo& info)
{
	// 1) まず手番を戻す（DoMove の最後の XORSide を打ち消す）
	XORSide(board); // side と zobristSide を反転

	// PST 合計を差分で戻す（DoMove で加えた分を打ち消す）
	board.pstScore[0]  = info.pstDelta[0];
	board.pstScore[1]  = info.pstDelta[1];
	board.handScore[0] = info.handScore[0];
	board.handScore[1] = info.handScore[1];
	board.materialScore[0] = info.materialScore[0];
	board.materialScore[1] = info.materialScore[1];
	board.pieceCount = info.pieceCount;

	Unit& u = board.units.at(info.unitIdx);
	int teamIdx = (board.currentTeam == Team::Blue ? 0 : 1);

	// 2) 現在の移動駒位置のハッシュを除去（存在するなら）
	if (u.isOnBoard) {
		XORPieceHash(board, info.unitIdx);
		if (u.pos.x >= 0 && u.pos.x < BoardSize && u.pos.y >= 0 && u.pos.y < BoardSize) {
			int sq = u.pos.y * BoardSize + u.pos.x;
			board.occ[u.pos.y][u.pos.x] = -1;
			if (sq >= 0 && sq < NumTiles) {
				board.occBB.reset(sq);
				int tIdx = static_cast<int>(u.type);
				if (tIdx >= 0 && tIdx < static_cast<int>(board.pieceBB.size())) board.pieceBB[tIdx].reset(sq);
				board.teamBB[teamIdx].reset(sq);
			}
		}
	}

	// 3) 捕獲の復元（先に戻す）
	if (info.captured && info.capturedIndex >= 0) {
		// 持ち駒から差分で戻す
		UnitType demoted = ToDemoted(info.capturedBefore.type);
		UpdateHandHash(board, demoted, teamIdx, -1);

		// 盤上に駒を元の状態で復元
		board.units[info.capturedIndex] = info.capturedBefore;
		if (board.units.at(info.capturedIndex).isOnBoard) {
			auto& t = board.units.at(info.capturedIndex);
			if (t.pos.x >= 0 && t.pos.x < BoardSize && t.pos.y >= 0 && t.pos.y < BoardSize) {
				board.occ[t.pos.y][t.pos.x] = info.capturedIndex;
				int rsq = t.pos.y * BoardSize + t.pos.x;
				if (rsq >= 0 && rsq < NumTiles) {
					board.occBB.set(rsq);
					int tidx = static_cast<int>(t.type);
					if (tidx >= 0 && tidx < static_cast<int>(board.pieceBB.size())) board.pieceBB[tidx].set(rsq);
					int oppTeamIdx = (t.team == Team::Blue ? 0 : 1);
					board.teamBB[oppTeamIdx].set(rsq);
				}
				// 復元した駒のハッシュを再追加
				XORPieceHash(board, info.capturedIndex);
			}
		}
	}

	// 4) 移動駒を元の位置に戻す（手駒に戻す or 元位置へ）
	if (info.from.x == -1 && info.from.y == -1) {
		// 元は打ち -> 盤上から取り除き、手駒へ戻す
		u.isOnBoard = false;
		UpdateHandHash(board, u.type, teamIdx, +1);
	}
	else {
		// 元の盤上位置に復元
		u.pos = info.from;
		u.isOnBoard = true;
		if (u.pos.x >= 0 && u.pos.x < BoardSize && u.pos.y >= 0 && u.pos.y < BoardSize) {
			board.occ[u.pos.y][u.pos.x] = info.unitIdx;
			int fsq = u.pos.y * BoardSize + u.pos.x;
			if (fsq >= 0 && fsq < NumTiles) {
				board.occBB.set(fsq);
				int tIdx = static_cast<int>(u.type);
				if (tIdx >= 0 && tIdx < static_cast<int>(board.pieceBB.size())) board.pieceBB[tIdx].set(fsq);
				board.teamBB[teamIdx].set(fsq);
			}
			// 元位置のハッシュを再追加
			XORPieceHash(board, info.unitIdx);
		}
	}

	// 5) 成りを戻す（あれば）
	if (info.promoted) {
		u.type = ToDemoted(u.type);
	}

	// 6) 王キャッシュ復元（もし王であれば）
	if (u.type == UnitType::King || u.type == UnitType::King1) {
		board.kingPos[teamIdx] = info.from;
	}
}
void ShidenAI::ExecuteMoveLocal(FastBoard& board, const Candidate& cand) {
	Unit& u = board.units[cand.unitIndex];
	int teamIdx = (board.currentTeam == Team::Blue ? 0 : 1);

	// 1) 移動前の状態を差分XORで取り除く
	if (u.isOnBoard) {
		XORPieceHash(board, cand.unitIndex);
		if (u.pos.y >= 0 && u.pos.y < BoardSize && u.pos.x >= 0 && u.pos.x < BoardSize) {
			int sq = u.pos.y * BoardSize + u.pos.x;
			board.occ[u.pos.y][u.pos.x] = -1;
			if (sq >= 0 && sq < NumTiles) {
				board.occBB.reset(sq);
				int tIdx = static_cast<int>(u.type);
				if (tIdx >= 0 && tIdx < static_cast<int>(board.pieceBB.size())) board.pieceBB[tIdx].reset(sq);
				board.teamBB[teamIdx].reset(sq);
			}
		}
	}
	else {
		UpdateHandHash(board, u.type, teamIdx, -1);
	}

	// 2) 実際の移動／捕獲／打ち
	if (cand.isDrop) {
		u.isOnBoard = true;
		u.pos = cand.target;
		// 到着地に追加
		if (u.pos.y >= 0 && u.pos.y < BoardSize && u.pos.x >= 0 && u.pos.x < BoardSize) {
			board.occ[u.pos.y][u.pos.x] = cand.unitIndex;
			int nsq = u.pos.y * BoardSize + u.pos.x;
			if (nsq >= 0 && nsq < NumTiles) {
				board.occBB.set(nsq);
				int tIdx = static_cast<int>(u.type);
				if (tIdx >= 0 && tIdx < static_cast<int>(board.pieceBB.size())) board.pieceBB[tIdx].set(nsq);
				board.teamBB[teamIdx].set(nsq);
			}
		}
	}
	else {
		// 捕獲があれば先にその駒の位置ハッシュを除く
		for (int i = 0; i < (int)board.units.size(); ++i) {
			Unit& target = board.units[i];
			if (target.isOnBoard && target.pos == cand.target && target.team != board.currentTeam) {
				XORPieceHash(board, i); // remove captured piece from hash
				UnitType demoted = ToDemoted(target.type);
				UpdateHandHash(board, demoted, teamIdx, +1);
				// ビットボード差分
				if (target.pos.x >= 0 && target.pos.x < BoardSize && target.pos.y >= 0 && target.pos.y < BoardSize) {
					int tsq = target.pos.y * BoardSize + target.pos.x;
					if (tsq >= 0 && tsq < NumTiles) {
						board.occBB.reset(tsq);
						int tidx = static_cast<int>(target.type);
						if (tidx >= 0 && tidx < static_cast<int>(board.pieceBB.size())) board.pieceBB[tidx].reset(tsq);
						int oppTeamIdx = (target.team == Team::Blue ? 0 : 1);
						board.teamBB[oppTeamIdx].reset(tsq);
					}
					board.occ[target.pos.y][target.pos.x] = -1;
				}
				target.isOnBoard = false;
				target.team = board.currentTeam;
				target.type = demoted;
				break;
			}
		}
		// 移動する駒の位置変更
		u.pos = cand.target;
		if (cand.promoteDecision) u.type = ToPromoted(u.type);
		if (u.pos.y >= 0 && u.pos.y < BoardSize && u.pos.x >= 0 && u.pos.x < BoardSize) {
			board.occ[u.pos.y][u.pos.x] = cand.unitIndex;
			int nsq = u.pos.y * BoardSize + u.pos.x;
			if (nsq >= 0 && nsq < NumTiles) {
				board.occBB.set(nsq);
				int tIdx = static_cast<int>(u.type);
				if (tIdx >= 0 && tIdx < static_cast<int>(board.pieceBB.size())) board.pieceBB[tIdx].set(nsq);
				board.teamBB[teamIdx].set(nsq);
			}
		}
	}

	// 3) 移動後の駒ハッシュを追加
	if (u.isOnBoard) {
		XORPieceHash(board, cand.unitIndex); // add at new pos
	}

	// 4) 王の位置キャッシュ更新
	if (u.type == UnitType::King || u.type == UnitType::King1) {
		board.kingPos[teamIdx] = u.pos;
	}

	// 5) 手番切替
	XORSide(board);
}




// AttackMap
// 指定したチームの駒からの攻撃マップを構築
static inline bool InBoardXY(int x, int y) noexcept { return x >= 0 && x < BoardSize && y >= 0 && y < BoardSize; }

// 指定ユニットからの利きをビットボードで返す（スライダーは手動でレイ走査）
Bitboard ShidenAI::AttackMap::AttacksFromUnit(const FastBoard& board, const Unit& u) {
	Bitboard att;
	att.clear();
	if (!u.isOnBoard) return att;

	// 最小限の Context を作る（Strategy 側は bitboard / map / moveDirection / selectedUnitPosition を参照する想定）
	Movement::Context ctx;
	ctx.map = board.mapRef;
	ctx.chipDB = board.chipDBRef;
	ctx.unitList = &board.units;
	ctx.selectedUnitPosition = u.pos;
	ctx.moveDirection = (u.team == Team::Blue) ? 1 : -1;

	// FastBoard 側のビットボード群を Context に注入（あれば AttachBitboardsFromExternal を使う）
	// Movement モジュールに該当ヘルパが実装されている前提
	Movement::AttachBitboardsFromExternal(ctx, board.occBB, board.pieceBB.data(), static_cast<int>(board.pieceBB.size()), board.teamBB);
	att = Movement::ComputeMoveBitboard(ctx, u.pos.y, u.pos.x, 1, u.type);
	return att;
	// フォールバック（既存のフォールバック挙動を簡潔に再現）
	auto setIfInBoard = [&](int xx, int yy) {
		if (xx >= 0 && xx < BoardSize && yy >= 0 && yy < BoardSize) att.set(SQ(xx, yy));
		};

	// 既存フォールバックは周囲1マス（王などの未知種に対して）
	for (int dy = -1; dy <= 1; ++dy) {
		for (int dx = -1; dx <= 1; ++dx) {
			if (dx == 0 && dy == 0) continue;
			setIfInBoard(u.pos.x + dx, u.pos.y + dy);
		}
	}
	return att;
}
void ShidenAI::AttackMap::BuildFromBoard(const FastBoard& board, Team enemyTeam)
{
	attackBB.clear();

	const int enemyIdx = (enemyTeam == Team::Blue) ? 0 : 1;

	// すべての駒種について、enemy チームのビットボードを走査して利きを合成
	for (int typeIdx = 0; typeIdx < static_cast<int>(board.pieceBB.size()); ++typeIdx) {
		Bitboard bb = board.pieceBB[typeIdx] & board.teamBB[enemyIdx];
		if (!bb.any()) continue;
		Bitboard tmp = bb;
		int sq;
		while ((sq = pop_lsb(tmp)) != -1) {
			// 最小限の Unit を作って AttacksFromUnit を呼ぶ（スライダー等は AttacksFromUnit 側で扱う）
			Unit u{};
			u.isOnBoard = true;
			u.pos.x = sq % BoardSize;
			u.pos.y = sq / BoardSize;
			u.type = static_cast<UnitType>(typeIdx);
			u.team = enemyTeam;
			attackBB = attackBB | AttacksFromUnit(board, u);
		}
	}
}

// AttackMap: Reset / IsCovered / MarkFromUnit の実装（ビットボード版）
void ShidenAI::AttackMap::Reset() {
	attackBB.clear();
}
bool ShidenAI::AttackMap::IsCovered(const Point& p) const {
	if (p.x < 0 || p.y < 0 || p.x >= BoardSize || p.y >= BoardSize) return false;
	return attackBB.test(SQ(p.x, p.y));
}
void ShidenAI::AttackMap::MarkFromUnit(const Unit& u, FastBoard& fastBoard, Team /*viewTeam*/) {
	// fastBoard を渡してもらえば、そこから occBB 等を参照して利き生成
	Bitboard a = AttacksFromUnit(fastBoard, u);
	attackBB = attackBB | a;
}

// CountEnemyAttacksAroundKing をビットボード版に変更（内部呼び出しは同じ名前のまま）
bool ShidenAI::IsSquareAttacked(FastBoard& board, Point p, Team enemyTeam) {
	if (p.x < 0 || p.y < 0 || p.x >= BoardSize || p.y >= BoardSize) return false;

	// 必要なら sliding table を一度だけ構築
	if (! g_slidingTableBuilt) { g_slidingTable.build_all(); g_slidingTableBuilt = true; }

	const int enemyIdx = (enemyTeam == Team::Blue) ? 0 : 1;
	const int forward = (enemyTeam == Team::Blue) ? -1 : 1;
	const int psq = p.y * BoardSize + p.x;
	const Bitboard occ = board.occBB; // 現在の占有ビットボード

	// 1) 歩による王手 (敵の歩が p の前方にいるか)
	{
		int ay = p.y - forward;
		if (ay >= 0 && ay < BoardSize) {
			int asq = SQ(p.x, ay);
			if (board.teamBB[enemyIdx].test(asq) && board.pieceBB[(int)UnitType::Pawn].test(asq)) return true;
		}
	}

	// 2) 桂馬チェック（2歩前、左右1）
	{
		int ny = p.y - 2 * forward;
		if (ny >= 0 && ny < BoardSize) {
			for (int dx : { -1, 1 }) {
				int nx = p.x + dx;
				if (nx < 0 || nx >= BoardSize) continue;
				int nsq = SQ(nx, ny);
				if (board.teamBB[enemyIdx].test(nsq) &&
					(board.pieceBB[(int)UnitType::Knight].test(nsq) || board.pieceBB[(int)UnitType::Knight1].test(nsq)))
					return true;
			}
		}
	}

	// 3) 近接駒（王・金・成駒・成銀等）の判定：周囲8マスだけ調べる（定数）
	for (int dy = -1; dy <= 1; ++dy) {
		for (int dx = -1; dx <= 1; ++dx) {
			if (dx == 0 && dy == 0) continue;
			int nx = p.x + dx, ny = p.y + dy;
			if (nx < 0 || nx >= BoardSize || ny < 0 || ny >= BoardSize) continue;
			int nsq = SQ(nx, ny);
			// 敵の駒がそこにあるかを高速判定
			if (!board.teamBB[enemyIdx].test(nsq)) continue;
			// occ 配列経由でユニット型を取得（1セルだけなのでコスト小）
			int occIdx = board.occ[ny][nx];
			if (occIdx < 0 || occIdx >= static_cast<int>(board.units.size())) continue;
			const Unit& found = board.units[occIdx];
			// 近接駒のルールを既存と同様に評価
			switch (found.type) {
			case UnitType::King:
			case UnitType::King1:
				return true;
			case UnitType::Gold:
			case UnitType::Pawn1:
			case UnitType::Silver1:
			case UnitType::Knight1:
			case UnitType::Lance1:
				return true;
			case UnitType::Silver:
				// Silver は後ろ直には利かない（敵の forward に依存）
				if (!(dx == 0 && dy == -forward)) return true;
				break;
			case UnitType::Pawn:
			{
				int fwd = (found.team == Team::Blue) ? -1 : 1;
				if (dx == 0 && dy == fwd) return true;
			}
			break;
			default:
				break;
			}
		}
	}

	// 4) スライダー系（飛車・香・角・成駒の付加利き）をビットボードでチェック
	// rook-like attackers (飛車, 龍, 香(直線のみ))
	{
		Bitboard rookAttackMask = g_slidingTable.rook_attacks_from(psq, occ);
		Bitboard enemyRookPositions = (board.pieceBB[(int)UnitType::Rook] | board.pieceBB[(int)UnitType::Rook1] | board.pieceBB[(int)UnitType::Lance]) & board.teamBB[enemyIdx];
		if ((rookAttackMask & enemyRookPositions).any()) return true;
		// 龍の斜め1マス利き（p の近傍にいるか）: 斜め1マスに敵の Rook1 がいれば王手
		for (int dy : { -1, 1 }) for (int dx : { -1, 1 }) {
			int nx = p.x + dx, ny = p.y + dy;
			if (nx < 0 || nx >= BoardSize || ny < 0 || ny >= BoardSize) continue;
			int nsq = SQ(nx, ny);
			if (board.teamBB[enemyIdx].test(nsq) && board.pieceBB[(int)UnitType::Rook1].test(nsq)) return true;
		}
	}

	// bishop-like attackers (角, 馬)
	{
		Bitboard bishopAttackMask = g_slidingTable.bishop_attacks_from(psq, occ);
		Bitboard enemyBishopPositions = (board.pieceBB[(int)UnitType::Bishop] | board.pieceBB[(int)UnitType::Bishop1]) & board.teamBB[enemyIdx];
		if ((bishopAttackMask & enemyBishopPositions).any()) return true;
		// 馬の orthogonal 1 マス利き
		for (const auto& d : std::array<std::pair<int, int>, 4>{ { {1,0},{-1,0},{0,1},{0,-1} } }) {
			int nx = p.x + d.first, ny = p.y + d.second;
			if (nx < 0 || nx >= BoardSize || ny < 0 || ny >= BoardSize) continue;
			int nsq = SQ(nx, ny);
			if (board.teamBB[enemyIdx].test(nsq) && board.pieceBB[(int)UnitType::Bishop1].test(nsq)) return true;
		}
	}

	return false;
}




// pinned 駒のインデックスリストを outPinnedIndices に格納
int ShidenAI::GetUnitIndexAt(const FastBoard& board, int x, int y) {
	if (x < 0 || x >= BoardSize || y < 0 || y >= BoardSize) return -1;
	for (int i = 0; i < static_cast<int>(board.units.size()); ++i) {
		const auto& u = board.units[i];
		if (!u.isOnBoard) continue;
		if (u.pos.x == x && u.pos.y == y) return i;
	}
	return -1;
}
bool ShidenAI::IsSquareOccupied(const Array<Unit>& units, int x, int y) {
	if (x < 0 || x >= BoardSize || y < 0 || y >= BoardSize) return false;
	for (const auto& u : units) {
		if (!u.isOnBoard) continue;
		if (u.pos.x == x && u.pos.y == y) return true;
	}
	return false;
}

// --- ComputePinnedPieces をビットボード版に置換 ---
void ShidenAI::ComputePinnedPieces(const FastBoard& board, Array<int>& outPinnedIndices, Array<Point>& outAttackerPos) {
	outPinnedIndices.clear(); outAttackerPos.clear();
	Point kingPos = FindKing(board.units, board.currentTeam);
	if (kingPos.x < 0) return;

	// 8方向をレイで探索。occBB を使って最初の駒を検出し、続けて次の駒（敵）を検出する
	const int dirs[8][2] = { {1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1} };
	for (int di = 0; di < 8; ++di) {
		int dx = dirs[di][0], dy = dirs[di][1];
		int firstUnitIdx = -1;
		int attackerSqX = -1, attackerSqY = -1;

		for (int step = 1; step < BoardSize; ++step) {
			int nx = kingPos.x + dx * step;
			int ny = kingPos.y + dy * step;
			if (nx < 0 || nx >= BoardSize || ny < 0 || ny >= BoardSize) break;
			const int nsq = SQ(nx, ny);
			if (!board.occBB.test(nsq)) continue; // 空なら次へ

			// ここに駒がある。ユニットインデックスを取得
			int foundIdx = GetUnitIndexAt(board, nx, ny);
			if (foundIdx < 0) break; // 整合しないならこの方向は終わり

			const Unit& found = board.units[foundIdx];
			if (firstUnitIdx == -1) {
				// 最初に見つかった駒（自駒であるか確認）
				if (found.team != board.currentTeam) break; // 最初が敵ならピンはあり得ない
				firstUnitIdx = foundIdx;
				continue;
			}
			// 2つ目の駒が敵でスライダーなら pinned
			if (found.team != board.currentTeam) {
				const UnitType t = found.type;
				bool isSlider = false;
				// 直線方向なら飛車系、斜め方向なら角系
				if (dx == 0 || dy == 0) {
					if (t == UnitType::Rook || t == UnitType::Rook1) isSlider = true;
					if ((dx == 0 && dy != 0) && (t == UnitType::Lance)) isSlider = true;
				}
				else {
					if (t == UnitType::Bishop || t == UnitType::Bishop1) isSlider = true;
				}
				if (isSlider) {
					outPinnedIndices << firstUnitIdx;
					outAttackerPos << board.units[foundIdx].pos;
				}
			}
			break;
		}
	}
}


int ShidenAI::StaticExchangeEvaluation(const FastBoard& board, int tx, int ty, int attackerIndex)
{
	// 範囲チェック / victim の存在チェック
	if (tx < 0 || tx >= BoardSize || ty < 0 || ty >= BoardSize) return 0;
	const int tsq = SQ(tx, ty);
	const int n = static_cast<int>(board.units.size());
	if (attackerIndex < 0 || attackerIndex >= n) return 0;
	int victimIndex = board.occ[ty][tx];
	if (victimIndex < 0 || victimIndex >= n) return 0;

	// ローカルなビットボードコピー（差分更新用）
	Bitboard localOcc = board.occBB;
	std::array<Bitboard, UnitTypeArraySize> localPieceBB = board.pieceBB;
	Bitboard localTeamBB[2] = { board.teamBB[0], board.teamBB[1] };

	// victim を除去（ターゲットは一旦空にする）
	UnitType victimType = board.units[victimIndex].type;
	int victimTypeIdx = static_cast<int>(victimType);
	int victimTeamIdx = (board.units[victimIndex].team == Team::Blue) ? 0 : 1;
	if (tsq >= 0 && tsq < BoardSize * BoardSize) {
		localOcc.reset(tsq);
		if (victimTypeIdx >= 0 && victimTypeIdx < static_cast<int>(localPieceBB.size())) localPieceBB[victimTypeIdx].reset(tsq);
		localTeamBB[victimTeamIdx].reset(tsq);
	}

	// attacker の確認
	const Unit& attackerUnit = board.units[attackerIndex];
	if (!attackerUnit.isOnBoard) return 0;
	int asq = attackerUnit.pos.y * BoardSize + attackerUnit.pos.x;
	if (asq < 0 || asq >= BoardSize * BoardSize) return 0;

	// V: 固定長配列に変更（ヒープ確保を排除）
	int V[KILLER_TABLE_PLY];
	int vlen = 0;
	auto pushV = [&](int x) {
		if (vlen < static_cast<int>(std::size(V))) V[vlen++] = x;
		};

	// 初期: victim, attacker
	pushV(PieceValue(victimType, ShidenAIConfig::PieceValue::TOTAL_VAL));
	pushV(PieceValue(attackerUnit.type, ShidenAIConfig::PieceValue::TOTAL_VAL));

	// attacker を到着地に「移動」(ローカル差分)
	{
		int atkTypeIdx = static_cast<int>(attackerUnit.type);
		int atkTeamIdx = (attackerUnit.team == Team::Blue) ? 0 : 1;
		localOcc.reset(asq);
		if (atkTypeIdx >= 0 && atkTypeIdx < static_cast<int>(localPieceBB.size())) localPieceBB[atkTypeIdx].reset(asq);
		localTeamBB[atkTeamIdx].reset(asq);

		localOcc.set(tsq);
		if (atkTypeIdx >= 0 && atkTypeIdx < static_cast<int>(localPieceBB.size())) localPieceBB[atkTypeIdx].set(tsq);
		localTeamBB[atkTeamIdx].set(tsq);
	}

	Team sideToMove = (attackerUnit.team == Team::Blue) ? Team::Red : Team::Blue;

	// 駒種を価値順に一度だけ準備（static）
	static std::vector<int> orderedTypes;
	if (orderedTypes.empty()) {
		orderedTypes.reserve(localPieceBB.size());
		for (int t = 0; t < static_cast<int>(localPieceBB.size()); ++t) orderedTypes.push_back(t);
		std::sort(orderedTypes.begin(), orderedTypes.end(), [](int a, int b) {
			return PieceValue(static_cast<UnitType>(a), ShidenAIConfig::PieceValue::TOTAL_VAL) < PieceValue(static_cast<UnitType>(b), ShidenAIConfig::PieceValue::TOTAL_VAL);
		});
	}

	// 取り合いシミュレーション（固定配列 V を使って実施）
	while (true)
	{
		Bitboard attackersMask = Movement::BuildAttackersMask(sideToMove, tsq, localOcc, localPieceBB, localTeamBB, g_slidingTable) & localOcc;

		// 最小価値の攻撃者を駒種ごとに探す（価値昇順）
		int foundSq = -1;
		int foundTypeIdx = -1;
		for (int typeIdx : orderedTypes) {
			Bitboard cand = attackersMask & localPieceBB[typeIdx] & localTeamBB[(sideToMove == Team::Blue) ? 0 : 1];
			if (!cand.any()) continue;
			int sq = lsb_index(cand);
			if (sq != -1) { foundSq = sq; foundTypeIdx = typeIdx; break; }
		}

		if (foundSq == -1) break; // これ以上攻撃者なし

		// 現在のターゲット上の駒（もしあれば）を除去（捕獲）
		for (int t = 0; t < static_cast<int>(localPieceBB.size()); ++t) {
			if (localPieceBB[t].test(tsq)) localPieceBB[t].reset(tsq);
		}
		localTeamBB[0].reset(tsq);
		localTeamBB[1].reset(tsq);
		localOcc.reset(tsq);

		// best attacker を移動（foundSq -> tsq）
		localOcc.reset(foundSq);
		if (foundTypeIdx >= 0 && foundTypeIdx < static_cast<int>(localPieceBB.size())) localPieceBB[foundTypeIdx].reset(foundSq);
		localTeamBB[(sideToMove == Team::Blue) ? 0 : 1].reset(foundSq);

		localOcc.set(tsq);
		if (foundTypeIdx >= 0 && foundTypeIdx < static_cast<int>(localPieceBB.size())) localPieceBB[foundTypeIdx].set(tsq);
		localTeamBB[(sideToMove == Team::Blue) ? 0 : 1].set(tsq);

		// 価値列に追加（この攻撃者が取られた対象として列に加わる）
		pushV(PieceValue(static_cast<UnitType>(foundTypeIdx), ShidenAIConfig::PieceValue::TOTAL_VAL));

		// 手番交代
		sideToMove = (sideToMove == Team::Blue) ? Team::Red : Team::Blue;
	}

	if (vlen == 0) return 0;
	int m = vlen;
	int gain[KILLER_TABLE_PLY];
	gain[0] = V[0];
	for (int i = 1; i < m; ++i) gain[i] = V[i] - gain[i - 1];
	for (int i = m - 2; i >= 0; --i) gain[i] = std::max(gain[i], -gain[i + 1]);
	return gain[0];
}




// a--b--p が同一直線上にあるか
bool ShidenAI::IsOnSameLine(const Point& a, const Point& b, const Point& p) const {
	int dx1 = b.x - a.x;
	int dy1 = b.y - a.y;
	int dx2 = p.x - a.x;
	int dy2 = p.y - a.y;
	// cross product == 0 なら同一直線
	return (dx1 * dy2 - dy1 * dx2) == 0;
}

// 駒生成
void ShidenAI::GenerateMoves(std::vector<Candidate>& out, const FastBoard& board, bool capturesOnly) {
	out.clear();

	// 自分の王位置
	Point kingPos = FindKing(board.units, board.currentTeam);

	// 王がいない（例：既に取られている等）は安全に何も生成せず戻る
	if (kingPos.x < 0 || kingPos.y < 0) {
		return;
	}

	// ピン検出
	Array<int> pinnedIdx; pinnedIdx.clear();
	Array<Point> pinnedAttackerPos; pinnedAttackerPos.clear();
	ComputePinnedPieces(board, pinnedIdx, pinnedAttackerPos);

	// チェッカー検出（王に利いている駒）
	Array<Point> checkers; checkers.clear();
	{
		if (!g_slidingTableBuilt) { g_slidingTable.build_all(); g_slidingTableBuilt = true; }
		const int selfKingX = kingPos.x, selfKingY = kingPos.y;
		const int kingSq = selfKingY * BoardSize + selfKingX;
		const Bitboard occ = board.occBB;
		Team enemy = (board.currentTeam == Team::Blue ? Team::Red : Team::Blue);
		const int enemyIdx = (enemy == Team::Blue) ? 0 : 1;

		// 飛車系スライダー
		{
			Bitboard atk = g_slidingTable.rook_attacks_from(kingSq, occ);
			Bitboard candidates = atk & (board.pieceBB[(int)UnitType::Rook] | board.pieceBB[(int)UnitType::Rook1] | board.pieceBB[(int)UnitType::Lance]) & board.teamBB[enemyIdx];
			for (int sq = 0; sq < BoardSize * BoardSize; ++sq) {
				if (!candidates.test(sq)) continue;
				int x = sq % BoardSize, y = sq / BoardSize;
				int idx = board.occ[y][x];
				if (idx >= 0 && idx < static_cast<int>(board.units.size())) checkers << board.units[idx].pos;
			}
			for (int dy = -1; dy <= 1; ++dy) for (int dx = -1; dx <= 1; ++dx) {
				if (std::abs(dx) != 1 || std::abs(dy) != 1) continue;
				int nx = selfKingX + dx, ny = selfKingY + dy;
				if (nx < 0 || nx >= BoardSize || ny < 0 || ny >= BoardSize) continue;
				int nsq = SQ(nx, ny);
				if (board.teamBB[enemyIdx].test(nsq) && board.pieceBB[(int)UnitType::Rook1].test(nsq)) checkers << Point{ nx, ny };
			}
		}
		// 角系スライダー
		{
			Bitboard atk = g_slidingTable.bishop_attacks_from(kingSq, occ);
			Bitboard candidates = atk & (board.pieceBB[(int)UnitType::Bishop] | board.pieceBB[(int)UnitType::Bishop1]) & board.teamBB[enemyIdx];
			for (int sq = 0; sq < BoardSize * BoardSize; ++sq) {
				if (!candidates.test(sq)) continue;
				int x = sq % BoardSize, y = sq / BoardSize;
				int idx = board.occ[y][x];
				if (idx >= 0 && idx < static_cast<int>(board.units.size())) checkers << board.units[idx].pos;
			}
			for (const auto& d : std::array<std::pair<int, int>, 4>{ { {1,0},{-1,0},{0,1},{0,-1} } }) {
				int nx = selfKingX + d.first, ny = selfKingY + d.second;
				if (nx < 0 || nx >= BoardSize || ny < 0 || ny >= BoardSize) continue;
				int nsq = SQ(nx, ny);
				if (board.teamBB[enemyIdx].test(nsq) && board.pieceBB[(int)UnitType::Bishop1].test(nsq)) checkers << Point{ nx, ny };
			}
		}
		// 桂馬チェック
		{
			int forwardEnemy = (enemy == Team::Blue) ? -1 : 1;
			int ny = selfKingY + 2 * forwardEnemy;
			if (ny >= 0 && ny < BoardSize) {
				for (int dx : { -1, 1 }) {
					int nx = selfKingX + dx;
					if (nx < 0 || nx >= BoardSize) continue;
					int nsq = SQ(nx, ny);
					if (board.teamBB[enemyIdx].test(nsq) && (board.pieceBB[(int)UnitType::Knight].test(nsq) || board.pieceBB[(int)UnitType::Knight1].test(nsq))) {
						checkers << Point{ nx, ny };
					}
				}
			}
		}
		// 隣接近接駒チェック
		{
			for (int dy = -1; dy <= 1; ++dy) for (int dx = -1; dx <= 1; ++dx) {
				if (dx == 0 && dy == 0) continue;
				int nx = selfKingX + dx, ny = selfKingY + dy;
				if (nx < 0 || nx >= BoardSize || ny < 0 || ny >= BoardSize) continue;
				int nsq = SQ(nx, ny);
				if (!board.teamBB[enemyIdx].test(nsq)) continue;
				int occIdx = board.occ[ny][nx];
				if (occIdx < 0 || occIdx >= static_cast<int>(board.units.size())) continue;
				const Unit& u = board.units[occIdx];
				bool isChecker = false;
				if (u.type == UnitType::Pawn) {
					int fwd = (u.team == Team::Blue) ? -1 : 1;
					if (dx == 0 && dy == fwd) isChecker = true;
				}
				else {
					if (u.type == UnitType::King || u.type == UnitType::King1 ||
						u.type == UnitType::Gold || u.type == UnitType::Pawn1 ||
						u.type == UnitType::Silver1 || u.type == UnitType::Knight1 || u.type == UnitType::Lance1) isChecker = true;
					if (u.type == UnitType::Silver) {
						int forwardDir = (u.team == Team::Blue) ? -1 : 1;
						if (!(dx == 0 && dy == -forwardDir)) isChecker = true;
					}
				}
				if (isChecker) checkers << Point{ nx, ny };
			}
		}
	}

	const bool inCheck = !checkers.empty();
	const bool doubleCheck = (checkers.size() >= 2);
	Team enemyTeam = (board.currentTeam == Team::Blue ? Team::Red : Team::Blue);

	auto IsBetweenOnLine = [&](const Point& a, const Point& b, const Point& target) -> bool {
		int dx1 = b.x - a.x, dy1 = b.y - a.y, dx2 = target.x - a.x, dy2 = target.y - a.y;
		if (dx1 * dy2 - dy1 * dx2 != 0) return false;
		if (std::abs(dx1) >= std::abs(dy1)) {
			int minx = std::min(a.x, b.x), maxx = std::max(a.x, b.x);
			return (target.x >= minx && target.x <= maxx);
		}
		else {
			int miny = std::min(a.y, b.y), maxy = std::max(a.y, b.y);
			return (target.y >= miny && target.y <= maxy);
		}
		};

	int mr[BoardSize][BoardSize];
	Movement::Context ctx;
	ctx.moveRange = mr; ctx.map = board.mapRef; ctx.chipDB = board.chipDBRef; ctx.unitList = &board.units;

	// 合法性チェック用: シミュレートして自玉が安全か確認するヘルパ
	auto push_if_legal = [&](const Candidate& c) {
		FastBoard tmp = board; // コピー
		MoveInfo info;
		DoMove(tmp, c, info); // tmp を進める
		Point myKingAfter = FindKing(tmp.units, board.currentTeam); // board.currentTeam は指し手の側
		Team opp = (board.currentTeam == Team::Blue ? Team::Red : Team::Blue);

		if (myKingAfter.x == -1) return; // 自玉が消えるのは違法
		if (IsSquareAttacked(tmp, myKingAfter, opp)) return; // 王手放置なら除外
		out.push_back(c);
		};

	for (int i = 0; i < static_cast<int>(board.units.size()); ++i) {
		const Unit& u = board.units[i];
		if (u.team != board.currentTeam) continue;
		if (doubleCheck && u.type != UnitType::King && u.isOnBoard) continue;

		// 持ち駒
		if (!u.isOnBoard) {
			if (capturesOnly) continue;
			int dr[BoardSize][BoardSize];
			Drop::ComputeDropMoveRange(dr, board.units, u.type, board.currentTeam);
			for (int y = 0; y < BoardSize; ++y) for (int x = 0; x < BoardSize; ++x) {
				if (dr[y][x] < 0) continue;
				if (inCheck && !doubleCheck) {
					bool allowed = false;
					Point checkerPos = checkers[0];
					if (x == checkerPos.x && y == checkerPos.y) allowed = true;
					if (!allowed && IsBetweenOnLine(kingPos, checkerPos, Point{ x,y })) allowed = true;
					if (!allowed) continue;
				}
				Candidate c = { i, {x,y}, {-1,-1}, true, false, 0 };
				push_if_legal(c);
			}
			continue;
		}

		// 盤上駒: move range
		ctx.selectedUnitPosition = u.pos;
		ctx.moveDirection = (board.currentTeam == Team::Blue ? 1 : -1);
		for (auto& row : mr) for (auto& cell : row) cell = -1;

		const auto& info = board.unitDBRef->at(u.type);
		const int cost = board.chipDBRef->at(board.mapRef->at(u.pos.y, u.pos.x)).cost;
		Movement::ComputeMoveRangeImpl(ctx, u.pos.y, u.pos.x, info.speed + cost, u.type, 0);

		Movement::ClearMoveRangeForCurrentTeam(mr, board.units, u.team);

		int pinIndexInArray = -1;
		Point pinAttackerPos{ -1, -1 };
		for (int pi = 0; pi < static_cast<int>(pinnedIdx.size()); ++pi) {
			if (pinnedIdx[pi] == i) { pinIndexInArray = pi; pinAttackerPos = pinnedAttackerPos[pi]; break; }
		}

		for (int y = 0; y < BoardSize; ++y) {
			for (int x = 0; x < BoardSize; ++x) {
				if (mr[y][x] < 0) continue;
				bool isCapture = false;
				int occIdx = board.occ[y][x];
				if (occIdx != -1 && occIdx >= 0 && occIdx < static_cast<int>(board.units.size()) && board.units[occIdx].team != u.team) isCapture = true;
				if (capturesOnly && !isCapture) continue;
				Point target{ x, y };

				if (pinIndexInArray != -1 && u.type != UnitType::King) {
					if (!IsOnSameLine(kingPos, pinAttackerPos, target)) continue;
				}
				if (inCheck) {
					if (doubleCheck) {
						if (u.type != UnitType::King) continue;
					}
					else {
						if (u.type != UnitType::King) {
							Point checkerPos = checkers[0];
							bool ok = false;
							if (isCapture && checkerPos.x == x && checkerPos.y == y) ok = true;
							if (!ok && IsBetweenOnLine(kingPos, checkerPos, target)) ok = true;
							if (!ok) continue;
						}
					}
				}

				// 王の移動はさらに安全確認を行う（既存のコピーなし判定は残すが、最終は push_if_legal で確認）
				if (u.type == UnitType::King || u.type == UnitType::King1) {
					int kingIdx = i;
					Point oldPos = u.pos;
					int enemyIdxLocal = (board.currentTeam == Team::Blue ? 1 : 0);
					int selfIdx = (board.currentTeam == Team::Blue ? 0 : 1);

					const_cast<FastBoard&>(board).occ[oldPos.y][oldPos.x] = -1;
					int targetOccIdx = board.occ[target.y][target.x];
					const_cast<FastBoard&>(board).occ[target.y][target.x] = kingIdx;
					const_cast<FastBoard&>(board).kingPos[selfIdx] = target;

					bool safe = !IsSquareAttacked(const_cast<FastBoard&>(board), target, enemyTeam);

					const_cast<FastBoard&>(board).occ[target.y][target.x] = targetOccIdx;
					const_cast<FastBoard&>(board).occ[oldPos.y][oldPos.x] = kingIdx;
					const_cast<FastBoard&>(board).kingPos[selfIdx] = oldPos;

					if (!safe) continue;
				}

				Candidate c = { i, target, u.pos, false, false, 0 };
				// 成り候補
				bool canProm = ToPromoted(u.type) != u.type;
				bool inZone = (u.team == Team::Blue ? y <= kBluePromoteLine : y >= BoardSize - kRedPromoteLine);
				if (u.type != UnitType::King && canProm && (inZone || (u.team == Team::Blue ? u.pos.y <= kBluePromoteLine : u.pos.y >= BoardSize - kRedPromoteLine))) {
					Candidate cprom = c; cprom.promoteDecision = true; push_if_legal(cprom);
				}
				// 非成
				if (!IsAutoPromotionRequired(u.type, u.team, y)) {
					c.promoteDecision = false;
					push_if_legal(c);
				}
			}
		}
	}
}
void ShidenAI::GenerateTacticalMoves(std::vector<Candidate>& out, const FastBoard& board, Point lastCapturedPos) {
	out.clear();

	// 1. 自玉の王手判定
	Point myKing = board.kingPos[(board.currentTeam == Team::Blue ? 0 : 1)];
	Team enemyTeam = (board.currentTeam == Team::Blue ? Team::Red : Team::Blue);
	bool inCheck = IsSquareAttacked(const_cast<FastBoard&>(board), myKing, enemyTeam);

	// 2. 王手されている場合は、逃げるための全合法手を生成（王手回避は「タクティカル」な行動）
	if (inCheck) {
		GenerateMoves(out, board, false); // 全合法手
		return;
	}

	// --- 事前に敵の利きマップを構築（EvaluateDetailed と同様） ---
	AttackMap enemyByAttack; enemyByAttack.Reset();
	enemyByAttack.BuildFromBoard(board, enemyTeam);

	// 3. 王手されていない場合：盤上の駒取り／成りに加え、持ち駒の「王手になる打ち」を追加
	for (int i = 0; i < (int)board.units.size(); ++i) {
		const Unit& u = board.units[i];
		if (u.team != board.currentTeam) continue;

		// --- 手駒 (打ち) の処理: 「打って王手になる手」を追加 ---
		if (!u.isOnBoard) {
			// 打ち範囲を列挙
			int dr[BoardSize][BoardSize];
			Drop::ComputeDropMoveRange(dr, board.units, u.type, board.currentTeam);

			// --- 追加: 直前に駒を取られた位置への取り返し（recapture） ---
			if (lastCapturedPos.x >= 0 && lastCapturedPos.y >= 0 &&
				lastCapturedPos.x < BoardSize && lastCapturedPos.y < BoardSize) {
				int lx = lastCapturedPos.x, ly = lastCapturedPos.y;
				if (dr[ly][lx] >= 0) {
					// 実際に置いて自玉が安全かを簡易チェック（DoMove + IsSquareAttacked）
					Candidate rc = { i, { lx, ly }, { -1, -1 }, true, false, ShidenAIConfig::MoveOrdering::MOVE_ORDER_CAPTURE_SCORE - ShidenAIConfig::MoveOrdering::BECOMING_CAPTURE_BONUS };
					// シミュレーションして自玉が安全か確認
					FastBoard tmp = board;
					MoveInfo info;
					DoMove(tmp, rc, info);
					Point myKingAfter = FindKing(tmp.units, board.currentTeam);
					Team opp = (board.currentTeam == Team::Blue ? Team::Red : Team::Blue);
					if (myKingAfter.x != -1 && !IsSquareAttacked(tmp, myKingAfter, opp)) {
						out.push_back(rc);
					}
					
					// UndoMove は不要（tmp はコピー）
				}
			}

			for (int y = 0; y < BoardSize; ++y) {
				for (int x = 0; x < BoardSize; ++x) {
					if (dr[y][x] < 0) continue;
					// 王手になる打ちだけ採用（従来の挙動）
					Candidate c = { i, {x, y}, {-1, -1}, true, false, 0 };
					if (GivesCheckAfterMove(*this, const_cast<FastBoard&>(board), c, board.currentTeam)) {
						out.push_back(c);
					}
				}
			}
			continue;
		}

		// 盤上駒: まず「その駒が敵に利かれているか」チェックして、必要なら逃げ手を増やす
		if (enemyByAttack.IsCovered(u.pos)) {
			Team attackersTeam = enemyTeam;
			int attackerIdx = FindLeastValuableAttacker(board, u.pos.x, u.pos.y, attackersTeam);
			if (attackerIdx != -1) {
				int see = StaticExchangeEvaluation(board, u.pos.x, u.pos.y, attackerIdx);
				// SEE が負（駒損）なら「逃げる手」を優先候補として追加
				if (see < 0) {
					// 移動可能範囲を計算（軽量）して、敵の現在の利きが届かない場所を候補に追加
					int mr[BoardSize][BoardSize];
					Movement::Context ctx; ctx.moveRange = mr; ctx.map = board.mapRef; ctx.chipDB = board.chipDBRef; ctx.unitList = &board.units;
					ctx.selectedUnitPosition = u.pos; ctx.moveDirection = (board.currentTeam == Team::Blue ? 1 : -1);
					for (auto& row : mr) for (auto& cell : row) cell = -1;
					const auto& info = board.unitDBRef->at(u.type);
					const int cost = board.chipDBRef->at(board.mapRef->at(u.pos.y, u.pos.x)).cost;
					Movement::ComputeMoveRangeImpl(ctx, u.pos.y, u.pos.x, info.speed + cost, u.type, 0);
					Movement::ClearMoveRangeForCurrentTeam(mr, board.units, u.team);

					const int escapeScore = ShidenAIConfig::MoveOrdering::MOVE_ORDER_CAPTURE_SCORE - ShidenAIConfig::MoveOrdering::POSSIBLE_HAND_BONUSES;
					for (int y = 0; y < BoardSize; ++y) for (int x = 0; x < BoardSize; ++x) {
						if (mr[y][x] < 0) continue;
						// 味方がいるマスは除外（通常の移動制約と一致）
						int occIdx = board.occ[y][x];
						if (occIdx != -1 && board.units[occIdx].team == board.currentTeam) continue;
						// 目標が現在の敵の利きマップで守られていない（より安全な場所）を優先
						if (!enemyByAttack.IsCovered(Point{ x, y })) {
							Candidate c = { i, {x, y}, u.pos, false, false, escapeScore };
							out.push_back(c);
						}
					}
					// 逃げ手を追加したら、盤上の通常タクティカル判定も続けて行う（重複候補は後でソートで整理）
				}
			}
		}

		// --- 盤上駒: 駒取りと成りのみを抽出（既存ロジックを継承） ---
		int mr[BoardSize][BoardSize];
		Movement::Context ctx = { mr, board.mapRef, board.chipDBRef, &board.units, u.pos, (board.currentTeam == Team::Blue ? 1 : -1) };
		Movement::ComputeMoveRange(mr);
		const auto& info = board.unitDBRef->at(u.type);
		const int cost = board.chipDBRef->at(board.mapRef->at(u.pos.y, u.pos.x)).cost;
		Movement::ComputeMoveRangeImpl(ctx, u.pos.y, u.pos.x, info.speed + cost, u.type, 0);

		for (int y = 0; y < BoardSize; ++y) {
			for (int x = 0; x < BoardSize; ++x) {
				if (mr[y][x] < 0) continue;

				int targetIdx = board.occ[y][x];
				bool isCapture = (targetIdx != -1 && board.units[targetIdx].team != board.currentTeam);

				// タクティカルな手として採用する条件：
				bool isPromotionZone = (u.team == Team::Blue ? y <= kBluePromoteLine : y >= BoardSize - kRedPromoteLine);
				bool canPromote = (ToPromoted(u.type) != u.type);
				bool isMajorPromotion = (u.type == UnitType::Pawn ||
					u.type == UnitType::Rook ||
					u.type == UnitType::Bishop ||
					u.type == UnitType::Silver ||
					u.type == UnitType::Lance);

				if (isCapture || (isPromotionZone && canPromote && isMajorPromotion)) {
					Candidate c = { i, {x, y}, u.pos, false, (isPromotionZone && canPromote), 0 };

					// スコア付け (MVV-LVA)
					if (isCapture) {
						c.score = PieceValue(board.units[targetIdx].type, ShidenAIConfig::PieceValue::TOTAL_VAL) - PieceValue(u.type, ShidenAIConfig::PieceValue::TOTAL_VAL);
					}
					else {
						c.score = ShidenAIConfig::Evaluation::PROMOTION_ONLY_BONUS; // 成りのみの評価
					}
					out.push_back(c);
				}
			}
		}
	}
}

int ShidenAI::StaticExchangeEvaluationForDrop(const FastBoard& board, int tx, int ty, UnitType dropType, Team dropTeam)
{
	// 範囲チェック
	if (tx < 0 || tx >= BoardSize || ty < 0 || ty >= BoardSize) return 0;
	const int tsq = SQ(tx, ty);

	// ローカルなビットボードコピー（差分更新用）
	Bitboard localOcc = board.occBB;
	std::array<Bitboard, UnitTypeArraySize> localPieceBB = board.pieceBB;
	Bitboard localTeamBB[2] = { board.teamBB[0], board.teamBB[1] };

	// 打ち駒を到着地にセット（最初は盤上に居るものとして扱う）
	int dropTypeIdx = static_cast<int>(dropType);
	int dropTeamIdx = (dropTeam == Team::Blue) ? 0 : 1;
	if (tsq >= 0 && tsq < BoardSize * BoardSize) {
		localOcc.set(tsq);
		if (dropTypeIdx >= 0 && dropTypeIdx < static_cast<int>(localPieceBB.size())) localPieceBB[dropTypeIdx].set(tsq);
		localTeamBB[dropTeamIdx].set(tsq);
	}

	// V: 固定長配列
	int V[KILLER_TABLE_PLY];
	int vlen = 0;
	auto pushV = [&](int x) { if (vlen < static_cast<int>(std::size(V))) V[vlen++] = x; };

	// 初手は打ち駒の価値（打ち駒が取られる側）
	pushV(PieceValue(dropType, ShidenAIConfig::PieceValue::TOTAL_VAL));

	// sideToMove は相手（打ちの後に取る側）
	Team sideToMove = (dropTeam == Team::Blue) ? Team::Red : Team::Blue;

	// 駒価昇順 type 配列の準備（static）
	static std::vector<int> orderedTypes;
	if (orderedTypes.empty()) {
		orderedTypes.reserve(localPieceBB.size());
		for (int t = 0; t < static_cast<int>(localPieceBB.size()); ++t) orderedTypes.push_back(t);
		std::sort(orderedTypes.begin(), orderedTypes.end(), [](int a, int b) {
			return PieceValue(static_cast<UnitType>(a), ShidenAIConfig::PieceValue::TOTAL_VAL) < PieceValue(static_cast<UnitType>(b), ShidenAIConfig::PieceValue::TOTAL_VAL);
		});
	}
	
	// 取り合いシミュレーション（固定配列）
	while (true)
	{
		Bitboard attackersMask = Movement::BuildAttackersMask(sideToMove, tsq, localOcc, localPieceBB, localTeamBB, g_slidingTable) & localOcc;

		// 最小価値の攻撃者を駒種ごとに探す（価値昇順）
		int foundSq = -1;
		int foundTypeIdx = -1;
		for (int typeIdx : orderedTypes) {
			Bitboard cand = attackersMask & localPieceBB[typeIdx] & localTeamBB[(sideToMove == Team::Blue) ? 0 : 1];
			if (!cand.any()) continue;
			int sq = lsb_index(cand);
			if (sq != -1) { foundSq = sq; foundTypeIdx = typeIdx; break; }
		}

		if (foundSq == -1) break;

		// 現在のターゲット上の駒を除去
		for (int t = 0; t < static_cast<int>(localPieceBB.size()); ++t) {
			if (localPieceBB[t].test(tsq)) localPieceBB[t].reset(tsq);
		}
		localTeamBB[0].reset(tsq);
		localTeamBB[1].reset(tsq);
		localOcc.reset(tsq);

		// best attacker を移動（foundSq -> tsq）
		localOcc.reset(foundSq);
		if (foundTypeIdx >= 0 && foundTypeIdx < static_cast<int>(localPieceBB.size())) localPieceBB[foundTypeIdx].reset(foundSq);
		localTeamBB[(sideToMove == Team::Blue) ? 0 : 1].reset(foundSq);

		localOcc.set(tsq);
		if (foundTypeIdx >= 0 && foundTypeIdx < static_cast<int>(localPieceBB.size())) localPieceBB[foundTypeIdx].set(tsq);
		localTeamBB[(sideToMove == Team::Blue) ? 0 : 1].set(tsq);

		// 価値列に追加
		pushV(PieceValue(static_cast<UnitType>(foundTypeIdx), ShidenAIConfig::PieceValue::TOTAL_VAL));

		sideToMove = (sideToMove == Team::Blue) ? Team::Red : Team::Blue;
	}

	if (vlen == 0) return 0;
	int m = vlen;
	int gain[KILLER_TABLE_PLY];
	gain[0] = V[0];
	for (int i = 1; i < m; ++i) gain[i] = V[i] - gain[i - 1];
	for (int i = m - 2; i >= 0; --i) gain[i] = std::max(gain[i], -gain[i + 1]);

	// gain[0] は「相手（取る側）」の利益 => 打ち側視点に反転
	return -gain[0];
}

// SortMoves を SEE を用いるように改良
void  ShidenAI::SortMoves(std::vector<Candidate>& moves, const FastBoard& board, int depth, int ttIdx, Point ttTarget) {
	// まず王手状態とチェッカー（利いている敵駒）のリストを軽量に収集
	Point kingPos = FindKing(board.units, board.currentTeam);
	Team enemy = (board.currentTeam == Team::Blue ? Team::Red : Team::Blue);
	bool inCheck = false;
	Array<Point> checkers; checkers.clear();
	if (kingPos.x >= 0) {
		for (const auto& u : board.units) {
			if (!u.isOnBoard) continue;
			if (u.team != enemy) continue;
			Bitboard att = AttackMap().AttacksFromUnit(board, u);
			if (att.test(SQ(kingPos.x, kingPos.y))) {
				inCheck = true;
				checkers << u.pos;
			}
		}
	}

	for (auto& m : moves) {
		// Transposition table の最善手は高優先（ただし信頼できる場合のみ）
		if (m.unitIndex == ttIdx && m.target == ttTarget) {
			TTEntry* ttEntry = LookupTT(board.hash);
			// 信頼条件: TT エントリが存在し、同一手で、保存深さが現在の探索深さに対して十分深い
			const bool ttReliable = (ttEntry != nullptr) && (ttEntry->bestUnitIndex == ttIdx)
				&& (ttEntry->bestTarget == ttTarget) && (ttEntry->depth >= depth - 1);
			if (ttReliable) {
				m.score = ShidenAIConfig::MoveOrdering::MOVE_ORDER_TT_SCORE;
				continue;
			}
			// 信頼できない TT は小さなボーナスに留める（誤誘導を防ぐ）
			m.score += ShidenAIConfig::MoveOrdering::PROXIMITY_BONUS_BASE;
		}

		// --- 追加: 打ち（isDrop）の優先付け ---
		if (m.isDrop) {
			// 王手フラグが立っていれば大きく加点
			if (m.givesCheck) {
				const int dropCheckBonus = ShidenAIConfig::MoveOrdering::BECOMING_CAPTURE_3POINTS_BONUS;
				m.score += dropCheckBonus;
			}

			// 王との近接ボーナスは軽量計算（王位置取得のみ）
			Point enemyKing = FindKing(board.units, enemy);
			if (enemyKing.x >= 0) {
				int dx = std::abs(m.target.x - enemyKing.x);
				int dy = std::abs(m.target.y - enemyKing.y);
				int cheb = std::max(dx, dy);
				if (cheb <= 1) {
					const int dropProximityUnit = ShidenAIConfig::MoveOrdering::PROXIMITY_BONUS_UNIT;
					int proximityBonus = dropProximityUnit * (2 - cheb);
					m.score += proximityBonus;
				}
			}

			// --- 追加: 打ちに対する SEE 評価を適用 ---
			// 敵がその到着マスに利きがある（合駒で取られる可能性がある）場合、SEE を計算して
			// 「打った側」の純利益をスコアに反映する（負なら損、0 に近い負が好ましい）
			if (IsSquareAttacked(const_cast<FastBoard&>(board), m.target, enemy)) {
				UnitType handType = board.units[m.unitIndex].type; // 持ち駒は非成のはず
				int seeDrop = StaticExchangeEvaluationForDrop(board, m.target.x, m.target.y, handType, board.currentTeam);
				// スケールしてスコアへ反映（既存の捕獲での SEE スケールに合わせる）
				m.score += seeDrop * ShidenAIConfig::System::SEE_SCALE;
			}

			// --- 追加: 王手時に「安い駒優先」を適用（合駒候補をさらに強調） ---
			if (inCheck && !checkers.empty()) {
				for (const auto& chk : checkers) {
					if (IsOnSameLine(kingPos, chk, m.target)) {
						UnitType handType = board.units[m.unitIndex].type;
						int handVal = PieceValue(handType, ShidenAIConfig::PieceValue::TOTAL_VAL);
						int bonus = (ShidenAIConfig::PieceValue::VAL.KING - handVal) / 10;
						if (bonus > 0) m.score += bonus;
						break;
					}
				}
			}

			continue;
		}

		// 非打ち手（捕獲/履歴/キラー等）は従来通り
		if (!m.isDrop) {
			bool isCapture = false;
			UnitType victimType = UnitType::King;
			for (const auto& t : board.units) {
				if (t.isOnBoard && t.pos == m.target && t.team != board.currentTeam) {
					isCapture = true;
					victimType = t.type;
					break;
				}
			}
			if (isCapture) {
				int seeValue = StaticExchangeEvaluation(board, m.target.x, m.target.y, m.unitIndex);
				int base = ShidenAIConfig::System::BASE_CAPTURE_SCORE;
				int seeBonus = seeValue * ShidenAIConfig::System::SEE_SCALE;
				int captureBase = PieceValue(victimType, ShidenAIConfig::PieceValue::TOTAL_VAL) * ShidenAIConfig::Evaluation::CAPTURE_BASE_VALUE_FACTOR - PieceValue(board.units[m.unitIndex].type, ShidenAIConfig::PieceValue::TOTAL_VAL);
				m.score = base + seeBonus + captureBase;
				continue;
			}
		}

		if (m.score < ShidenAIConfig::MoveOrdering::MOVE_ORDER_CAPTURE_SCORE) {
			if (m.unitIndex == killerMoves[depth][0].unitIndex && m.target == killerMoves[depth][0].target) m.score = ShidenAIConfig::MoveOrdering::MOVE_ORDER_KILLER_SCORE;
			else {
				if (m.target.y >= 0 && m.target.y < BoardSize && m.target.x >= 0 && m.target.x < BoardSize) {
					m.score = historyTable[board.currentTeam == Team::Blue ? 0 : 1][(int)board.units[m.unitIndex].type][m.target.y * BoardSize + m.target.x];
				}
			}
		}
	}
	std::sort(moves.begin(), moves.end(), [](const Candidate& a, const Candidate& b) { return a.score > b.score; });
}

// 盤上駒の候補生成（移動・捕獲・成りの簡易判断含む）
void ShidenAI::GeneratePieceCandidates(std::vector<Candidate>& out, FastBoard& root, Team aiTeam)
{
	for (int i = 0; i < static_cast<int>(root.units.size()); ++i)
	{
		const Unit& u = root.units[i];
		if (u.team != aiTeam) continue;
		if (!u.isOnBoard) continue;
		if (u.hasMoved) continue;

		int moveRange[BoardSize][BoardSize];
		Movement::Context ctx;
		ctx.moveRange = moveRange;
		ctx.map = root.mapRef;
		ctx.chipDB = root.chipDBRef;
		ctx.unitList = &root.units;
		ctx.selectedUnitPosition = Point{ u.pos.x, u.pos.y };
		ctx.moveDirection = (aiTeam == Team::Blue ? 1 : -1);
		Movement::ComputeMoveRange(moveRange);

		const auto& info = root.unitDBRef->at(u.type);
		const int cost = root.chipDBRef->at(root.mapRef->at(u.pos.y, u.pos.x)).cost;
		Movement::ComputeMoveRangeImpl(ctx, u.pos.y, u.pos.x, info.speed + cost, u.type, 0);
		Movement::ClearMoveRangeForCurrentTeam(moveRange, root.units, u.team);

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
					c.promoteDecision = (promoted != u.type) && (PieceValue(promoted, ShidenAIConfig::PieceValue::TOTAL_VAL) > PieceValue(u.type, ShidenAIConfig::PieceValue::TOTAL_VAL));
				}

				out.push_back(c);
			}
		}
	}
}


// 終盤判定：盤上駒数が閾値以下なら終盤とみなす
bool ShidenAI::IsEndgame(const FastBoard& board) const {
	int pieceCount = 0;
	for (const auto& u : board.units) {
		if (!u.isOnBoard) continue;
		if (u.type == UnitType::King || u.type == UnitType::King1) continue;
		++pieceCount;
		if (pieceCount > ShidenAIConfig::Endgame::ENDGAME_PIECE_COUNT_THRESHOLD) return false;
	}
	return true;
}


// 動的な王の PST 評価：序盤は従来テーブル、終盤は中央化・入玉・逃げ道を重視
int ShidenAI::GetDynamicKingPST(const FastBoard& board, Team team, int y, int x, AttackMap& enemyAttackMap) {
	if (y < 0 || y >= BoardSize || x < 0 || x >= BoardSize) return 0;
	int evalY = (team == Team::Blue) ? y : (BoardSize - 1 - y);

	int base = ShidenAIConfig::PST::KING[evalY][x];

	// 終盤でなければ従来値を返す
	if (!IsEndgame(board)) return base;

	// 終盤: 中央化・敵陣優先・逃げ道を評価に加える
	const int centerX = BoardSize / 2;
	const int centerY = BoardSize / 2;
	int maxDistToCenter = std::max(std::abs(x - centerX), std::abs(y - centerY));
	int centerScore = (centerX - maxDistToCenter) * ShidenAIConfig::Endgame::ENDGAME_CENTER_WEIGHT;

	int enemyZoneBonus = 0;
	if ((team == Team::Blue && y <= centerY - 1) || (team == Team::Red && y >= centerY + 1)) {
		enemyZoneBonus = ShidenAIConfig::Endgame::ENDGAME_ENEMY_ZONE_BONUS;
	}

	// 逃げ道（liberty）: enemyAttackMap は事前に構築済みの想定
	int liberty = CountKingLiberty(Point{ x, y }, enemyAttackMap);
	int libertyScore = liberty * ShidenAIConfig::Endgame::ENDGAME_LIBERTY_WEIGHT;

	return base + centerScore + enemyZoneBonus + libertyScore;
}
int ShidenAI::GetPSTValue(UnitType type, int y, int x, Team team) {
	if (y < 0 || y >= BoardSize || x < 0 || x >= BoardSize) return 0;
	int evalY = (team == Team::Blue) ? y : (BoardSize - 1 - y);
	int evalX = x;

	switch (type) {
	case UnitType::Pawn:   return ShidenAIConfig::PST::PAWN[evalY][evalX];
	case UnitType::Knight: return ShidenAIConfig::PST::KNIGHT[evalY][evalX];

		// 追加分 ---
	case UnitType::Silver: return ShidenAIConfig::PST::SILVER[evalY][evalX];
	case UnitType::Lance:  return ShidenAIConfig::PST::LANCE[evalY][evalX];
	case UnitType::Bishop: return ShidenAIConfig::PST::BISHOP[evalY][evalX];
		// ---------

	case UnitType::Gold:
	case UnitType::Pawn1:
	case UnitType::Silver1:
	case UnitType::Knight1:
	case UnitType::Lance1:
		return ShidenAIConfig::PST::GOLD[evalY][evalX];

	case UnitType::Rook: return ShidenAIConfig::PST::ROOK[evalY][evalX];
	case UnitType::Rook1:   return ShidenAIConfig::PST::ROOK[evalY][evalX] + ShidenAIConfig::Evaluation::DRAGON_HORSE_ACTIVITY_BONUS;
	case UnitType::Bishop1: return ShidenAIConfig::PST::BISHOP[evalY][evalX] + ShidenAIConfig::Evaluation::DRAGON_HORSE_ACTIVITY_BONUS; // 馬は角より強い

	default: return 0;
	}
}


inline int ShidenAI::PawnStructure(const Unit& u, Team rootAI, const Array<Unit>& units) {
	if (u.type != UnitType::Pawn) return 0;

	int val = 0;
	int forward = (rootAI == Team::Blue) ? -1 : 1;
	if ((rootAI == Team::Blue && u.pos.y <= kBluePromoteLine) || (rootAI == Team::Red && u.pos.y >= BoardSize - kRedPromoteLine)) val += ShidenAIConfig::Evaluation::DROOPING_STEP_BONUS;
	for (const auto& ID : piecesID.goldID) {
		if (ID < 0 || ID >= static_cast<int>(units.size())) continue;
		const Unit& t = units[ID];
		if (u.team != rootAI) continue;
		if (t.pos.x == u.pos.x &&
			t.pos.y == u.pos.y - forward) val += ShidenAIConfig::Evaluation::BOTTOM_STEP_BONUS;
	}
	for (const auto& ID : piecesID.bishopID) {
		if (ID < 0 || ID >= static_cast<int>(units.size())) continue;
		const Unit& t = units[ID];
		if (t.type == UnitType::Bishop
			&& t.pos.x == u.pos.x
			&& t.pos.y == u.pos.y + forward) val += ShidenAIConfig::Evaluation::ROUND_HEADED_PIECE_BONUS;
	}
	for (const auto& ID : piecesID.knightID) {
		if (ID < 0 || ID >= static_cast<int>(units.size())) continue;
		const Unit& t = units[ID];
		if (u.type != UnitType::Knight)continue;
		if (t.pos.x == u.pos.x
			&& t.pos.y == u.pos.y + forward) val += ShidenAIConfig::Evaluation::ROUND_HEADED_PIECE_BONUS;
	}

	return val;
}


inline int ShidenAI::EvalUnitIndividual(const Unit& u, Team rootAI, Point myKing, Point enKing, const Array<Unit>& units)
{
	int score = 0;
	if (!u.isOnBoard)return 0;
	if (u.type == UnitType::Pawn) {
		score += PawnStructure(u, rootAI, units);
		return score;
	}
	if (u.type == UnitType::Gold
		&& u.type == UnitType::Silver
		&& u.type == UnitType::Knight) {
		Point King(u.team == rootAI ? myKing : enKing);
		if (King.x >= 0) {
			int dist = std::abs(u.pos.x - King.x) + std::abs(u.pos.y - King.y);
			if (dist <= ShidenAIConfig::MoveOrdering::PROXIMITY_BONUS_MAX_DISTANCE) score += (ShidenAIConfig::MoveOrdering::PROXIMITY_BONUS_MAX - dist) * ShidenAIConfig::Evaluation::CASTLE_NEAR_KING_BONUS;
		}
		return score;
	}
	return score;
}

int ShidenAI::CountKingLiberty(Point kingPos, const AttackMap& enemyAttacks) {
	if (kingPos.x == -1) return 0;
	int liberty = 0;
	for (int dy = -1; dy <= 1; ++dy) {
		for (int dx = -1; dx <= 1; ++dx) {
			if (dx == 0 && dy == 0) continue;
			Point p = { kingPos.x + dx, kingPos.y + dy };
			if (p.x >= 0 && p.x < BoardSize && p.y >= 0 && p.y < BoardSize) {
				// 敵の利きがなく、味方の駒もなければ「逃げ道」
				if (!enemyAttacks.IsCovered(p)) {
					liberty++;
				}
			}
		}
	}
	return liberty;
}

// 距離評価（マンハッタン距離の負）
int ShidenAI::DistanceScore(const Point& a, const Point& b)
{
	return -(std::abs(a.x - b.x) + std::abs(a.y - b.y));
}

// 浮き駒ペナルティ
int ShidenAI::FloatingPiecePenalty(const Unit& u, Team aiTeam, AttackMap attacBkyMyself, AttackMap enemyByAttack) {
	if (!u.isOnBoard) return 0;
	if (u.team != aiTeam) return 0;
	if (u.type == UnitType::King ||
		u.type == UnitType::King1) return 0;
	if (!enemyByAttack.IsCovered(u.pos))return PieceValue(u.type, ShidenAIConfig::PieceValue::TOTAL_VAL) -ShidenAIConfig::Evaluation::NO_ENEMY_ATTACK_BONUS;
	if (attacBkyMyself.IsCovered(u.pos))return -ShidenAIConfig::Evaluation::SUPPORT_BONUS;
	return PieceValue(u.type, ShidenAIConfig::PieceRisk::TOTAL_UNDER_ATTACK_PENALTY);
}

int ShidenAI::FindLeastValuableAttacker(const FastBoard& board, int tx, int ty, Team attackerTeam)
{
	if (tx < 0 || tx >= BoardSize || ty < 0 || ty >= BoardSize) return -1;
	int bestIdx = -1;
	int bestVal = std::numeric_limits<int>::max();
	// 各敵ユニットがターゲットに利いているかを検査
	for (int i = 0; i < static_cast<int>(board.units.size()); ++i) {
		const Unit& u = board.units[i];
		if (!u.isOnBoard) continue;
		if (u.team != attackerTeam) continue;
		// 到着位置での利き判定（軽量: ビットボードを返す）
		Bitboard att = AttackMap().AttacksFromUnit(board, u);
		int tsq = SQ(tx, ty);
		if (!att.test(tsq)) continue;
		int val = PieceValue(u.type, ShidenAIConfig::PieceValue::TOTAL_VAL);
		if (val < bestVal) { bestVal = val; bestIdx = i; }
	}
	return bestIdx;
}
int ShidenAI::EvalBasicMaterial(const FastBoard& board, Team rootAI) {
	int score = 0;
	int selfIdx = (rootAI == Team::Blue ? 0 : 1);
	int oppIdx = 1 - selfIdx;
	score += board.pstScore[selfIdx] - board.pstScore[oppIdx];
	score += board.handScore[selfIdx] - board.handScore[oppIdx];
	score += board.materialScore[selfIdx] - board.materialScore[oppIdx];
	return score;
}
double ShidenAI::CalculatePhase(const FastBoard& board){
	int pieceCount = board.pieceCount;
	double phase = static_cast<double>(pieceCount) / ShidenAIConfig::Evaluation::PAWN_PHASE_DIVISOR;
	if (phase < 0.0) return 0.0;
	if (phase > 1.0) return 1.0;
	return phase;
}
int ShidenAI::EvalTacticalStability(const FastBoard& board, const Unit& u, Team rootAI,
	AttackMap& myAttacks, AttackMap& enemyAttacks) {
	int score = 0;
	Team enemyTeam = (rootAI == Team::Blue ? Team::Red : Team::Blue);
	// 浮き駒ペナルティ（既存）
	if (u.team == rootAI) {
		score -= FloatingPiecePenalty(u, rootAI, myAttacks, enemyAttacks);
	}
	else {
		score += FloatingPiecePenalty(u, enemyTeam, enemyAttacks, myAttacks);
	}

	// --- SEE を呼ぶ条件を制限: 敵の利きがある駒のみ ---
	if (enemyAttacks.IsCovered(u.pos)) {
		Team attackersTeam = (u.team == rootAI) ? enemyTeam : rootAI;
		int attackerIdx = FindLeastValuableAttacker(board, u.pos.x, u.pos.y, attackersTeam);
		if (attackerIdx != -1) {
			int see = StaticExchangeEvaluation(board, u.pos.x, u.pos.y, attackerIdx);
			if (see < 0) score += see * 2; // 確定的な駒損をより重く見積もる
			score += (u.team == rootAI) ? -see : see;
		}
	}
	return score;
}


int ShidenAI::EvaluateDetailed(FastBoard& board, Team rootAI, bool heavy) {
	int score = 0;

	Team enemyTeam = (rootAI == Team::Blue ? Team::Red : Team::Blue);
	int selfIdx = (rootAI == Team::Blue ? 0 : 1);
	int oppIdx = (enemyTeam == Team::Blue ? 1 : 0);
	Point myKing = board.kingPos[selfIdx];
	Point enKing = board.kingPos[oppIdx];
	if (myKing.x == -1) return -ShidenAIConfig::Evaluation::KING_TRAPPED_PENALTY;
	if (enKing.x == -1) return ShidenAIConfig::Evaluation::KING_TRAPPED_PENALTY;
	score = EvalBasicMaterial(board, rootAI);
	// --- フェーズ計算（盤上の非玉駒数で序中盤・終盤を正規化） ---
	
	double phase = CalculatePhase(board);

	// --- 事前にチーム別の AttackMap を構築（重複計算削減） ---
	AttackMap attacBkyMyself; attacBkyMyself.Reset();
	AttackMap enemyByAttack; enemyByAttack.Reset();
	attacBkyMyself.BuildFromBoard(board, rootAI);
	enemyByAttack.BuildFromBoard(board, enemyTeam);

	// 盤上駒の評価
	for (const auto& u : board.units) {
		int val = 0;
		if (!u.isOnBoard) continue;
	
			if (u.type == UnitType::King || u.type == UnitType::King1) {
				// 敵の利きマップ（王の相手チーム）を選ぶ
				AttackMap& enemyAttackMapForKing = (u.team == rootAI) ? enemyByAttack : attacBkyMyself;
				val += GetDynamicKingPST(board, u.team, u.pos.y, u.pos.x, enemyAttackMapForKing);
			}
			else {
				val += DistanceScore(u.pos, (u.team == rootAI) ? enKing : myKing);
			}

			int rank = (u.team == Team::Blue ? BoardSize - 1 - u.pos.y : u.pos.y);
			val += rank * ShidenAIConfig::Evaluation::ADVANCE_BONUS;

			// --- 歩の序盤前進ボーナス ---
			if (u.type == UnitType::Pawn) {
				int pawnAdvanceBonus = static_cast<int>(ShidenAIConfig::Evaluation::PAWN_ADVANCE_BONUS * rank * phase);
				val += pawnAdvanceBonus;
			}
			// 王の前駒ボーナス（king shield）
			int kingShieldBonus = 0;
			int forward = (u.team == Team::Blue ? -1 : 1);
			if (myKing.x >= 0 && myKing.y >= 0) {
				Point shieldPos = { myKing.x, myKing.y + forward };
				if (shieldPos.x >= 0 && shieldPos.x < BoardSize && shieldPos.y >= 0 && shieldPos.y < BoardSize) {
					int protectorIdx = board.occ[shieldPos.y][shieldPos.x];
					if (protectorIdx >= 0 && protectorIdx < static_cast<int>(board.units.size())) {
						if (board.units[protectorIdx].team == rootAI) {
							kingShieldBonus += ShidenAIConfig::Evaluation::KING_SHIELD_BONUS;
						}
					}
				}
			}
		score += EvalTacticalStability(board, u, rootAI, attacBkyMyself, enemyByAttack);// 浮き駒ペナルティと SEE に基づく安定性評価
		score += (u.team == rootAI ? val : -val);
	}
	if (heavy) {
		if (enemyByAttack.IsCovered(myKing))  score -= ShidenAIConfig::Evaluation::CHECKMATE_TO_ENEMY_KING; // 王手ペナルティ
		if (attacBkyMyself.IsCovered(enKing)) score += ShidenAIConfig::Evaluation::CHECKMATE_TO_ENEMY_KING; // 王手ボーナス
	}
	return score;
}
void ShidenAI::TickNode() {
	static unsigned int nodeCount = 0;
	// 4096 ノードごとに一度だけ時刻を確認
	if (((++nodeCount) & ShidenAIConfig::System::NODES_PER_TIME_CHECK) == 0u) {
		if ((std::chrono::steady_clock::now() - startTime) > std::chrono::milliseconds(ShidenAIConfig::SearchConfig::TIME_LIMIT_MS)) {
			timeUp = true;
		}
	}
}

int ShidenAI::Quiescence(FastBoard& board, Team rootAI, int alpha, int beta, int q_depth, bool heavy) {
	TickNode();
	if (timeUp) return 0;

	int stand_pat = EvaluateDetailed(board, rootAI, heavy);
	if (stand_pat >= beta) return beta;
	if (alpha < stand_pat) alpha = stand_pat;
	if (q_depth >= ShidenAIConfig::SearchConfig::MAX_QUIESCENCE_DEPTH) return alpha;
	// プールを使う
	auto& moves = GetMoveListForDepth(q_depth, true);
	moves.clear();
	GenerateTacticalMoves(moves, board);
	SortMoves(moves, board, 0, -1, { -1, -1 });
	for (const auto& mv : moves) {
		MoveInfo info;
		DoMove(board, mv, info);
		int val = -Quiescence(board, rootAI, -beta, -alpha, q_depth + 1, true);
		UndoMove(board, info);

		if (val >= beta) return beta;
		if (val > alpha) alpha = val;
	}
	return alpha;
}
int ShidenAI::AlphaBeta(FastBoard& board, Team rootAI, int depth, int alpha, int beta, int dist) {
	TickNode();
	if (timeUp) return 0;

	// 軽量王手判定
	int movedSideIdx = (board.currentTeam == Team::Blue ? 1 : 0);
	Point myKing = board.kingPos[movedSideIdx];
	if (myKing.x == -1) return -ShidenAIConfig::Evaluation::KING_TRAPPED_PENALTY + dist;
	Team enemyTeam = (board.currentTeam == Team::Blue ? Team::Red : Team::Blue);
	bool inCheck = IsSquareAttacked(board, myKing, enemyTeam);

	// 千日手判定
	for (uint64 h : searchStack) if (h == board.hash) return ShidenAIConfig::System::REPETITION_DRAW_SCORE;

	// 置換表アクセス
	TTEntry* tt = LookupTT(board.hash);
	if (tt && tt->depth >= depth) {
		if (tt->flag == HashFlag::Exact) return tt->score;
		if (tt->flag == HashFlag::Alpha && tt->score <= alpha) return alpha;
		if (tt->flag == HashFlag::Beta && tt->score >= beta) return beta;
	}

	if (depth <= 0) return Quiescence(board, rootAI, alpha, beta, 0, true);

	// Null Move Pruning（既存のロジックを維持）
	if (depth >= 3 && dist > 0 && !inCheck) {
		FastBoard next = board;
		next.currentTeam = enemyTeam;
		next.hash ^= zobristSide;
		int val = -AlphaBeta(next, rootAI, depth - ShidenAIConfig::SearchConfig::NULL_MOVE_REDUCTION, -beta, -beta + 1, dist + 1);
		if (val >= beta) return beta;
	}

	// 手の生成
	auto& moves = GetMoveListForDepth(depth, false);
	moves.clear();
	GenerateMoves(moves, board, false);
	if (moves.empty()) return -ShidenAIConfig::Evaluation::KING_TRAPPED_PENALTY + dist;

	SortMoves(moves, board, depth, (tt ? tt->bestUnitIndex : -1), (tt ? tt->bestTarget : Point{ -1, -1 }));

	int bestVal = -ShidenAIConfig::System::SCORE_INF;
	Candidate bestMove{};
	int moveCount = 0;
	searchStack.push_back(board.hash);

	bool isFirst = true;
	for (const auto& mv : moves) {
		TickNode();
		if (timeUp) return 0;
		MoveInfo info;
		DoMove(board, mv, info);

		// 合法手チェックを軽量化
		int movedSideIdx2 = (board.currentTeam == Team::Blue ? 1 : 0);
		Point currentKing = board.kingPos[movedSideIdx2];
		if (currentKing.x == -1 || IsSquareAttacked(board, currentKing, board.currentTeam)) {
			UndoMove(board, info);
			continue;
		}

		int val = 0;
		bool isTactical = info.captured || mv.promoteDecision || mv.isDrop;
		bool isKiller = (mv.unitIndex == killerMoves[depth][0].unitIndex && mv.target == killerMoves[depth][0].target);

		if (isFirst) {
			// 最初の手はフルウィンドウで探索（PV確定）
			val = -AlphaBeta(board, rootAI, depth - 1, -beta, -alpha, dist + 1);
			isFirst = false;
		}
		else {
			// PVS: まず Null Window (α, α+1) で探索（高速に否定できる場合が多い）
			val = -AlphaBeta(board, rootAI, depth - 1, -alpha - 1, -alpha, dist + 1);

			// もし有望（> α）ならフルウィンドウで再探索（必要なら LMR を適用）
			if (val > alpha) {
				// 既存の LMR（浅削減）条件を尊重
				if (depth >= 3 && moveCount > 5 && !isTactical && !isKiller && !inCheck) {
					// まず減らし幅で狭いウィンドウ探索
					val = -AlphaBeta(board, rootAI, depth - 2, -alpha - 1, -alpha, dist + 1);
					if (val > alpha) {
						// 必要ならフルウィンドウ再探索
						val = -AlphaBeta(board, rootAI, depth - 1, -beta, -alpha, dist + 1);
					}
				}
				else {
					// 通常のフルウィンドウ再探索
					val = -AlphaBeta(board, rootAI, depth - 1, -beta, -alpha, dist + 1);
				}
			}
		}

		UndoMove(board, info);

		if (val >= beta) {
			// Betaカット：キラールール更新（ただし捕獲手は除外）
			if (!info.captured) {
				killerMoves[depth][1] = killerMoves[depth][0];
				killerMoves[depth][0] = mv;
			}
			searchStack.pop_back();
			return beta;
		}

		if (val > bestVal) {
			bestVal = val;
			bestMove = mv;
			if (val > alpha) alpha = val;
		}
		moveCount++;
	}

	searchStack.pop_back();

	// 置換表に登録（深さ・フラグは既存の方針に従う）
	if (!timeUp && bestMove.unitIndex != -1) {
		HashFlag flag = (bestVal >= beta ? HashFlag::Beta : (bestVal <= alpha ? HashFlag::Alpha : HashFlag::Exact));
		StoreTT(board.hash, depth, bestVal, flag, bestMove);
	}

	return bestVal;
}



bool ShidenAI::GivesCheckAfterMove(ShidenAI& ai, ShidenAI::FastBoard board, const Candidate& cand, Team mover) {
	MoveInfo info;
	ai.DoMove(board, cand, info);
	Team enemy = (mover == Team::Blue) ? Team::Red : Team::Blue;
	Point enemyKing = ai.FindKing(board.units, enemy);
	bool gives = (enemyKing.x != -1) && ai.IsSquareAttacked(board, enemyKing, mover);
	ai.UndoMove(board, info);
	return gives;
}


bool ShidenAI::MateDFS(ShidenAI& ai, ShidenAI::FastBoard& board, Team sideToMove, Team attacker, int depth) {
	if (timeUp || (std::chrono::steady_clock::now() - ai.startTime) > std::chrono::milliseconds(ShidenAIConfig::SearchConfig::TIME_LIMIT_MS)) {
		timeUp = true;
		return false;
	}

	// 敵玉が存在しなければ詰み
	Team defender = (attacker == Team::Blue) ? Team::Red : Team::Blue;
	if (ai.FindKing(board.units, defender).x == -1) return true;

	if (depth <= 0) return false;

	// moves をプールから取得
	auto& moves = ai.GetMoveListForDepth(depth, false);
	moves.clear();
	ai.GenerateMoves(moves, board, false);
	if (moves.empty()) {
		// 手がない＝詰んでいるなら成立（ただし王手限定探索なので通常ここは false）
		return (ai.FindKing(board.units, sideToMove).x == -1);
	}

	// checkMoves は別スロットを使う（深さ+1 を使用）
	auto& checkMoves = ai.GetMoveListForDepth(depth + 1, false);
	checkMoves.clear();
	for (const auto& mv : moves) {
		if (GivesCheckAfterMove(ai, board, mv, sideToMove)) checkMoves.push_back(mv);
	}

	if (checkMoves.empty()) {
		return false;
	}

	if (sideToMove == attacker) {
		for (const auto& mv : checkMoves) {
			TickNode();
			if (timeUp) return false;
			MoveInfo info;
			ai.DoMove(board, mv, info);
			bool res = MateDFS(ai, board, (sideToMove == Team::Blue ? Team::Red : Team::Blue), attacker, depth - 1);
			ai.UndoMove(board, info);
			if (res) return true;
			if (timeUp) return false;
		}
		return false;
	}
	else {
		for (const auto& mv : checkMoves) {
			TickNode();
			if (timeUp) return false;
			MoveInfo info;
			ai.DoMove(board, mv, info);
			bool res = MateDFS(ai, board, (sideToMove == Team::Blue ? Team::Red : Team::Blue), attacker, depth - 1);
			ai.UndoMove(board, info);
			if (!res) return false;
			if (timeUp) return false;
		}
		return true;
	}
}

bool ShidenAI::FindMateAtRoot(ShidenAI& ai, FastBoard& root, Team aiTeam, int maxDepth, Candidate& outMove) {
	// ルートは depth 指定がないため root 用インデックス 0 を使う
	auto& moves = ai.GetMoveListForDepth(0, false);
	moves.clear();
	ai.GenerateMoves(moves, root, false);
	if (moves.empty()) return false;

	ai.SortMoves(moves, root, 0, -1, Point{ -1, -1 });

	for (int depth = 1; depth <= maxDepth; ++depth) {
		TickNode();
		if (timeUp) { break; }
		for (const auto& mv : moves) {
			if (!GivesCheckAfterMove(ai, root, mv, aiTeam)) continue;
			MoveInfo info;
			ai.DoMove(root, mv, info);
			Team next = (aiTeam == Team::Blue) ? Team::Red : Team::Blue;
			bool mate = MateDFS(ai, root, next, aiTeam, depth - 1);
			ai.UndoMove(root, info);
			if (mate) {
				outMove = mv;
				return true;
			}
			if (timeUp) return false;
		}
	}
	return false;
}


Candidate ShidenAI::IterativeDeepening(Game& game, Team aiTeam, AIState& state) {
	searchStack = state.gameHistory; // 実際の対局履歴をスタックの底に積む
	startTime = std::chrono::steady_clock::now(); timeUp = false;
	ClearHeuristics(); currentAge++;
	InitZobrist();

	// pool をここで確実に初期化しておく
	GetMoveListForDepth(0, false);

	FastBoard root(game, *this); Candidate best;
	best.score = -ShidenAIConfig::System::SCORE_INF;
	Candidate mateMove;
	if (FindMateAtRoot(*this, root, aiTeam, ShidenAIConfig::SearchConfig::MATE_TRY_DEPTH, mateMove)) {
		return mateMove;
	}
	piecesID.PiecesIDSearch(root.units);

	// ルート用 moves をプールから取得
	auto& movesRoot = GetMoveListForDepth(0, false);
	movesRoot.clear();
	GenerateMoves(movesRoot, root, false);
	if (movesRoot.empty()) {
		GeneratePieceCandidates(movesRoot, root, aiTeam);
		return movesRoot[0]; // 空の候補を返す
	}

	best = movesRoot[0]; // これで moves[0] が安全になる

	Team enemyTeam = (aiTeam == Team::Blue ? Team::Red : Team::Blue);
	Point enemyKingPos = FindKing(root.units, enemyTeam);

	for (const auto& mv : movesRoot) {
		if (timeUp || (std::chrono::steady_clock::now() - startTime) > std::chrono::milliseconds(ShidenAIConfig::SearchConfig::TIME_LIMIT_MS)) { timeUp = true; break; }
		if (!mv.isDrop && mv.target == enemyKingPos) {
			return mv;
		}
	}

	for (int d = 1; d <= ShidenAIConfig::SearchConfig::MAX_DEPTH; ++d) {
		AlphaBeta(root, aiTeam, d, -ShidenAIConfig::System::SCORE_INF, ShidenAIConfig::System::SCORE_INF, 0);
		if (timeUp) break;
		TTEntry* tt = LookupTT(root.hash);
		if (tt && tt->bestUnitIndex >= 0 && tt->bestUnitIndex < static_cast<int>(root.units.size())) {
			best = { tt->bestUnitIndex, tt->bestTarget, root.units[tt->bestUnitIndex].pos, false, tt->bestPromote, tt->score };
		}
	}

	if (best.unitIndex >= 0) {
		return best;
	}

	// fallback
	auto& fallbackMoves = GetMoveListForDepth(0, false);
	fallbackMoves.clear();
	GenerateMoves(fallbackMoves, root, false);
	if (!fallbackMoves.empty()) {
		SortMoves(fallbackMoves, root, 0, -1, Point{ -1, -1 });

		struct Scored { Candidate cand; int score; };
		std::vector<Scored> safeCandidates;
		Team enemy = (aiTeam == Team::Blue) ? Team::Red : Team::Blue;

		for (const auto& cand : fallbackMoves) {
			if (timeUp || (std::chrono::steady_clock::now() - startTime) > std::chrono::milliseconds(ShidenAIConfig::SearchConfig::TIME_LIMIT_MS)) {
				timeUp = true;
				break;
			}

			FastBoard tmp = root;
			MoveInfo info;
			DoMove(tmp, cand, info); // tmp を一手進める（内部で side 反転）

			Point myKingPos = FindKing(tmp.units, aiTeam);
			bool illegal = (myKingPos.x == -1) || IsSquareAttacked(tmp, myKingPos, enemy);

			if (!illegal) {
				int s = EvaluateDetailed(tmp, aiTeam, true);

				if (!cand.isDrop) {
					int occIdx = -1;
					if (cand.target.y >= 0 && cand.target.y < BoardSize && cand.target.x >= 0 && cand.target.x < BoardSize) {
						occIdx = root.occ[cand.target.y][cand.target.x];
						if (occIdx >= 0 && occIdx < static_cast<int>(root.units.size()) && root.units[occIdx].team != aiTeam) {
							s += PieceValue(root.units[occIdx].type, ShidenAIConfig::PieceValue::TOTAL_VAL) * ShidenAIConfig::Evaluation::CAPTURE_BONUS_MULTIPLIER;
						}
					}
					if (cand.promoteDecision) s += ShidenAIConfig::Evaluation::PROMOTION_BONUS;
				}
				else {
					s += ShidenAIConfig::MoveOrdering::DROP_BONUS;// ドロップは多少評価を上げる（安全な手がない場合の選択肢として優先されるように）
				}

				safeCandidates.push_back({ cand, s });
			}

			UndoMove(tmp, info);
		}

		if (!safeCandidates.empty() && !timeUp) {
			const int n = std::min((int)safeCandidates.size(), ShidenAIConfig::SearchConfig::FALLBACK_TOP_N);
			Candidate bestChosen = safeCandidates.front().cand;
			int bestVal = std::numeric_limits<int>::min();
			int bestShallow = safeCandidates.front().score;

			for (int i = 0; i < n; ++i) {
				if (timeUp || (std::chrono::steady_clock::now() - startTime) > std::chrono::milliseconds(ShidenAIConfig::SearchConfig::TIME_LIMIT_MS)) {
					timeUp = true;
					break;
				}

				const Candidate& cand = safeCandidates[i].cand;
				MoveInfo info;
				DoMove(root, cand, info);

				int val = -AlphaBeta(root, aiTeam, ShidenAIConfig::SearchConfig::FALLBACK_DEEP_DEPTH, -ShidenAIConfig::System::SCORE_INF, ShidenAIConfig::System::SCORE_INF, 0);

				UndoMove(root, info);

				if (val > bestVal || (val == bestVal && safeCandidates[i].score > bestShallow)) {
					bestVal = val;
					bestChosen = cand;
					bestShallow = safeCandidates[i].score;
				}
			}

			return bestChosen;
		}

		const Candidate& fallback = fallbackMoves.front();
		return fallback;
	}

	game.SetGameActive(false);
	return best;
}



void ShidenAI::StartThinking(Game& game, Team aiTeam, AIState& state) {
	if (IsThinking()) return;
	// 非同期で実行開始
	aiTask = Async([this, g_copy = game, state_copy = state , aiTeam]() mutable {
		return IterativeDeepening(g_copy, aiTeam, state_copy);
	});
}

void ShidenAI::MakeMove(Game& game, Team aiTeam) {
	if (!game.IsGameActive()) return;
	if (aiTask.isReady()) {
		Candidate best = aiTask.get();
		
		if (best.unitIndex >= 0) {
			ExecuteCandidate(game, best);
		}
	}
	else
	{
		StartThinking(game, aiTeam, aiState);
	}

}

void ShidenAI::ExecuteCandidate(Game& game, const Candidate& cand) {
	if (cand.unitIndex < 0) return;
	game.SelectUnitByIndex(cand.unitIndex);
	game.ExecuteMoveOrCaptureSelected(cand.target.x, cand.target.y);
	if (game.GetUnitList()[cand.unitIndex].promoteFlag) {
		if (cand.promoteDecision) game.ApplyPromotionByIndex(cand.unitIndex);
		else game.CancelPromotionByIndex(cand.unitIndex);
	}
	game.TurnEndDetermination();
	
}

// デバッグ用：盤面上の攻撃範囲を赤く表示
//攻撃範囲の表示
void ShidenAI::DisplayAttackMap(Game& game, Team team) {
	FastBoard root(game, *this);
	AttackMap attackMap; attackMap.Reset();
	attackMap.BuildFromBoard(root, team);
	for (int y = 0; y < BoardSize; ++y) {
		for (int x = 0; x < BoardSize; ++x) {
			if (!attackMap.IsCovered(Point{ x, y })) {

				Rect{ x * TileSize + BoardOffsetXDefault, y* TileSize + BoardOffsetYDefault, TileSize }.draw(DrawMoveRangeColor); // 赤で塗る
			}
		}
		std::cout << std::endl;
	}
}
