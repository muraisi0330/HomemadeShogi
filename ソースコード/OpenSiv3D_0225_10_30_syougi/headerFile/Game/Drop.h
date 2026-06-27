#pragma once
#include "Types/types.h"
#include "Constants/CommonConstants.h"

// Drop モジュール：持ち駒（打ち）に関するルールを提供する
namespace Drop
{
	// moveRange を初期化して、持ち駒 u を team が打てるマスを 1 に、不可を -1 にする
	void ComputeDropMoveRange(int moveRange[BoardSize][BoardSize], const Array<Unit>& unitList, UnitType u, Team team) noexcept;
}
