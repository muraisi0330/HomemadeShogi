#pragma once
#include "Types/types.h"
#include "Constants/CommonConstants.h"
#include <Siv3D.hpp>

/// Game: ゲームのドメインロジック（状態保持・ルール実行）
/// - 描画 / 入力は外部（GameRenderer / GUIController）で行うことを想定
class Game
{
public:
	// コンストラクタ / ライフサイクル
	Game();

	// --- メイン更新 / 入力連携（GUIController などが呼ぶ） ---
	/// 盤面クリックを処理（選択中ユニットの移動・打ちなど）
	void ProcessBoardClick(Point clickedPos);

	/// ユニット選択検出（クリック → moveRange 計算等）
	void HandleUnitSelection();

	/// 成り判定（自動成りのみを適用、対話は GUIController 側で行う）
	void EvaluatePromotions();

	/// 自動成り（単一ユニット）を適用（公開してテスト可能に）
	bool ApplyAutoPromotion(Unit& u);

	void UpdateElapsedTime();


	/// ユニットの位置を変更する補助（描画コードで利用）
	void MoveUnit(Unit& u, int x, int y) const noexcept;

	// --- 基本操作 API（外部から状態を操作するための安定 API） ---
	/// 選択済みユニット一覧（読み取り専用）
	const Array<Unit>& GetUnitList() const noexcept;

	/// マップデータ（読み取り専用）
	const Grid<int32>& GetMap() const noexcept;

	/// チップ DB（読み取り専用）
	const HashTable<int32, ChipInfo>& GetChipDB() const noexcept;

	/// 駒情報 DB（読み取り専用）
	const HashTable<UnitType, UnitInfo>& GetUnitDB() const noexcept;

	/// 盤面オフセット（描画用）
	int32 GetBoardOffsetX() const noexcept;
	int32 GetBoardOffsetY() const noexcept;

	/// 指定マスの移動可能値（-1 = 不可）
	int32 GetMoveRange(int y, int x) const noexcept;

	int32 GetSelectedUnitIndex() const noexcept;
	void SetSelectedUnitIndex(int32 index) noexcept;

	/// 現在の経過時間（ゲーム進行用表示）
	double GetElapsedTime() const noexcept;

	/// ゲーム有効フラグの取得
	bool IsGameActive() const noexcept;

	/// ゲーム有効フラグの設定（GUI 等から呼ばれることを想定）
	void SetGameActive(bool a) noexcept;

	/// ユニット選択中か
	bool IsUnitSelected() const noexcept;

	/// 現在のチーム（先手/後手）
	Team GetCurrentTeam() const noexcept;

	// --- 選択 / 移動系の操作 API（GUI から呼ぶ） ---
	/// 指定インデックスのユニットを選択（GUI 用）
	void SelectUnitByIndex(int index);

	/// 手駒を盤上へ配置（成功/失敗を返す）
	bool PlaceUnitFromHand(int unitIndex, int x, int y);

	/// 選択済みユニットに対して移動／捕獲処理を実行するラッパー
	/// - 選択済みユニットが無ければ false を返す
	bool ExecuteMoveOrCaptureSelected(int x, int y) noexcept;

	// --- プロモーション関連（GUI 側へ表示を移譲） ---
	/// 保留中の成り対象（最初に見つかったインデックス、該当なしは -1）
	int GetPendingPromotionIndex() const noexcept;

	/// GUI が「はい」を押したときに成りを適用
	void ApplyPromotionByIndex(int index) noexcept;

	/// GUI が「いいえ」を押したときに成り要求をキャンセル
	void CancelPromotionByIndex(int index) noexcept;

	/// ダイアログ表示フラグ取得（GUI が表示中フラグを管理）
	bool IsPromoteDialog() const noexcept;

	/// ダイアログ表示フラグ設定（GUI が表示中フラグを管理）
	void SetIsPromoteDialog(bool v) noexcept;


	int32 GetDeathKingIndex() const noexcept;
	// --- サウンド / リセット / UI 連携 ---
	void SetOnGameEvent(std::function<void(GameEvent)> callback);

	/// ゲーム状態を初期化（GUI 等から呼べるよう非 const 実装）
	void ResetState() noexcept;
	//勝利判定
	void CheckVictoryCondition();

	//手番
	bool IsOwnTurn(Team team);

