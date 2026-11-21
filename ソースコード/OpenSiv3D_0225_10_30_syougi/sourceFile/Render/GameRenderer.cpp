#include "stdafx.h"
#include "Render/GameRenderer.h"
#include "Game/Game.h"
#include "Types/types.h"



// 描画専用の「降格」マップ（持ち駒表示用）
static UnitType DemoteForDisplay(UnitType t)
{
	switch (t)
	{
	case UnitType::Pawn1:   return UnitType::Pawn;
	case UnitType::Lance1:  return UnitType::Lance;
	case UnitType::Knight1: return UnitType::Knight;
	case UnitType::Silver1: return UnitType::Silver;
	case UnitType::Bishop1: return UnitType::Bishop;
	case UnitType::Rook1:   return UnitType::Rook;
	default:                return t;
	}
}

Point GameRenderer::GridToScreen(Point boardPos) const
{
	return boardPos * TileSize;
}
void GameRenderer::DrawButton(RectF Button, const String text) const
{
	int32 x = static_cast<int32>(Button.x + Button.w / 2.0);
	int32 y = static_cast<int32>(Button.y + Button.h / 2.0);
	ColorF colo = ButtonColor;
	if (Button.mouseOver())
	{
		colo = ButtonHoverColor;
		// マウスカーソルを手の形にする
		Cursor::RequestStyle(CursorStyle::Hand);
	}
	Button.draw(Palette::Lightgray).drawFrame(FrameThickness, colo);
	FontAsset(U"UnitNameFont")(text).drawAt(x, y, DefaultTextColor);
}

void GameRenderer::DrawBoard() const
{
	const Grid<int32>& map = m_game.GetMap();
	const int32 offX = m_game.GetBoardOffsetX();
	const int32 offY = m_game.GetBoardOffsetY();
	// 背景 (既存の意図に合わせた簡易描画)
	
	TextureAsset(U"将棋台").drawAt(ShogiTableCenterX, ShogiTableCenterY);
	// 星印の描画
	Circle{ ShogiTableCenterX + Star, ShogiTableCenterY + Star,Starsize }.draw(ShogiTableLineColor);
	Circle{ ShogiTableCenterX + Star, ShogiTableCenterY - Star,Starsize }.draw(ShogiTableLineColor);
	Circle{ ShogiTableCenterX - Star, ShogiTableCenterY + Star,Starsize }.draw(ShogiTableLineColor);
	Circle{ ShogiTableCenterX - Star, ShogiTableCenterY - Star,Starsize }.draw(ShogiTableLineColor);
	
	for (auto y : step(map.height()))
	{
		for (auto x : step(map.width()))
		{
			ColorF color = ColorF{ 0,0,0,0 };
			Point pos = GridToScreen(Point(x, y));
			Rect{ pos.x + offX, pos.y + offY, TileSize }.draw(color).drawFrame(ShogiTableFrameThickness, ShogiTableLineColor);
		}
	}
}

void GameRenderer::DrawMoveRangeOverlay() const
{
	if (!m_game.IsUnitSelected()) return;

	const int32 offX = m_game.GetBoardOffsetX();
	const int32 offY = m_game.GetBoardOffsetY();

	for (int y = 0; y < BoardSize; ++y)
	{
		for (int x = 0; x < BoardSize; ++x)
		{
			if (m_game.GetMoveRange(y, x) < 0) continue;
			Point pos = GridToScreen(Point(x, y));
			Rect{ pos.x + offX, pos.y + offY, TileSize }.draw(DrawMoveRangeColor);
		}
	}
}

void GameRenderer::DrawUnit(int32 px,const int32 py, bool selectedPiece, double Angle,int offX,int32 offY, const String unitName, ColorF color) const
{
	Point pos = GridToScreen(Point(px, py));
	pos.x += offX + PieceTextAdjust;
	pos.y += offY + PieceTextAdjust;
	if(selectedPiece)TextureAsset(U"駒").scaled(SelectedPieceScale).rotated(Angle).drawAt(pos.x, pos.y, SelectedPieceColor);
	TextureAsset(U"駒").rotated(Angle).drawAt(pos.x, pos.y);
	Mat3x2 mat = Mat3x2::Identity();
	mat = mat * Mat3x2::Rotate(Angle);
	mat = mat * Mat3x2::Translate(pos.x, pos.y);
	Transformer2D transformer{ mat };
	FontAsset(U"UnitNameFont3")(unitName).drawAt(0, 0, color);
}

