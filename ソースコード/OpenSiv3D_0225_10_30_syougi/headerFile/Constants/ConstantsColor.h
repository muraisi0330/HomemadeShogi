#pragma once
#include <Siv3D.hpp>

//移動範囲オーバーレイの色
constexpr ColorF DrawMoveRangeColor = ColorF{ 1,0,0,0.4 };

constexpr ColorF SelectedPieceColor = Palette::Red;

// ボタンは色にする
constexpr ColorF ButtonColor = Palette::Black;

constexpr ColorF ButtonHoverColor = Palette::Yellow;

// 駒の	デフォルトの文字色
constexpr ColorF PieceTextColor = Palette::Black;
// 成駒は赤
constexpr ColorF PromotedPieceTextColor = Palette::Red;
