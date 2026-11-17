#include "stdafx.h"
#include "Game/Game.h"
#include "Constants/CommonConstants.h"
#include "Game/Promotion.h"
#include "Game/Movement.h"
#include "Game/Drop.h"


// コンストラクタ: データベース / ユニットリストを初期化
// - 副作用: chipDB, unitDB, unitList を生成して初期化する
  Game::Game()
{
	// Impl 内の DB/リストを初期化（元のコードと同じ挙動）
	chipDB = CreateDefaultChipDB();
	unitDB = CreateDefaultUnitDB();
	unitList = CreateInitialUnitList();
	// 既存 Game.cpp で行っていた初期化（フォント / Asset 登録等）が
	// Main 側に残っているためここでは DB / List のみ初期化しています。
}
// ユニット選択解除処理
void Game::DeselectUnit(int x, int y)
{
	if (m_moveRange[y][x] >= 0)return;
	gameState.unitSelectionFlag = false;
}
//手駒の打ち処理
void Game::HandlePlaceFromHand(Unit* sel, int x, int y)
{
	if (m_moveRange[y][x] < 0)return;
	if(IsUnitAt(x, y))return;
	if(sel->isOnBoard)return;
	OrganizeYourPieces(sel);
	PlaceUnitFromHand(sel, x, y);
}
// 通常の移動 / 捕獲処理
void Game::HandleNormalMoveOrCapture(Unit* sel, int x, int y)
{
	if (m_moveRange[y][x] < 0)return;
	if (!IsUnitAt(x, y) && !sel->isOnBoard)return;
	// 通常の移動 / 捕獲処理
	HandleMoveOrCaptureInternal(sel, x, y, gameState.currentTeam);
	
}
//選択解除
void Game::ClearSelectionMoveRange()
{
	ClearMoveRange();
	gameState.selectedUnitIndex = ResetSelectedIndex;
}

// チーム交代処理
void Game::TeamSwitching(int x, int y)
{
	if (m_moveRange[y][x] < 0)return;
	TurnEndDetermination();
}

// 盤面クリック処理ルーチン
void Game::ProcessBoardClick(Point clickedPos)
{
	// 選択ユニットがない場合は早期リターン
	if (gameState.selectedUnitIndex < 0) return;
	if (clickedPos == ResetSelectedTarget)return;
	int32 x = clickedPos.x;
	int32 y = clickedPos.y;
	Unit* sel = &unitList[gameState.selectedUnitIndex];
	DeselectUnit(x, y);
	HandlePlaceFromHand(sel, x, y);
	HandleNormalMoveOrCapture(sel, x, y);
	TeamSwitching(x, y);
	ClearSelectionMoveRange();
}
// 駒選択検出：盤上の駒をクリックしたら選択状態にする
void Game::SelectBoardUnit()
{// 盤上の駒選択（boardOffset を考慮）
	if (gameState.selectedUnitIndex < 0) return;
	Unit& sel = unitList[gameState.selectedUnitIndex];
	if (!sel.isOnBoard)	return;
		ClearMoveRange();
		gameState.selectedUnitPosition.x = sel.pos.x;
		gameState.selectedUnitPosition.y = sel.pos.y;
		int cost = chipDB[map[sel.pos.y][sel.pos.x]].cost;
		// 移動力 = 駒の speed + 現在マスのコスト
		{
			Movement::Context ctx;
			ctx.moveRange = m_moveRange;
			ctx.map = &map;
			ctx.chipDB = &chipDB;
			ctx.unitList = &unitList;
			ctx.selectedUnitPosition.x = gameState.selectedUnitPosition.x;
			ctx.selectedUnitPosition.y = gameState.selectedUnitPosition.y;
			ctx.moveDirection = gameState.moveDirection;
			Movement::ComputeMoveRange(m_moveRange);
			Movement::ComputeMoveRangeImpl(ctx, sel.pos.y, sel.pos.x, unitDB[sel.type].speed + cost, sel.type, 0);
		}
		Movement::ClearMoveRangeForCurrentTeam(m_moveRange, unitList, gameState.currentTeam);
		gameState.unitSelectionFlag = true;
}
// 駒選択検出：手駒をクリックしたら選択状態にする
void Game::SelectHandUnit() {
	if (gameState.selectedUnitIndex < 0) return;
	Unit& u = unitList[gameState.selectedUnitIndex];
	if (u.isOnBoard)	return;
		ClearMoveRange();
		{
			Drop::ComputeDropMoveRange(m_moveRange, unitList, u.type, u.team);
		}
		gameState.unitSelectionFlag = true;
}


