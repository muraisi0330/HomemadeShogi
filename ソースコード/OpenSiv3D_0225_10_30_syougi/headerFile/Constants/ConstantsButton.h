#pragma once
#include <Siv3D.hpp>

//枠線の太さ
constexpr int32 FrameThickness = 2;// 枠線の太さ

//リスタートボタン
constexpr int32 RestartButtonX = 400;        // リスタートボタンX座標
constexpr int32 RestartButtonY = 400;        // リスタートボタンY座標
constexpr Point RestartButtonSize = { 200 ,50 };       // リスタートボタンサイズ
constexpr int32 RestartButtonGap = 60;
constexpr RectF RestartButtonRect = RectF(Arg::center(RestartButtonX, RestartButtonY), RestartButtonSize);// リスタートボタンRect
constexpr RectF ReturnToHomeButtonRect = RectF(Arg::center(RestartButtonX, RestartButtonY + RestartButtonGap), RestartButtonSize);// リスタートボタンRect


// 成りダイアログ関連定数
constexpr int32 PromoteDialogX = 710;         // 成りダイアログX座標
constexpr int32 PromoteDialogY = 420;         // 成りダイアログY座標
constexpr int32 PromoteDialogFontY = 350;	  // 成りダイアログフォントY座標
constexpr Point PromoteYesButton = { 710,400 };//成りのボタンで決定X座標
constexpr int32 PromoteButtonGap = 50;// 成りのボタンで決定とキャンセルの間隔
constexpr Point PromoteButtonSize = { 100 ,40 };// 成りのボタンで決定とキャンセルのサイズ
constexpr RectF PromoteYesButtonRect = RectF(Arg::center(PromoteYesButton), PromoteButtonSize);// 成りのボタンで決定Rect
constexpr RectF PromoteNoButtonRect = RectF(Arg::center(PromoteYesButton.x, PromoteYesButton.y + PromoteButtonGap), PromoteButtonSize);// 成りのボタンでキャンセルRect
// シーン移動ボタン
constexpr Point SceneButton = { 400,470 };// シーン移動ボタンのX座標
constexpr Point SceneButtonSize = { 200,50 };// シーン移動ボタンのサイズ
constexpr int32 SceneButtonGap = 60;// シーン移動ボタンの間隔
constexpr RectF SceneButtonRect = RectF(Arg::center(SceneButton), SceneButtonSize);// シーン移動ボタンRect
constexpr RectF SettingsSceneButtonRect = RectF(Arg::center(SceneButton.x, SceneButton.y + SceneButtonGap), SceneButtonSize);// 設定シーン移動ボタンRect

//スライダー
constexpr int32 SliderX = 250;// スライダーのX座標
constexpr int32 SliderY = 300;// スライダーのY座標
constexpr int32 SliderW = 150;// スライダーの幅
constexpr int32 SliderGap = 50;// スライダーの間隔

// プレイヤー設定ボタン
constexpr Point PlayerSettingsButton = { 300,500 };// シーン移動ボタンのX座標
constexpr Point PlayerSettingsSize = { 200,40 };// シーン移動ボタンのサイズ
constexpr int32 PlayerSettingsGap = 50;// シーン移動ボタンの間隔
const RectF BlueButtonRect(PlayerSettingsButton, PlayerSettingsSize);// シーン移動ボタンRect
const RectF RedButtonRect(PlayerSettingsButton.x, PlayerSettingsButton.y + PlayerSettingsGap, PlayerSettingsSize);

//持ち駒移動ボタン
constexpr Point CapturedPieceMoveButton = { 400, 300 };// 持ち駒移動ボタンの
constexpr int32 CapturedPieceMoveButtonYAdjust = 275;// 持ち駒移動ボタンのY座標調整用
constexpr int32 CapturedPieceMoveButtonXAdjust = 375;// 持ち駒移動ボタンのY座標調整用
constexpr Point CapturedPieceMoveButtonButtonSize = { 50, 50 };// 持ち駒移動ボタンのサイズ


// 持ち駒移動ボタンRect
const RectF RedCapturedPieceMoveButtonRight(Arg::center(CapturedPieceMoveButton.x + CapturedPieceMoveButtonXAdjust, CapturedPieceMoveButton.y - CapturedPieceMoveButtonYAdjust), CapturedPieceMoveButtonButtonSize);// 持ち駒右移動ボタンRect
const RectF RedCapturedPieceMoveButtonRect(Arg::center(CapturedPieceMoveButton.x - CapturedPieceMoveButtonXAdjust, CapturedPieceMoveButton.y - CapturedPieceMoveButtonYAdjust), CapturedPieceMoveButtonButtonSize);// 持ち駒左移動ボタンRect
const RectF BlueCapturedPieceMoveButtonRight(Arg::center(CapturedPieceMoveButton.x + CapturedPieceMoveButtonXAdjust, CapturedPieceMoveButton.y + CapturedPieceMoveButtonYAdjust), CapturedPieceMoveButtonButtonSize);// 持ち駒右移動ボタンRect
const RectF BlueCapturedPieceMoveButtonRect(Arg::center(CapturedPieceMoveButton.x - CapturedPieceMoveButtonXAdjust, CapturedPieceMoveButton.y + CapturedPieceMoveButtonYAdjust), CapturedPieceMoveButtonButtonSize);// 持ち駒左移動ボタンRect