void GameRenderer::DrawUnits() const
{
	// コピーを作っておく（const HashTable から operator[] を使うと実装依存でコンパイルエラーになる場合があるため）
	auto unitDB = m_game.GetUnitDB();
	const auto& unitList = m_game.GetUnitList();

	const int32 offX = m_game.GetBoardOffsetX();
	const int32 offY = m_game.GetBoardOffsetY();
	const int32 blueHandOffset = m_game.GetBlueHandOffset();
	const int32 redHandOffset = m_game.GetRedHandOffset();
	for (const auto&& [i, u] : IndexedRef(unitList))
	{
		bool selectedPiece = false;
		ColorF color = PieceTextColor;
		if (m_game.GetSelectedUnitIndex() == i)
		{
			selectedPiece = true;
		}
		if (u.isOnBoard)
		{
			String unitName = unitDB[u.type].name;
			if (IsPromoted(u.type))
			{
				color = PromotedPieceTextColor;
			}
			const int px = u.pos.x;
			const int py = u.pos.y;
			if (u.team == Team::Red)
			{
				DrawUnit(px, py, selectedPiece, GotePieceAngle, offX, offY, unitName, color);

			}
			else
			{
				DrawUnit(px, py, selectedPiece, 0_deg, offX, offY, unitName, color);
			}
		}
		else
		{
			UnitType displayType = DemoteForDisplay(u.type);
			String unitName = unitDB[displayType].name;

			if (u.team == Team::Red)
			{
				const int y1 = RedHandY;
				const int x1 =  u.handIndex;
				DrawUnit(x1 , y1, selectedPiece, GotePieceAngle, redHandOffset + TileSize, 0, unitName, color);
			}
			else
			{
				const int y1 = BlueHandY;
				const int x1 = u.handIndex;
				DrawUnit(x1, y1, selectedPiece, 0_deg, blueHandOffset + TileSize, 0, unitName, color);
			}
		}
	}
	
	DrawButton(RedCapturedPieceMoveButtonRight, U">");
	DrawButton(RedCapturedPieceMoveButtonRect, U"<");
  
	DrawButton(BlueCapturedPieceMoveButtonRight, U">");
	DrawButton(BlueCapturedPieceMoveButtonRect, U"<");
}
void GameRenderer::DrawPromotionDialog() const
{
	// GUI 側で保留中の成りを検出してダイアログを描画・処理する
	int idx = m_game.GetPendingPromotionIndex();
	if (idx < 0) return;
	if (!m_game.IsGameActive()) return;
	// UI 描画：テクスチャと文字
	TextureAsset(U"掛け軸").scaled(KakejikuScale).drawAt(PromoteDialogX, PromoteDialogY);
	FontAsset(U"UnitNameFont")(U"成る").drawAt(PromoteDialogX, PromoteDialogFontY, DefaultTextColor);
	DrawButton(PromoteYesButtonRect, U"はい");
	DrawButton(PromoteNoButtonRect, U"いいえ");
}
void GameRenderer::DrawVictoryScreen() const
{
	if (m_game.IsGameActive()) return;
	if (m_game.GetDeathKingIndex() < 0) return;
	auto& u = m_game.GetUnitList()[m_game.GetDeathKingIndex()];
	// 描画
	Rect{ WinRectX,WinRectY,WinRectW,WinRectH }.draw();
	FontAsset(U"WinFont")((u.team == Team::Red) ? goteName + U"の勝利" : senteName + U"の勝利").drawAt(WinFontX, WinFontY, DefaultTextColor);
	DrawButton(RestartButtonRect, U"再スタート");
	DrawButton(ReturnToHomeButtonRect, U"ホームに戻る");
	
}
void GameRenderer::DrawBackground() const
{
	TextureAsset(U"畳").draw(TatamiTilePositions[0]);
	TextureAsset(U"畳").draw(TatamiTilePositions[1]);
	TextureAsset(U"畳").rotated(TatamiTileRotation).drawAt(TatamiTilePositions[2]);
	TextureAsset(U"畳").rotated(TatamiTileRotation).drawAt(TatamiTilePositions[3]);
	TextureAsset(U"畳").draw(TatamiTilePositions[4]);
	TextureAsset(U"畳").draw(TatamiTilePositions[5]);
}

void GameRenderer::ApplyScreenLight(double light) const
{
	Rect{ 0, 0,Scene::Size() }.draw(ColorF{ 0, 0, 0, 0.9 - light });
}
void GameRenderer::HomeScreenDraw() const
{
	// 背景
	TextureAsset(U"ホーム背景").scaled(BackgroundAspectCorrection).draw(0, 0);
	// タイトル
	TextureAsset(U"タイトル").scaled(TitleScale).drawAt(Title);
	FontAsset(U"WinFont")(U"将棋").drawAt(Title, DefaultTextColor);
	DrawButton(SceneButtonRect, U"ゲーム開始");
	DrawButton(SettingsSceneButtonRect, U"設定");
	DrawButton(ExitButtonRect, U"終了");
	//ライセンス
	DrawButton(LicenseButtonRect, U"クレジット");
}
void GameRenderer::SettingsScreenDraw() const
{
	// 背景
	TextureAsset(U"ホーム背景").scaled(BackgroundAspectCorrection).draw(0, 0);
	// タイトル
	TextureAsset(U"タイトル").scaled(TitleScale).drawAt(Title);
	FontAsset(U"WinFont")(U"設定").drawAt(Title, DefaultTextColor);
	DrawButton(SceneButtonRect, U"ホームへ戻る");
}

