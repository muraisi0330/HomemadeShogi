#include "stdafx.h"
#include "Controller/GUIController.h"
#include "Game/Game.h"


void GUIController::UpdateInputs()
{
	RedHandMoveButton();
	BlueHandMoveButton();
}

void GUIController::RedHandMoveButton()
{
	
	int32 off = m_game.GetRedHandOffset();
	int32 maxRedIndex = -1;
	for (const auto& u : m_game.GetUnitList())
	{
		if (u.isOnBoard) continue;
		if (u.team == Team::Red && u.handIndex > maxRedIndex) maxRedIndex = u.handIndex;
	}
	if (RedCapturedPieceMoveButtonRight.leftPressed() && off <= 0)off++;// 右キーで右スクロール
	if (RedCapturedPieceMoveButtonRect.leftPressed() && maxRedIndex >= 0 && off > -TileSize * maxRedIndex)off--;
	m_game.SetRedHandOffset(off);
}
void GUIController::BlueHandMoveButton()
{
	int32 off = m_game.GetBlueHandOffset();
	int32 maxBlueIndex = -1;
	for (const auto& u : m_game.GetUnitList())
	{
		if (u.isOnBoard) continue;
		if (u.team == Team::Blue && u.handIndex > maxBlueIndex) maxBlueIndex = u.handIndex;
	}
	if (BlueCapturedPieceMoveButtonRight.leftPressed() && off <= 0)off++;// 右キーで右スクロール
	if (BlueCapturedPieceMoveButtonRect.leftPressed() && maxBlueIndex >= 0 && off > -TileSize * maxBlueIndex)off--;
	m_game.SetBlueHandOffset(off);
}

Point GridToScreen(Point boardPos)
{
	return boardPos * TileSize;
}
//盤面クリック検出
Point GUIController::BoardClickDetection()
{
	if (m_game.GetSelectedUnitIndex() < 0) return Point(-1, -1);
	const int32 offX = m_game.GetBoardOffsetX();
	const int32 offY = m_game.GetBoardOffsetY();
	for (int y = 0; y < BoardSize; ++y)
	{
		for (int x = 0; x < BoardSize; ++x)
		{
			Point pos = GridToScreen(Point{x,y});
			const Rect tileRect{ pos.x+ offX,pos.y + offY, TileSize };
			if (!tileRect.leftClicked()) continue;
			return Point(x,y);
		}
	}
	return Point(-1, -1);
}
// 盤面クリック処理
void GUIController::ProcessBoardClick()
{
	if (!m_game.IsGameActive())return;
	Point clickedPos = BoardClickDetection();
	m_game.ProcessBoardClick(clickedPos);
}
void GUIController::SelectUnit(int index, const Unit& u)
{
	if (!u.isOnBoard)	return;
	Point pos = GridToScreen(Point{ u.pos.x,u.pos.y });
	if (!Rect{ pos.x + m_game.GetBoardOffsetX(), pos.y + m_game.GetBoardOffsetY(), TileSize }
	.leftClicked()) return;
	m_game.SetSelectedUnitIndex(index);
	
}

// 手駒クリック検出
void GUIController::SelectUnitFromHand(int index, const Unit& u)
{
	if (u.isOnBoard)	return;
	int cursorPosX = Cursor::Pos().x;
	if (cursorPosX < LeftMarginIgnoreClickX) return;// 左端の余白クリック無視
	if (cursorPosX > RightMarginIgnoreClickX) return;// 右端の余白クリック無視
	const int handY = (u.team == Team::Blue) ? BlueHandY : RedHandY;
	const int handX = u.handIndex;
	const int posY = handY * TileSize;
	const int posX = TileSize + handX * TileSize + ((u.team == Team::Blue) ? m_game.GetBlueHandOffset() : m_game.GetRedHandOffset());
	if (!Rect{ posX,posY, TileSize }.leftClicked()) return;
	m_game.SetSelectedUnitIndex(index);

}

