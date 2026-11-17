#pragma once
#include <Siv3D.hpp>

//将棋台
constexpr int32 ShogiTableCenterX = 400;// 将棋台の中心X座標
constexpr int32 ShogiTableCenterY = 300;// 将棋台の中心Y座標
constexpr int32 ForAdjustingTheWidthOfTheShogiTable = 10;// 将棋盤テクスチャの幅調整用
//将棋台 枠線の太さ
constexpr int32 ShogiTableFrameThickness = 1;// 将棋台の枠線の太さ

//星印
constexpr int32 Star = 75;// 星印の座標
constexpr int32 Starsize = 5;// 星印のサイズ

// 持ち駒表示関連定数
constexpr int32 BlueHandY = 11;               // 青持ち駒Y座標
constexpr int32 RedHandY = 0;                 // 赤持ち駒Y座標

// 勝利表示関連定数
constexpr int32 WinRectX = 100;               // 勝利表示Rect X座標
constexpr int32 WinRectY = 250;               // 勝利表示Rect 幅
constexpr int32 WinRectW = 600;                 // 勝利表示Rect 幅
constexpr int32 WinRectH = 100;               // 勝利表示Rect 高さ
// 勝利表示フォント
constexpr int32 WinFontX = 400;// 勝利表示フォントX座標
constexpr int32 WinFontY = 300;// 勝利表示フォントY座標

// HUD関連定数
constexpr Point HUDSign = { 700, 170 };// 看板の位置
constexpr Vec2 SignScale = { 0.6,0.6 };// 看板のスケーリング倍率
constexpr double VerticalSignRotation = 90_deg;// 看板の回転角度

constexpr Point TextSente = { 700, 100 };// 先手テキスト座標
constexpr Point TextGote = { 700, 150 };// 後手テキスト座標
constexpr Point ElapsedTimePos = { 700, 200 };// 経過時間表示位置
constexpr int32 ElapsedTimeGap = 30;// 経過時間間隔

constexpr Point CurrentTurnPos = { 70, 300 };// 現在手の表示位置
constexpr Vec2 CurrentTurnScale = { 0.4,0.4 };// 現在手表示のスケーリング倍率

//選択中の駒の拡大率
constexpr double SelectedPieceScale = 1.2;// 選択中の駒の拡大率
//駒の向き
constexpr double GotePieceAngle = 180_deg;// 後手の駒の向き


//背景のアスペクト比補正
constexpr Vec2 BackgroundAspectCorrection = { 1.33334 ,1 };// 背景のアスペクト比補正

//掛け軸テクスチャのスケーリング倍率
constexpr Vec2 KakejikuScale = { 1.3,1 };// 掛け軸テクスチャのスケーリング倍率



constexpr Point Title = { 400, 150 };// タイトル座標
constexpr Vec2 TitleScale = { 1,0.8 };// タイトルスケーリング倍率

// クレジットテキスト関連定数
constexpr Point CreditTextPosition = { 400, 270 };// クレジットテキストの位置
constexpr int32 CreditTextGap = 25;// クレジットテキスト間隔

//畳 背景タイルの敷き詰め座標
constexpr std::array<Point, 6> TatamiTilePositions = {{
	{0, 0}, {400, 0}, {100, 400}, {700, 400}, {200, 200}, {200, 400}
} };
constexpr double TatamiTileRotation = 90_deg;// 畳タイルの回転角度

//フェードイン
constexpr double FadeInSpeed = 600;// フェードイン速度
constexpr std::array<Point, 4> FadeInSakuraPositions = {{
	{ 100, 0 }, { -200, 0 }, { 400, 0 }, { 700, 0 }
}};
constexpr int MaxFadeInSpeed = 300;
//フェードアウト
constexpr double FadeOutSpeed = 600;// フェードアウト速度
constexpr std::array<Point, 4> FadeOutSakuraPositions = {{
		{ 800, 0 }, { 1100, 0 }, { -300, 0 }, { -600, 0 }
} };
constexpr int MaxFadeOutSpeed = 400;





