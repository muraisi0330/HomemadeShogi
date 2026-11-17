#include "stdafx.h"
#include "Game/Drop.h"
#include "Constants/CommonConstants.h"
#include "Game/Promotion.h"

namespace Drop
{
	void ComputeDropMoveRange(int moveRange[BoardSize][BoardSize], const Array<Unit>& unitList, UnitType u, Team team) noexcept
	{
		// 特殊駒打ちルール判定ラムダ
		auto SpecialPiecePlayRuleDetermination = [&](int y,bool b) -> bool
			{
				if(b)return true;
				if (u != UnitType::Pawn &&
					u != UnitType::Knight &&
					u != UnitType::Lance) return false;
				if ((team == Team::Blue && y == 0))	return true;
				if((team == Team::Red && y == (BoardSize - 1))) return true;
				if (u != UnitType::Knight) return false;
				if ((team == Team::Blue && y == 1))	return true;
				if((team == Team::Red && y == (BoardSize - 2)))	return true;
				return false;
		};

		for (int y = 0; y < BoardSize; ++y)
		{
			for (int x = 0; x < BoardSize; ++x)
			{
				// 空きマスなら 1、駒があれば -1
				bool occupied = false;
				// チェック：既に駒があるか
				for (const auto& k : unitList)
				{
					if (!k.isOnBoard) continue;
					if(k.pos.x != x)continue;
					if(k.pos.y != y)continue;
					occupied = true;
					break;
				}
				occupied = SpecialPiecePlayRuleDetermination(y, occupied);
				moveRange[y][x] = occupied ? -1 : 1;
			}
		}

		// Pawn の特殊ルールのみ適用（元の実装に準拠）
		if (u != UnitType::Pawn) return;

		// 同じファイルに自分の歩がある場合、そのファイルには打てない
		auto fileHasUnpromotedPawn = [&](int fileX) -> bool
		{
			for (const auto& k : unitList)
			{
				if (!k.isOnBoard) continue;
				if (k.team != team) continue;
				if (k.type != UnitType::Pawn) continue;
				if (k.pos.x != fileX) continue;
				return true;
			}

			return false;
		};
		//------------------------
		// 同じファイルに自分の歩がある場合、そのファイルには打てない
		//------------------------
		for (int y = 0; y < BoardSize; ++y)
		{
			for (int x = 0; x < BoardSize; ++x)
			{
				if (moveRange[y][x] < 0) continue;
				if (!fileHasUnpromotedPawn(x))continue;
					moveRange[y][x] = ResetSelectedIndex;
					continue;
			}
		}

		

		// ------------------------
		// 打ち歩詰め判定
		// 注意: この関数は board/map 情報を持たないため、移動範囲生成を行わず「利き判定／衝突判定ベース」で逃げ手を判定する。これで打ち歩詰め判定に必要十分な判定を行う（王手の解消は主に王移動・駒取り・塞ぎで決まるため）。
		// ------------------------

		// 相手の王の位置を特定
		Team defender = (team == Team::Blue) ? Team::Red : Team::Blue;
		Point kingPos(-1, -1);
		for (const auto& k : unitList)
		{
			if (k.isOnBoard && k.team == defender && (k.type == UnitType::King || k.type == UnitType::King1))
			{
				kingPos = k.pos;
				break;
			}
		}
		// 王が見つからない場合は判定不能なのでスキップ
		if (kingPos.x < 0) return;

		// どのマスが「王の直前（王手になる位置）」か
		int forward = (team == Team::Blue) ? -1 : 1;
		Point dropCheckPos = { kingPos.x, kingPos.y - forward };


		// ヘルパ関数群（IsSquareAttackedBy, CanUnitAttackSquare など）
		auto IsSquareAttackedBy = [&](const Array<Unit>& units, int tx, int ty, Team teamAttacker) -> bool {
			// ... (既存のコードそのまま)
			// 省略せずに元のロジックをここに配置します
			int fwd = (teamAttacker == Team::Blue) ? -1 : 1;
			for (const auto& u2 : units)
			{
				if (!u2.isOnBoard) continue;
				if (u2.team != teamAttacker) continue;
				int dx = tx - u2.pos.x;
				int dy = ty - u2.pos.y;
				int absX = std::abs(dx);
				int absY = std::abs(dy);
				switch (u2.type)
				{
				case UnitType::Pawn: if (dx == 0 && dy == fwd) return true; break;
				case UnitType::Knight: if (absX == 1 && dy == 2 * fwd) return true; break;
				case UnitType::King:
				case UnitType::King1: if (absX <= 1 && absY <= 1) return true; break;
				case UnitType::Gold:
				case UnitType::Pawn1:
				case UnitType::Lance1:
				case UnitType::Knight1:
				case UnitType::Silver1: if (absX <= 1 && absY <= 1 && !(absX == 1 && dy == -fwd)) return true; break;
				case UnitType::Silver: if ((absX <= 1 && absY <= 1) && !(absX == 0 && dy == -fwd) && !(absY == 0)) return true; break;
				default: if (absX <= 1 && absY <= 1 && !(absX == 0 && absY == 0)) return true; break;
				}
			}
			return false;
			};

		auto CanUnitAttackSquare = [&](const Array<Unit>& units, int unitIndex, int tx, int ty) -> bool {
			// ... (既存のコードそのまま)
			if (unitIndex < 0 || unitIndex >= static_cast<int>(units.size())) return false;
			const Unit& u2 = units[unitIndex];
			if (!u2.isOnBoard) return false;
			int fwd = (u2.team == Team::Blue) ? -1 : 1;
			int dx = tx - u2.pos.x;
			int dy = ty - u2.pos.y;
			int absX = std::abs(dx);
			int absY = std::abs(dy);
			switch (u2.type)
			{
			case UnitType::Pawn: return (dx == 0 && dy == fwd);
			case UnitType::Knight: return (absX == 1 && dy == 2 * fwd);
			case UnitType::King:
			case UnitType::King1: return (absX <= 1 && absY <= 1 && !(absX == 0 && absY == 0));
			default: return (absX <= 1 && absY <= 1 && !(absX == 0 && absY == 0));
			}
			};

		for (int ty = 0; ty < BoardSize; ++ty)
		{
			for (int tx = 0; tx < BoardSize; ++tx)
			{
				if (moveRange[ty][tx] < 0) continue;

				// ★【最重要の高速化】打つ場所が「王の直前（王手になる位置）」以外なら、
				// 打ち歩詰めのわけがないので、このマスの重い判定を完全スキップ！
				if (tx != dropCheckPos.x || ty != dropCheckPos.y) continue;

				// --- 以下、本当に王の直前に歩を打つ場合のみ実行される重い処理 ---
				Array<Unit> unitsAfter = unitList;
				bool placed = false;
				for (auto& cu : unitsAfter)
				{
					if (cu.isOnBoard) continue;
					if (cu.team != team) continue;
					if (ToDemoted(cu.type) != UnitType::Pawn && cu.type != UnitType::Pawn) continue;
					cu.isOnBoard = true;
					cu.pos.x = tx;
					cu.pos.y = ty;
					cu.promoteFlag = false;
					cu.hasMoved = true;
					cu.type = UnitType::Pawn;
					placed = true;
					break;
				}
				if (!placed) continue;

				// 1) この打ちで本当に王手になっているか
				if (!IsSquareAttackedBy(unitsAfter, kingPos.x, kingPos.y, team))
				{
					continue;
				}

				// 攻撃元（王手をかけている歩）を列挙
				Array<int> attackers;
				for (int i = 0; i < static_cast<int>(unitsAfter.size()); ++i)
				{
					const auto& cu = unitsAfter[i];
					if (!cu.isOnBoard) continue;
					if (cu.team != team) continue;
					if (CanUnitAttackSquare(unitsAfter, i, kingPos.x, kingPos.y)) attackers.push_back(i);
				}

				bool escapeExists = false;

				// (A) 王の移動で逃げられるか（周囲8マス）
				for (int dy = -1; dy <= 1 && !escapeExists; ++dy)
				{
					for (int dx = -1; dx <= 1; ++dx)
					{
						if (dx == 0 && dy == 0) continue;
						int nx = kingPos.x + dx;
						int ny = kingPos.y + dy;
						if (nx < 0 || nx >= BoardSize || ny < 0 || ny >= BoardSize) continue;

						bool occByDefender = false;
						for (const auto& m : unitsAfter) if (m.isOnBoard && m.team == defender && m.pos.x == nx && m.pos.y == ny) { occByDefender = true; break; }
						if (occByDefender) continue;

						Array<Unit> tmp = unitsAfter;
						for (auto& km : tmp) {
							if (!km.isOnBoard) continue;
							if (km.team != defender) continue;
							if (km.type != UnitType::King && km.type != UnitType::King1) continue;
							if (km.pos == kingPos) { km.pos.x = nx; km.pos.y = ny; break; }
						}
						for (auto& t : tmp) {
							if (!t.isOnBoard) continue;
							if (t.team != team) continue;
							if (t.pos.x == nx && t.pos.y == ny) { t.isOnBoard = false; break; }
						}
						if (!IsSquareAttackedBy(tmp, nx, ny, team))
						{
							escapeExists = true;
							break;
						}
					}
				}

				if (escapeExists) continue;

				// (B) 攻撃駒（打った歩）を取れるか
				for (int ai : attackers)
				{
					int ax = unitsAfter[ai].pos.x;
					int ay = unitsAfter[ai].pos.y;
					for (int di = 0; di < static_cast<int>(unitsAfter.size()) && !escapeExists; ++di)
					{
						const auto& defenderUnit = unitsAfter[di];
						if (!defenderUnit.isOnBoard) continue;
						if (defenderUnit.team != defender) continue;
						if ((defenderUnit.type == UnitType::King || defenderUnit.type == UnitType::King1) && defenderUnit.pos == kingPos) continue;

						if (CanUnitAttackSquare(unitsAfter, di, ax, ay))
						{
							Array<Unit> tmp = unitsAfter;
							for (auto& t : tmp) if (t.isOnBoard && t.team == team && t.pos.x == ax && t.pos.y == ay) { t.isOnBoard = false; break; }
							for (auto& t : tmp) if (t.isOnBoard && t.team == defender && t.pos.x == defenderUnit.pos.x && t.pos.y == defenderUnit.pos.y) { t.pos.x = ax; t.pos.y = ay; break; }

							Point kpos2 = kingPos;
							if (!IsSquareAttackedBy(tmp, kpos2.x, kpos2.y, team))
							{
								escapeExists = true;
								break;
							}
						}
					}
					if (escapeExists) break;
				}

				// 逃げ場が一切なければ「打ち歩詰め」なので配置不可（-1）にする
				if (!escapeExists)
				{
					moveRange[ty][tx] = ResetSelectedIndex;
				}
			}
		}
	}
}
