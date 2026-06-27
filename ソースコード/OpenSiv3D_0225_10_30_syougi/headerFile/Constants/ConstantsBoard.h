#pragma once
#include <Siv3D.hpp>

constexpr int32 BoardSize = 9;                // 盤面サイズ
// 将棋の盤面は9x9
constexpr int32 NumTiles = BoardSize * BoardSize; // 盤面のマスの総数

constexpr int32 TileSize = 50;                // 1マスのサイズ

constexpr int32 BoardOffsetXDefault = 175;    // 盤面Xオフセット初期値
constexpr int32 BoardOffsetYDefault = 75;     // 盤面Yオフセット初期値

constexpr int32 kBluePromoteLine = 2;//青チームの成り判定用境界線
constexpr int32 kRedPromoteLine = 3;//赤チームの成り判定用境界線


constexpr int32 RightMarginIgnoreClickX = TileSize * 15;// 右端の余白クリック無視X座標
constexpr int32 LeftMarginIgnoreClickX = TileSize;// 左端の余白クリック無視X座標
//駒のテキスト位置調整用
constexpr int32 PieceTextAdjust = 25;// 駒のテキストX座標調整用
//リセット
constexpr int32 ResetSelectedIndex = -1;// 選択インデックスリセット用
constexpr Point ResetSelectedTarget = { -1,-1 };// 選択ターゲットリセット用