void GUIController::HandClickDetection()
{
	for (auto&& [i, u] : IndexedRef(m_game.GetUnitList()))
	{
		if (u.team != m_game.GetCurrentTeam()) continue;
		if (u.hasMoved) continue;
		if (!m_game.IsGameActive()) continue;
		if (!m_game.IsPromoteDialog()) continue;
		// 盤上の駒選択（boardOffset を考慮）
		SelectUnit(i, u);
		SelectUnitFromHand(i, u);
	}
}

// 手駒クリック処理
void GUIController::HandleUnitSelection()
{
	HandClickDetection();
	m_game.HandleUnitSelection();
}

void GUIController::EvaluatePromotions()
{
	// まず Game 側で自動成りなどの判定を行わせる
	m_game.EvaluatePromotions();
	// GUI 側で保留中の成りを検出してダイアログを描画・処理する
	int idx = m_game.GetPendingPromotionIndex();
	if (idx < 0) return;
	if (!m_game.IsGameActive()) return;
	// ダイアログ表示中はゲーム入力をブロックするためフラグを切る（元の実装に合わせる）
	if (m_game.IsPromoteDialog())
	{
		// ダイアログ表示開始
		m_game.SetIsPromoteDialog(false);
       
	}
	// ボタンで決定／キャンセル
	if (PromoteYesButtonRect.leftClicked()) {
		m_game.ApplyPromotionByIndex(idx);
		m_game.SetIsPromoteDialog(true);
		m_game.TurnEndDetermination();
	}
	if (PromoteNoButtonRect.leftClicked()) {
		m_game.CancelPromotionByIndex(idx);
		m_game.SetIsPromoteDialog(true);
		m_game.TurnEndDetermination();
	}
}


void GUIController::ClearGUI() {
	m_game.CheckVictoryCondition();
	if (m_game.IsGameActive()) return;
	if (m_game.GetDeathKingIndex() < 0) return;
	if (!RestartButtonRect.leftClicked()) return;
	m_game.ResetState();
}
// --- シーン移動ボタン群 ---
bool GUIController::GameSceneMoveButton()
{
	if(!SceneButtonRect.leftClicked())return false;
	return true;
}
bool GUIController::SettingsSceneMoveButton()
{
	if(!SettingsSceneButtonRect.leftClicked())return false;
	return true;
}
bool GUIController::StartSceneMoveButton()
{
	if (!SceneButtonRect.leftClicked())return false;
	return true;
}

bool GUIController::ButtonToSwitch(RectF button) {
	if (!button.leftClicked())return false;
	return true;
}
bool GUIController::ReturnToHomeButton() {
	if (!ReturnToHomeButtonRect.leftClicked())return false;
	if (m_game.IsGameActive()) return false;
	return true;
}
bool GUIController::ExitButton() {
	if (!ExitButtonRect.leftClicked())return false;
	return true;
}
bool GUIController::LicenseButton() {
	if (!LicenseButtonRect.leftClicked())return false;
	return true;
}


double GUIController::DrawEffectVolumeControl(double EffectVolume)
{
	// 効果音スライダー
	double vol = EffectVolume;
	SimpleGUI::Slider(U"SE       🔊 {:.1f}"_fmt(vol), vol, MinimumSliderValue, MaximumSoundEffectValue, Vec2{ SliderX, SliderY + SliderGap }, SliderW, SliderW);
	return vol;
}
double GUIController::DrawBgmVolumeControl(double BGMVolume)
{
	// BGMスライダー
	double bgmVol = BGMVolume;
	SimpleGUI::Slider(U"BGM   🔊 {:.1f}"_fmt(bgmVol), bgmVol, MinimumSliderValue, MaximumBGMValue, Vec2{ SliderX, SliderY + (SliderGap*2)}, SliderW, SliderW);
	return  bgmVol;
}
double GUIController::DrawLightControl(double light)
{
	// ライトスライダー
	double Light = light;
	SimpleGUI::Slider(U"ライト💡 {:.1f}"_fmt(Light), Light, MinimumSliderValue, MaximumLightValue, Vec2{ SliderX, SliderY }, SliderW, SliderW);
	return Light;
}
