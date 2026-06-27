#pragma once
#include <Siv3D.hpp>
#include "Game/Game.h"
#include "Types/types.h"
class Game;

class GameRenderer
{
public:
	explicit GameRenderer(const Game& game) : m_game(game) {}

	// 描画のみ（副作用無し）
	void DrawBoard() const;// 盤面描画
	void DrawMoveRangeOverlay() const;// 移動可能範囲オーバーレイ描画
	void DrawUnits() const;// ユニット描画
	void UpdateHUD() const;// HUD 更新
	void DrawBackground() const;// 背景描画
	// 成りダイアログ描画
	void DrawPromotionDialog() const;
	//勝利画面描画
	void DrawVictoryScreen() const;
	
	// 画面ライト
	void ApplyScreenLight(double light) const;
	// ホーム画面描画
	void HomeScreenDraw() const;
	// 設定画面描画
	void SettingsScreenDraw() const;
	Point GridToScreen(Point boardPos)const;
	void ButtonToSwitch(RectF button, String player);

	//先手・後手 名前の設定
	void SetSenteName(const String& name) { senteName = U"先手:" + name; }
	void SetGoteName(const String& name) { goteName = U"後手:" + name; }
private:
	const Game& m_game;
	//先手・後手 名前
	String senteName = U"先手:青";//デフォルト先手名前
	String goteName = U"後手:赤";//デフォルト後手名前

	//ボタン描画補助
	void DrawButton(RectF Button, const String text) const;

	void DrawUnit(int32 px, const int32 py, bool selectedPiece, double Angle, int offX, int32 offY, const String unitName, ColorF color) const;// ユニット描画補助

};
