#include "stdafx.h"
#include "Game/Promotion.h"

// 成り駒変換関数
UnitType ToPromoted(UnitType t) noexcept
{
    switch (t)
    {
    case UnitType::Pawn:   return UnitType::Pawn1;
    case UnitType::Lance:  return UnitType::Lance1;
    case UnitType::Knight: return UnitType::Knight1;
    case UnitType::Silver: return UnitType::Silver1;
    case UnitType::Bishop: return UnitType::Bishop1;
    case UnitType::Rook:   return UnitType::Rook1;
    default:               return t;
    }
}

// 成り駒を元に戻す関数（表示用）
UnitType ToDemoted(UnitType t) noexcept
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

// 自動成り（強制成り）の閾値チェック
bool IsAutoPromotionRequired(UnitType t, Team team, int y) noexcept
{
    if (t == UnitType::Pawn || t == UnitType::Lance)
    {
        // Pawn/Lance: 最終段で強制成り
        if (team == Team::Blue) return (y == 0);
        return (y == BoardSize - 1);
    }
    if (t == UnitType::Knight)
    {
        // Knight: 最後から2行目〜強制成り（移動できなくなる行）
        if (team == Team::Blue) return (y <= 1);
        return (y >= BoardSize - 2);
    }
    return false;
}