//クレジット画面描画
void GameRenderer::CreditScreenDraw() const
{
	// 背景
	TextureAsset(U"ホーム背景").scaled(BackgroundAspectCorrection).draw(0, 0);
	// タイトル
	TextureAsset(U"タイトル").scaled(TitleScale).drawAt(Title);
	FontAsset(U"CreditFont")(U"クレジット").drawAt(Title, DefaultTextColor);
	DrawButton(SceneButtonRect, U"ホームへ戻る");
	TextureAsset(U"和紙").scaled(0.2,0.12).drawAt(CreditTextPosition.x, CreditTextPosition.y + CreditTextGap * 1.5);
	FontAsset(U"UnitNameFont")(U"制作者: 村石　裕音").drawAt(CreditTextPosition, DefaultTextColor);
	FontAsset(U"UnitNameFont")(U"BGM: 甘茶の音楽工房").drawAt(CreditTextPosition.x, CreditTextPosition.y + CreditTextGap, DefaultTextColor);
	FontAsset(U"UnitNameFont")(U"SE: 効果音ラボ").drawAt(CreditTextPosition.x, CreditTextPosition.y + CreditTextGap * 2, DefaultTextColor);
	FontAsset(U"UnitNameFont")(U"使用ライブラリ: Siv3D").drawAt(CreditTextPosition.x, CreditTextPosition.y + CreditTextGap*3, DefaultTextColor);
	DrawButton(LicenseButtonRect, U"ライセンス: Siv3D");
}

//フェードイン時の描画処理
void GameRenderer::DrawFadeIn(double alpha) const
{
	double speed = FadeInSpeed *  alpha;
	double speedZ = (speed < MaxFadeInSpeed) ? 0 : speed - MaxFadeInSpeed;

	//桜の襖が徐々に開く演出
	TextureAsset(U"桜の襖").draw(FadeInSakuraPositions[1].x - speedZ, 0);
	TextureAsset(U"桜の襖").mirrored(true).draw(FadeInSakuraPositions[3].x + speedZ, 0);
	TextureAsset(U"桜の襖").draw(FadeInSakuraPositions[0].x - speed, 0);
	TextureAsset(U"桜の襖").mirrored(true).draw(FadeInSakuraPositions[2].x + speed, 0);
	
}
//フェードアウト時の描画処理
void GameRenderer::DrawFadeOut(double alpha) const
{
	int speed = FadeOutSpeed *  alpha ;
	if (speed > MaxFadeOutSpeed)
	{
		speed = MaxFadeOutSpeed;
	}
	TextureAsset(U"桜の襖").mirrored(true).draw(FadeOutSakuraPositions[0].x - (speed), 0);
	TextureAsset(U"桜の襖").draw(FadeOutSakuraPositions[2].x + (speed), 0);
	TextureAsset(U"桜の襖").mirrored(true).draw(FadeOutSakuraPositions[1].x - (speed), 0);
	TextureAsset(U"桜の襖").draw(FadeOutSakuraPositions[3].x + (speed), 0);
}

// HUDの更新
void GameRenderer::UpdateHUD() const
{
	// 基本表示
	TextureAsset(U"看板").scaled(SignScale).rotated(VerticalSignRotation).drawAt(HUDSign);// 看板の表示
	FontAsset(U"UnitNameFont")( senteName).drawAt(TextSente);
	FontAsset(U"UnitNameFont")(goteName).drawAt(TextGote);

	FontAsset(U"UnitNameFont")(U"経過時間").drawAt(ElapsedTimePos);
	FontAsset(U"UnitNameFont")(U"{:.2f}"_fmt(m_game.GetElapsedTime())).drawAt(ElapsedTimePos.x, ElapsedTimePos.y + ElapsedTimeGap);

	// 現在手の表示
	TextureAsset(U"看板").scaled(CurrentTurnScale).drawAt(CurrentTurnPos);
	FontAsset(U"UnitNameFont")((m_game.GetCurrentTeam() == Team::Blue) ? U"先手" : U"後手").drawAt(CurrentTurnPos);
	
	
}
void GameRenderer::ButtonToSwitch(RectF button, String player) {
	DrawButton(button, player);
}