// 駒選択検出ルーチン
void Game::HandleUnitSelection()
{
	SelectBoardUnit();
	SelectHandUnit();
}

// 成り評価ルーチン
void Game::EvaluatePromotions()
{
	for (auto&& u : unitList)
	{
		if (!ApplyAutoPromotion(u)) continue;
		u.type = ToPromoted(u.type);
		u.promoteFlag = false;
		TurnEndDetermination();
		return;
	}
}

// 自動成り適用（強制成り）
bool Game::ApplyAutoPromotion(Unit& u)
{
	if (!u.isOnBoard) return false;
	const int y = u.pos.y;
	if(!IsAutoPromotionRequired(u.type,u.team,y))return false;
	
		return true;
}

// ゲーム状態のリセット
void Game::ResetState() noexcept
{
	// 基本ステートのリセット
	gameState = GameStateData{};
	ResetTimer();
	// 移動可能エリアをクリア
	// 各ユニットの状態を明示的に初期化
	auto initialUnits = CreateInitialUnitList();
	unitList = initialUnits; // まるごとコピー（または代入）
	// chipDB / unitDB は必要なら再構築（今回は CreateDefault を用いる）
	chipDB = CreateDefaultChipDB();
	unitDB = CreateDefaultUnitDB();
	InvokeEvent(GameEvent::GameStart);
}
// 経過時間の更新（GUI 側から呼ばれる想定）
void Game::ResetTimer() {
	m_stopwatch.restart();
}

// ユニットの位置を変更する補助
void Game::MoveUnit(Unit& u, int x, int y) const noexcept
{
	u.pos.x = x;
	u.pos.y = y;
}

// Getters: 内部状態の読み取り専用アクセス
const Array<Unit>& Game::GetUnitList() const noexcept { return unitList; }
const Grid<int32>& Game::GetMap() const noexcept { return map; }
const HashTable<int32, ChipInfo>& Game::GetChipDB() const noexcept { return chipDB; }
const HashTable<UnitType, UnitInfo>& Game::GetUnitDB() const noexcept { return unitDB; }
int32 Game::GetBoardOffsetX() const noexcept { return kBoardOffsetX; }
int32 Game::GetBoardOffsetY() const noexcept { return kBoardOffsetY; }
double Game::GetElapsedTime() const noexcept { return gameState.elapsedTime; }
// 指定マスの移動可能値を返す（範囲外は -1）
// - note: moveRangeArea は -1 = 不可、>=0 = 残り移動力
int32 Game::GetMoveRange(int y, int x) const noexcept
{
	if (y < 0) return ResetSelectedIndex;
	if(y >= BoardSize)return ResetSelectedIndex;
	if(x < 0)return ResetSelectedIndex;
	if(x >= BoardSize)return ResetSelectedIndex;
	return m_moveRange[y][x];
}

bool Game::IsUnitSelected() const noexcept { return gameState.unitSelectionFlag; }
bool Game::IsGameActive() const noexcept { return gameState.isGameActive; }
Team Game::GetCurrentTeam() const noexcept { return gameState.currentTeam; }

// 選択 / 解除 API（UI から呼ぶ）
// - Select: selectedUnitIndex を設定し isUnitSelected を有効にする
void Game::SelectUnitByIndex(int index)
{
	gameState.selectedUnitIndex = index;
	gameState.unitSelectionFlag = true;
	gameState.selectedUnitPosition = unitList[index].pos;
}

