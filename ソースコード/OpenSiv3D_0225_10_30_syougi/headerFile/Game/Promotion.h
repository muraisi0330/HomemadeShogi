#pragma once
#include "Types/types.h"
#include "Constants/CommonConstants.h"

// Promotion モジュール：成り判定と変換を純関数的に提供する。
// 依存は types.h のみ。副作用なし。
// - toPromoted: 与えられた駒種を成駒に変換して返す（成りが存在しない場合は元を返す）
// - toDemoted: 表示用に成駒を元に戻す
// - isAutoPromotionRequired: 盤上位置 y によって強制成り判定を行う（駒種別ごとに閾値を受け取る）
UnitType ToPromoted(UnitType t) noexcept;
UnitType ToDemoted(UnitType t) noexcept;
bool IsAutoPromotionRequired(UnitType t, Team team, int y) noexcept;
