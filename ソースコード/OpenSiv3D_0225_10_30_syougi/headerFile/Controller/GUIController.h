#pragma once
#include <Siv3D.hpp>
#include "Types/types.h"

class Game;

class GUIController
{
public:

	explicit GUIController(Game& game) : m_game(game){}

	// 入力・状態更新（Game の public API を呼ぶ）
	void UpdateInputs();            // キー等の入力検出（現状薄いラッパー）
	void ProcessBoardClick();
	void HandleUnitSelection();
	void EvaluatePromotions();
	void ClearGUI();
	//持ち駒の動きボタン
	void RedHandMoveButton();
	void BlueHandMoveButton();
	//盤面クリック検出
	Point BoardClickDetection();
	//駒クリック検出
	void HandClickDetection();
	void SelectUnit(int index, const Unit& u);
	void SelectUnitFromHand(int index, const Unit& u);
	//シーンの移動ボタン
	bool GameSceneMoveButton();// ゲームシーン移動ボタン
	bool SettingsSceneMoveButton();// 設定シーン移動ボタン
	bool StartSceneMoveButton();// ホームシーン移動ボタン
	bool ButtonToSwitch(RectF button);
	bool ReturnToHomeButton();
	bool ExitButton();
	bool LicenseButton();

	// GUI（スライダー・ダイアログ表示など）を描画
	double DrawEffectVolumeControl(double EffectVolume);// 効果音スライダー等
	//BGM音量スライダー等
	double DrawBgmVolumeControl(double BGMVolume);
	// ライトスライダー
	double DrawLightControl(double light);


private:
	Game& m_game;
	//スライダーの最小値
	static constexpr double MinimumSliderValue = 0.0f;
	//効果音の最大値
	static constexpr double MaximumSoundEffectValue = 10.0f;
	//BGMの最大値
	static constexpr double MaximumBGMValue = 10.0f;
	//ライトの最大値
	static constexpr double MaximumLightValue = 0.9f;



};