// 手駒から盤上に配置する API（検証用に public）
// - 戻り値: 成功なら true
bool Game::PlaceUnitFromHand(int unitIndex, int x, int y)
{
	if (unitIndex < 0) return false;
	if(unitIndex >= static_cast<int>(unitList.size()))return false;
	auto& u = unitList[unitIndex];
	if (u.isOnBoard) return false;
	u.isOnBoard = true;
	u.pos.x = x;
	u.pos.y = y;
	u.hasMoved = true;
	return true;
}

int32 Game::GetBlueHandOffset() const noexcept
{
	return gameState.blueHandOffset;
}

int32 Game::GetRedHandOffset() const noexcept
{
	return gameState.redHandOffset;
}
int32 Game::GetSelectedUnitIndex() const noexcept
{
	return gameState.selectedUnitIndex;
}
void Game::SetSelectedUnitIndex(int32 index) noexcept
{
	gameState.selectedUnitIndex = index;
}

void Game::SetBlueHandOffset(int32 v) noexcept
{
	gameState.blueHandOffset = v;
}

void Game::SetRedHandOffset(int32 v) noexcept
{
	gameState.redHandOffset = v;
}

// ゲーム有効フラグの設定（const 環境から呼ばれることを想定）
// - isGameActive は mutable のため const メソッド内で変更可能
void Game::SetGameActive(bool a) noexcept
{
	gameState.isGameActive = a;
}

// 移動範囲のリセット（内部ユーティリティ）
void Game::ClearMoveRange()
{
	for (int y = 0; y < BoardSize; ++y)
	{
		for (int x = 0; x < BoardSize; ++x)
		{
			if (m_moveRange[y][x] == ResetSelectedIndex) continue;
			m_moveRange[y][x] = ResetSelectedIndex;
		}
	}
}

// 現在手の駒だけをリセット（選択中のユニットを残す）
void Game::ClearMoveRangeForCurrentTeam()
{
	for (auto& u : unitList)
	{
		if (gameState.currentTeam != u.team) continue;
		if (!u.isOnBoard) continue;
		m_moveRange[u.pos.y][u.pos.x] = ResetSelectedIndex;
	}
}

// マップ範囲内判定
bool Game::IsInsideMap(int x, int y) const
{
	if (x < 0) return false;
	if (y < 0) return false;
	if (x >= map.width()) return false;
	if (y >= map.height()) return false;
	return true;
}

// タイル通過判定：選択ユニットの元位置は通過可能とみなす
bool Game::IsTilePassable(int x, int y) const
{
	for (auto& u : unitList)
	{
		if (u.pos.x != x) continue;
		if (u.pos.y != y) continue;
		if (!u.isOnBoard) continue;
		if (gameState.selectedUnitPosition.x != x ) continue;
		if(	gameState.selectedUnitPosition.y != y)  continue;
		return false;
	}
	return true;
}

// 指定座標にユニットがいるか
bool Game::IsUnitAt(int x, int y) const
{
	for (auto&& u : unitList)
	{
		if (u.pos.x != x) continue;
		if (u.pos.y != y) continue;
		if (!u.isOnBoard) continue;
		return true;
	}
	return false;
}

// 手駒を盤上に置く内部ヘルパー（ポインタ引数）
// - sel が nullptr の場合は何もしない
void Game::PlaceUnitFromHand(Unit* sel, int x, int y)
{
	if (!sel) return;
	if (IsUnitAt(x, y)) return;
	sel->isOnBoard = true;
	sel->promoteFlag = false;
	sel->pos.x = x;
	sel->pos.y = y;
	InvokeEvent(GameEvent::PieceMoved);
	sel->hasMoved = true;
}

// 指定座標に自軍のユニットがいるか（現在手の判定に使う）
// - 読み取り専用
bool Game::IsUnitOfCurrentTeamAt(int x, int y, Team team) const
{
	for (auto&& u : unitList)
	{
		if (u.pos.x != x) continue;
		if (u.pos.y != y) continue;
		if (team != u.team) continue;
		if (!u.isOnBoard) continue;
		return true;
	}
	return false;
}
// 持ち駒整理ヘルパー
void Game::OrganizeYourPieces(Unit* sel) {
	for (auto&& k : unitList)
	{
		if (k.handIndex <= sel->handIndex) continue;
		if (gameState.currentTeam != k.team) continue;
		if (k.isOnBoard) continue;
		k.handIndex--;
	}
}