	/// 手駒表示用オフセット（左右スクロール）
	int32 GetBlueHandOffset() const noexcept;
	int32 GetRedHandOffset() const noexcept;
	void SetBlueHandOffset(int32 v) noexcept;
	void SetRedHandOffset(int32 v) noexcept;
	void TurnEndDetermination();
	void ResetTimer();
private:
	struct Impl; // 将来の pImpl 用（未使用）
	// 成り判定用境界線
	const int32 kBoardOffsetX = BoardOffsetXDefault;// 盤面Xオフセット
	const int32 kBoardOffsetY = BoardOffsetYDefault; // 盤面Yオフセット
	// --- 内部状態（実装詳細） ---
    int32 m_moveRange[BoardSize][BoardSize]{};   // 移動可能範囲
	
	struct GameStateData {
		Team currentTeam = Team::Blue;
		int32 moveDirection = 1;// 移動方向（青: +1、赤: -1）
		bool unitSelectionFlag = false;// ユニット選択中フラグ
		int selectedUnitIndex = ResetSelectedIndex; // インデックスをリセット
		Vec2 selectedUnitPosition = Vec2{ 0, 0 };
		int32 blueHandOffset = 0;
		int32 redHandOffset = 0;
		double elapsedTime = 0.0;
		bool isGameActive = true;// ゲーム有効フラグ
		bool isPromoteDialog = true;// 成りダイアログ表示中フラグ
	};
	GameStateData gameState;
	HashTable<int32, ChipInfo> chipDB;// チップ情報DB
	HashTable<UnitType, UnitInfo> unitDB;// 駒情報DB
	Array<Unit> unitList;// ユニット一覧
	Grid<int32> map{
		{1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1},
	};// マップデータ

	int32 pendingPromotionIndex = -1;// 保留中の成り対象インデックス
	int32 deathKingIndex = -1;// 死亡した王将のインデックス（勝利判定用）
	std::function<void(GameEvent)> m_onGameEvent;
	s3d::Stopwatch m_stopwatch{ StartImmediately::Yes };
	void InvokeEvent(GameEvent e) const;

	// --- 内部ヘルパー（実装は Game.cpp） ---
	void OrganizeYourPieces(Unit* sel);//持ち駒整理
	bool HasAnyUnitMoved();//ユニットが移動済みか
	// 盤上の駒選択
	void SelectBoardUnit();//ユニット選択
	void SelectHandUnit();//手駒選択
	void DeselectUnit(int x, int y);//ユニット選択解除
	void HandlePlaceFromHand(Unit* sel, int x, int y);//手駒の打ち処理
	void HandleNormalMoveOrCapture(Unit* sel, int x, int y);//通常移動・捕獲処理
	// 現在手を切り替え
	//移動
	void ComputeMoveRange(Unit* sel, int x, int y);
	//捕縛
	void ComputeDropMoveRange(Unit* sel, int x, int y);
	//選択解除
	void ClearSelectionMoveRange();
	// チーム切り替え
	void TeamSwitching(int x, int y);

	void SwitchCurrentTeam();
	void ResetUnitsMoveFlag();
	void RecalculateHandIndices(Unit& sel);//持ち駒インデックス再計算
	void ClearMoveRange();//移動可能範囲クリア
	void ClearMoveRangeForCurrentTeam();// 現在チームの移動可能範囲クリア
	bool IsInsideMap(int x, int y) const;// マップ内判定
	bool IsTilePassable(int x, int y) const;// 通行可能判定
	// computeMoveRange / computeDropMoveRange は Movement / Drop に移動済
	bool IsUnitAt(int x, int y) const;// 指定位置にユニットがいるか
	void PlaceUnitFromHand(Unit* sel, int x, int y);// 手駒を盤上に配置
	bool IsUnitOfCurrentTeamAt(int x, int y, Team team) const;// 指定位置に現在チームのユニットがいるか
	void HandleMoveOrCaptureInternal(Unit* sel, int x, int y, Team activeTeam);// 移動・捕獲処理
	bool IsPromotionEligible(const Unit& unit, Team team, int y, int x) const;// 成り判定
	bool IsInPromotionZoneForEitherTeam(const Unit& u, int y, int Bx, int Rx) const;// 両チームの成りゾーン判定
};