void Game::RecalculateHandIndices(Unit& sel) {
	int32 handIndexCounter = ResetSelectedIndex;
		for (auto& q : unitList)
		{
			if (q.isOnBoard) continue;
			if (q.team != sel.team) continue;
			handIndexCounter++;
		}
		sel.handIndex = handIndexCounter;
}
//移動
void Game::ComputeMoveRange(Unit* sel, int x, int y)
{
	if (!sel->isOnBoard) OrganizeYourPieces(sel);
	sel->pos.x = x;
	sel->pos.y = y;
	sel->promoteFlag = true;
	InvokeEvent(GameEvent::PieceMoved);
	sel->hasMoved = true;
	gameState.unitSelectionFlag = false;
}
//捕獲を
void Game::ComputeDropMoveRange(Unit* sel, int x, int y)
{
	for (auto&& k : unitList)
	{
		if (k.pos.x != x) continue;
		if (k.pos.y != y) continue;
		if (!k.isOnBoard) continue;
		if (gameState.currentTeam == k.team) continue;
		k.isOnBoard = false;
		k.team = gameState.currentTeam;
		k.type = ToDemoted(k.type);
		// 持ち駒インデックスを再計算
		RecalculateHandIndices(k);
	}
}

// 移動 / 捕獲を実行する内部ヘルパー
// - sel が nullptr または自軍なら何もしない
// - 敵駒を取ると持ち駒に変換（team と handIndex を更新）
void Game::HandleMoveOrCaptureInternal(Unit* sel, int x, int y, Team activeTeam)
{
	if (!sel) return;
	if (IsUnitOfCurrentTeamAt(x, y, activeTeam)) return;
	ComputeMoveRange(sel, x, y);
	// 敵駒を捕獲して持ち駒に変換
	ComputeDropMoveRange(sel, x, y);
}

// 成り判定ユーティリティ（読み取り専用）
// - 条件を満たせば true
bool Game::IsPromotionEligible(const Unit& unit, Team team, int y, int x) const
{
	if (unit.team != team) return false;
	if (!unit.promoteFlag) return false;
	if (!unit.isOnBoard) return false;
	if (y > x) return false;
	return true;
}

void Game::CheckVictoryCondition() 
{
	if (!gameState.isGameActive) return;
	for (const auto&& [i, u] : IndexedRef(unitList))
	{
		if (u.isOnBoard)continue;
		if(u.type != UnitType::King &&
			u.type != UnitType::King1)continue;
		    InvokeEvent(GameEvent::GameWon);
			gameState.isGameActive = false;
			deathKingIndex = i;
			return;
	}
}

int32 Game::GetDeathKingIndex() const noexcept
{
	return deathKingIndex;
}

// 任意チームの成りゾーン判定ユーティリティ（読み取り専用）
bool Game::IsInPromotionZoneForEitherTeam(const Unit& u, int y, int Bx, int Rx) const
{
	if (IsPromotionEligible(u, Team::Blue, y, Bx)) return true;
	if (IsPromotionEligible(u, Team::Red, Rx, y))  return true;
	return false;
}
bool Game::IsOwnTurn(Team team) {
	if(gameState.currentTeam != team)return false;
	return true;
}

// 経過時間更新：ゲーム進行中のみ elapsedTime を加算
void Game::UpdateElapsedTime() {
	if (!IsGameActive()) {
		m_stopwatch.pause(); // ゲーム終了時は止める
		return;
	}
	gameState.elapsedTime = m_stopwatch.sF(); // 秒数を float で取得
}
bool Game::HasAnyUnitMoved()
{
	for (auto& u : unitList)
	{
		if (u.team != gameState.currentTeam) continue;
		if (!u.hasMoved) continue;
		return true;
	}
	return false;
}
void Game::ResetUnitsMoveFlag()
{
	for (auto& u : unitList)
	{
		if (u.team != gameState.currentTeam) continue;
		u.hasMoved = false;
	}
}
void Game::SwitchCurrentTeam()
{
	gameState.currentTeam = (gameState.currentTeam == Team::Blue) ? Team::Red : Team::Blue;
	gameState.moveDirection = (gameState.currentTeam == Team::Blue) ? 1 : -1;
}
// ターン終了判定：any ユニットが移動済みならターン交代
void Game::TurnEndDetermination() 
{
	if (GetPendingPromotionIndex() >= 0) return;
	if (!HasAnyUnitMoved()) return;
			// ユニットの移動フラグをリセット
	        ResetUnitsMoveFlag();
			// 現在手を切り替え
			SwitchCurrentTeam();
			// 選択状態を完全にクリア（AIの連続実行防止）
			gameState.selectedUnitIndex = ResetSelectedIndex;
			gameState.unitSelectionFlag = false;
}

// 保留中の成り対象を返す（最初に見つかったインデックス、該当なしは -1）
int Game::GetPendingPromotionIndex() const noexcept
{
	for (const auto&& [i, u] : IndexedRef(unitList))
	{
		if (!u.isOnBoard) continue;
		if (!u.promoteFlag) continue;

		// 成り可能種かどうか
		bool promotable = IsDemoted(u.type);
		if (!promotable) continue;

		// 成りゾーン判定（既存ロジックに合わせる）
		const int y = u.pos.y;
		bool inZone = (u.team == Team::Blue) ? (y <= kBluePromoteLine) : (y >= BoardSize - kRedPromoteLine);
		if (!inZone) continue;
		return i;
	}
	return ResetSelectedIndex;
}
// 指定インデックスの成りを適用（UI が「はい」を押したとき）
void Game::ApplyPromotionByIndex(int index) noexcept
{
	if (index < 0 ) return;
	if(index >= static_cast<int>(unitList.size())) return;
	auto& u = unitList[index];
	if (!u.isOnBoard) return;
	u.type = ToPromoted(u.type);
	u.promoteFlag = false;
	InvokeEvent(GameEvent::PieceMoved);
}

// 指定インデックスの成り要求をキャンセル（UI が「いいえ」を押したとき）
void Game::CancelPromotionByIndex(int index) noexcept
{
	if (index < 0 || index >= static_cast<int>(unitList.size())) return;
	auto& u = unitList[index];
	u.promoteFlag = false;
}

// isPromoteDialog の getter/setter（GUI がダイアログ表示中フラグを制御する）
bool Game::IsPromoteDialog() const noexcept { return gameState.isPromoteDialog; }
void Game::SetIsPromoteDialog(bool v) noexcept {
	gameState.isPromoteDialog = v;
	if (v)return;
	InvokeEvent(GameEvent::Promoted);
}







// 選択ユニットを指定座標に移動または捕獲を実行する API
// - 戻り値: 成功なら true
bool Game::ExecuteMoveOrCaptureSelected(int x, int y) noexcept
{
	// 選択状態を確認
	if (gameState.selectedUnitIndex < 0) return false;
	if (!IsInsideMap(x, y)) return false;

	Unit* sel = &unitList[gameState.selectedUnitIndex];

	// 持ち駒の場合は placeFromHand（既存の挙動）を使う
	if (!sel->isOnBoard)
	{
		OrganizeYourPieces(sel);
		PlaceUnitFromHand(sel, x, y);
		// clear selection state as processBoardClick でも行っているのと同等にする
		ClearMoveRange();
		gameState.selectedUnitIndex = ResetSelectedIndex;
		gameState.unitSelectionFlag = false;
		return true;
	}
	// 盤上の駒の場合は内部の handleMoveOrCapture を呼んで捕獲処理を正しく行う
	HandleMoveOrCaptureInternal(sel, x, y, gameState.currentTeam);
	// handleMoveOrCapture は isUnitSelected/hasMoved などを更新するが、
	// 選択インデックスは明示的に解除しておく
	ClearMoveRange();
	gameState.selectedUnitIndex = ResetSelectedIndex;
	gameState.unitSelectionFlag = false;
	return true;
}

void Game::SetOnGameEvent(std::function<void(GameEvent)> callback)
{
	m_onGameEvent = callback;
}

void Game::InvokeEvent(GameEvent e) const
{
	if (m_onGameEvent) m_onGameEvent(e);
}
