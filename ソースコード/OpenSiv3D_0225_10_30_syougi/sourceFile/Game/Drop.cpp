#include "stdafx.h"
#include "Game/Drop.h"
#include "Constants/CommonConstants.h"
#include "Game/Movement.h" // 盤上の利きや移動判定に整合させるため（一部は独自実装）
#include "Game/Promotion.h"
#include <algorithm>

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

		// ヘルパ: 指定マスが攻撃されているか（攻撃者 teamAttacker の利き）を判定する
		auto IsSquareAttackedBy = [&](const Array<Unit>& units, int tx, int ty, Team teamAttacker) -> bool
		{
			// forward の定義は ShidenAI 側実装と合わせる
			int forward = (teamAttacker == Team::Blue) ? -1 : 1;

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
				case UnitType::Pawn:
					if (dx == 0 && dy == forward) return true;
					break;
				case UnitType::Knight:
					if (absX == 1 && dy == 2 * forward) return true;
					break;
				case UnitType::King:
				case UnitType::King1:
					if (absX <= 1 && absY <= 1) return true;
					break;
				case UnitType::Gold:
				case UnitType::Pawn1:
				case UnitType::Lance1:
				case UnitType::Knight1:
				case UnitType::Silver1:
					if (absX <= 1 && absY <= 1 && !(absX == 1 && dy == -forward)) return true;
					break;
				case UnitType::Silver:
					// 銀の判定（ShidenAI 実装に合わせる）
					if ( (absX <= 1 && absY <= 1) && !(absX == 0 && dy == -forward) && !(absY == 0) ) return true;
					break;
				case UnitType::Lance:
					if (dx == 0 && ((dy > 0 && forward > 0) || (dy < 0 && forward < 0)))
					{
						// 途中に障害物がないか確認
						bool blocked = false;
						int step = (dy > 0) ? 1 : -1;
						for (int s = u2.pos.y + step; s != ty; s += step)
						{
							for (const auto& b : units) if (b.isOnBoard && b.pos.x == u2.pos.x && b.pos.y == s) { blocked = true; break; }
							if (blocked) break;
						}
						if (!blocked) return true;
					}
					break;
				case UnitType::Bishop:
				case UnitType::Bishop1:
					if (absX == absY && absX > 0)
					{
						bool blocked = false;
						int stepX = dx / absX;
						int stepY = dy / absY;
						for (int i = 1; i < absX; ++i)
						{
							int mx = u2.pos.x + i * stepX;
							int my = u2.pos.y + i * stepY;
							for (const auto& b : units) if (b.isOnBoard && b.pos.x == mx && b.pos.y == my) { blocked = true; break; }
							if (blocked) break;
						}
						if (!blocked) return true;
					}
					// 成角は周辺1マスも利く
					if (u2.type == UnitType::Bishop1 && (absX + absY == 1)) return true;
					break;
				case UnitType::Rook:
				case UnitType::Rook1:
					if ((dx == 0 || dy == 0) && (absX + absY > 0))
					{
						bool blocked = false;
						int stepX = (dx == 0) ? 0 : (dx / absX);
						int stepY = (dy == 0) ? 0 : (dy / absY);
						int dist = std::max(absX, absY);
						for (int i = 1; i < dist; ++i)
						{
							int mx = u2.pos.x + i * stepX;
							int my = u2.pos.y + i * stepY;
							for (const auto& b : units) if (b.isOnBoard && b.pos.x == mx && b.pos.y == my) { blocked = true; break; }
							if (blocked) break;
						}
						if (!blocked) return true;
					}
					if (u2.type == UnitType::Rook1 && absX == 1 && absY == 1) return true;
					break;
				default:
					// その他は周囲1マスに限定（安全策）
					if (absX <= 1 && absY <= 1 && !(absX == 0 && absY == 0)) return true;
					break;
				}
			}
			return false;
		};

		// ヘルパ: 指定ユニットが指定マスを利いているか（単一ユニット判定）
		auto CanUnitAttackSquare = [&](const Array<Unit>& units, int unitIndex, int tx, int ty) -> bool
		{
			if (unitIndex < 0 || unitIndex >= static_cast<int>(units.size())) return false;
			const Unit& u2 = units[unitIndex];
			if (!u2.isOnBoard) return false;

			int forward = (u2.team == Team::Blue) ? -1 : 1;
			int dx = tx - u2.pos.x;
			int dy = ty - u2.pos.y;
			int absX = std::abs(dx);
			int absY = std::abs(dy);

			switch (u2.type)
			{
			case UnitType::Pawn:
				return (dx == 0 && dy == forward);
			case UnitType::Knight:
				return (absX == 1 && dy == 2 * forward);
			case UnitType::King:
			case UnitType::King1:
				return (absX <= 1 && absY <= 1 && !(absX == 0 && absY == 0));
			case UnitType::Gold:
			case UnitType::Pawn1:
			case UnitType::Lance1:
			case UnitType::Knight1:
			case UnitType::Silver1:
				return (absX <= 1 && absY <= 1 && !(absX == 1 && dy == -forward));
			case UnitType::Silver:
				return (absX <= 1 && absY <= 1 && !(absX == 0 && dy == -forward) && !(absY == 0));
			case UnitType::Lance:
				if (dx != 0) return false;
				if ((dy > 0 && forward <= 0) || (dy < 0 && forward >= 0)) return false;
				{
					int step = (dy > 0) ? 1 : -1;
					for (int s = u2.pos.y + step; s != ty; s += step)
						for (const auto& b : units) if (b.isOnBoard && b.pos.x == u2.pos.x && b.pos.y == s) return false;
					return true;
				}
			case UnitType::Bishop:
			case UnitType::Bishop1:
				if (absX == absY && absX > 0)
				{
					int stepX = dx / absX;
					int stepY = dy / absY;
					for (int i = 1; i < absX; ++i)
					{
						int mx = u2.pos.x + i * stepX;
						int my = u2.pos.y + i * stepY;
						for (const auto& b : units) if (b.isOnBoard && b.pos.x == mx && b.pos.y == my) return false;
					}
					return true;
				}
				if (u2.type == UnitType::Bishop1 && (absX + absY == 1)) return true;
				return false;
			case UnitType::Rook:
			case UnitType::Rook1:
				if ((dx == 0 || dy == 0) && (absX + absY > 0))
				{
					int stepX = (dx == 0) ? 0 : (dx / absX);
					int stepY = (dy == 0) ? 0 : (dy / absY);
					int dist = std::max(absX, absY);
					for (int i = 1; i < dist; ++i)
					{
						int mx = u2.pos.x + i * stepX;
						int my = u2.pos.y + i * stepY;
						for (const auto& b : units) if (b.isOnBoard && b.pos.x == mx && b.pos.y == my) return false;
					}
					return true;
				}
				if (u2.type == UnitType::Rook1 && absX == 1 && absY == 1) return true;
				return false;
			default:
				return (absX <= 1 && absY <= 1 && !(absX == 0 && absY == 0));
			}
		};

		// 打ち歩詰め判定本体: 打ち位置 (dx,dy) が既に moveRange で 1 の場合、その打ちが詰みかを判定する
		for (int ty = 0; ty < BoardSize; ++ty)
		{
			for (int tx = 0; tx < BoardSize; ++tx)
			{
				if (moveRange[ty][tx] < 0) continue; // 既に不可
				// 仮想局面を作る（unitList のコピー）
				Array<Unit> unitsAfter = unitList;

				// 手駒から打つ対象のユニットを探し、最初の該当手駒を盤上に置く
				bool placed = false;
				for (auto& cu : unitsAfter)
				{
					if (cu.isOnBoard) continue;
					if (cu.team != team) continue;
					// 持ち駒は非成に正規化されているとは限らないが、引数 u は駒種なのでそれに合わせる
					if (ToDemoted(cu.type) != UnitType::Pawn && cu.type != UnitType::Pawn) continue;
					// 同一手駒を示唆するため handIndex 等は無視して最初に見つかった同種の持ち駒を使う
					cu.isOnBoard = true;
					cu.pos.x = tx;
					cu.pos.y = ty;
					cu.promoteFlag = false;
					cu.hasMoved = true;
					cu.type = UnitType::Pawn; // 打つのは生の歩
					placed = true;
					break;
				}
				if (!placed)
				{
					// 持ち駒が見つからない（理論上はありえない呼び出し）なら安全側で許可
					continue;
				}

				// 相手の王を見つける（守備側: 相手チーム）
				Team defender = (team == Team::Blue) ? Team::Red : Team::Blue;
				Point kingPos = ResetSelectedTarget;
				for (const auto& k : unitsAfter)
				{
					if (!k.isOnBoard) continue;
					if (k.team != defender) continue;
					if (k.type == UnitType::King || k.type == UnitType::King1) { kingPos = k.pos; break; }
				}
				if (kingPos.x < 0) {
					// 王が盤上に存在しない（異常）なら禁止しない
					continue;
				}

				// 1) この打ちで王手になっているか
				if (!IsSquareAttackedBy(unitsAfter, kingPos.x, kingPos.y, team))
				{
					// 王手になっていない -> 合法
					continue;
				}

				// 攻撃元（王手をかけている駒）を列挙
				Array<int> attackers;
				for (int i = 0; i < static_cast<int>(unitsAfter.size()); ++i)
				{
					const auto& cu = unitsAfter[i];
					if (!cu.isOnBoard) continue;
					if (cu.team != team) continue;
					if (CanUnitAttackSquare(unitsAfter, i, kingPos.x, kingPos.y)) attackers.push_back(i);
				}

				// 王手を掛けている駒が複数ある場合（ダブルチェック相当）は王の逃げ以外は無理
				bool escapeExists = false;

				// (A) 王の移動で逃げられるか（周囲8マス）
				for (int dy = -1; dy <= 1 && !escapeExists; ++dy)
				{
					for (int dx = -1; dx <= 1; ++dx)
					{
						if (dx == 0 && dy == 0) continue;
						int nx = kingPos.x + dx;
						int ny = kingPos.y + dy;
						// 範囲チェック
						if (nx < 0 || nx >= BoardSize || ny < 0 || ny >= BoardSize) continue;
						// 移動先が味方駒で占有されているか
						bool occByDefender = false;
						for (const auto& m : unitsAfter) if (m.isOnBoard && m.team == defender && m.pos.x == nx && m.pos.y == ny) { occByDefender = true; break; }
						if (occByDefender) continue;
						// 移動先に敵駒がいるならその駒を取ることになる -> 取り得るかのチェック（取り先が別の敵の利きで安全かを後で判定）
						// 仮想的に王を移動させて、その移動後にその王が相手に利かれていないかを判定
						Array<Unit> tmp = unitsAfter;
						// king を移動させる（対象王を tmp 内で見つけて移動）
						for (auto& km : tmp) {
							if (!km.isOnBoard) continue;
							if (km.team != defender) continue;
							if (km.type != UnitType::King && km.type != UnitType::King1) continue;
							if (km.pos == kingPos) { km.pos.x = nx; km.pos.y = ny; break; }
						}
						// 移動先が相手駒であればその駒は盤上から消える（捕獲）
						for (auto& t : tmp) {
							if (!t.isOnBoard) continue;
							if (t.team != team) continue;
							if (t.pos.x == nx && t.pos.y == ny) { t.isOnBoard = false; break; }
						}
						// 移動後に相手の利きに捕られていないか
						if (!IsSquareAttackedBy(tmp, nx, ny, team))
						{
							escapeExists = true;
							break;
						}
					}
				}

				if (escapeExists) continue;

				// (B) 攻撃駒を取れるか（盤上駒による捕獲）
				// 攻撃駒リストを基に、いずれかを取れるかを調べる
				for (int ai : attackers)
				{
					int ax = unitsAfter[ai].pos.x;
					int ay = unitsAfter[ai].pos.y;
					// 盤上の守備駒で捕獲できるか
					for (int di = 0; di < static_cast<int>(unitsAfter.size()) && !escapeExists; ++di)
					{
						const auto& defenderUnit = unitsAfter[di];
						if (!defenderUnit.isOnBoard) continue;
						if (defenderUnit.team != defender) continue;
						// 王自身は既に試しているため除外（王以外で捕獲できれば良い）
						if ((defenderUnit.type == UnitType::King || defenderUnit.type == UnitType::King1) && defenderUnit.pos == kingPos) continue;
						if (CanUnitAttackSquare(unitsAfter, di, ax, ay))
						{
							// 捕獲した後に王が安全かを確認する（捕獲で詰みが解消されるか）
							Array<Unit> tmp = unitsAfter;
							// 捕獲: defenderUnit moves to (ax,ay), attacker at (ax,ay) removed
							// find and remove attacker
							for (auto& t : tmp) if (t.isOnBoard && t.team == team && t.pos.x == ax && t.pos.y == ay) { t.isOnBoard = false; break; }
							// move the capturing unit
							for (auto& t : tmp) if (t.isOnBoard && t.team == defender && t.pos.x == defenderUnit.pos.x && t.pos.y == defenderUnit.pos.y) { t.pos.x = ax; t.pos.y = ay; break; }
							// kingPos unchanged; check if king still in check
							// find king pos in tmp
							Point kpos2 = ResetSelectedTarget;
							for (const auto& t : tmp) if (t.isOnBoard && t.team == defender && (t.type == UnitType::King || t.type == UnitType::King1)) { kpos2 = t.pos; break; }
							if (kpos2.x >= 0 && !IsSquareAttackedBy(tmp, kpos2.x, kpos2.y, team))
							{
								escapeExists = true;
								break;
							}
						}
					}
					if (escapeExists) break;

					// 持ち駒で捕獲できるか（打ちで取れるか）: 持ち駒が攻撃駒の位置に打てるかを調べる
					for (int h = 0; h < static_cast<int>(unitsAfter.size()) && !escapeExists; ++h)
					{
						const auto& hand = unitsAfter[h];
						if (hand.isOnBoard) continue;
						if (hand.team != defender) continue;
						// その持ち駒を (ax,ay) に打てるかを判定（ComputeDropMoveRange を呼ぶ）
						int dr[BoardSize][BoardSize];
						// 注意: ComputeDropMoveRange は本関数。再帰的呼び出しによる副作用に配慮してローカル配列を使う。
						ComputeDropMoveRange(dr, unitsAfter, hand.type, defender);
						if (dr[ay][ax] >= 0)
						{
							// 打って取ることで回避できるなら escape
							// simulate drop capture: the placed piece occupies ax,ay and attacker is removed
							Array<Unit> tmp = unitsAfter;
							// find first matching hand in tmp and place it
							for (auto& t : tmp) {
								if (t.isOnBoard) continue;
								if (t.team != defender) continue;
								if (ToDemoted(t.type) != ToDemoted(hand.type)) continue;
								t.isOnBoard = true; t.pos.x = ax; t.pos.y = ay; t.type = ToDemoted(hand.type); break;
							}
							// remove attacker
							for (auto& t : tmp) if (t.isOnBoard && t.team == team && t.pos.x == ax && t.pos.y == ay) { t.isOnBoard = false; break; }
							// find king and test safety
							Point kpos2 = ResetSelectedTarget;
							for (const auto& t : tmp) if (t.isOnBoard && t.team == defender && (t.type == UnitType::King || t.type == UnitType::King1)) { kpos2 = t.pos; break; }
							if (kpos2.x >= 0 && !IsSquareAttackedBy(tmp, kpos2.x, kpos2.y, team))
							{
								escapeExists = true;
								break;
							}
						}
					}
					if (escapeExists) break;

					// 間に入れる（スライダー攻撃の場合のみ意味がある）
					const auto& atk = unitsAfter[ai];
					if (!(atk.type == UnitType::Bishop || atk.type == UnitType::Rook || atk.type == UnitType::Lance || atk.type == UnitType::Bishop1 || atk.type == UnitType::Rook1))
					{
						// 非スライダーなら間に入る手は無意味
						continue;
					}
					// スライダーの場合、攻撃元と王の間のマスを列挙してそこに守備駒が入れるかを調べる
					int dx = kingPos.x - atk.pos.x;
					int dy = kingPos.y - atk.pos.y;
					int stepX = (dx == 0) ? 0 : (dx / std::abs(dx));
					int stepY = (dy == 0) ? 0 : (dy / std::abs(dy));
					int dist = std::max(std::abs(dx), std::abs(dy));
					for (int s = 1; s < dist && !escapeExists; ++s)
					{
						int bx = atk.pos.x + s * stepX;
						int by = atk.pos.y + s * stepY;
						// 盤上駒でそのマスに移動できるか
						for (int di = 0; di < static_cast<int>(unitsAfter.size()) && !escapeExists; ++di)
						{
							const auto& defenderUnit = unitsAfter[di];
							if (!defenderUnit.isOnBoard) continue;
							if (defenderUnit.team != defender) continue;
							if (CanUnitAttackSquare(unitsAfter, di, bx, by))
							{
								// 移動後に王が安全か確認（簡易）
								Array<Unit> tmp = unitsAfter;
								// move defenderUnit to (bx,by) and if some attacker there remove it
								for (auto& t : tmp) if (t.isOnBoard && t.team == defender && t.pos == defenderUnit.pos) { t.pos.x = bx; t.pos.y = by; break; }
								for (auto& t : tmp) if (t.isOnBoard && t.team == team && t.pos.x == bx && t.pos.y == by) { t.isOnBoard = false; break; }
								Point kpos2 = ResetSelectedTarget;
								for (const auto& t : tmp) if (t.isOnBoard && t.team == defender && (t.type == UnitType::King || t.type == UnitType::King1)) { kpos2 = t.pos; break; }
								if (kpos2.x >= 0 && !IsSquareAttackedBy(tmp, kpos2.x, kpos2.y, team))
								{
									escapeExists = true;
									break;
								}
							}
						}
						if (escapeExists) break;
						// 持ち駒でそのマスに打てるか
						for (int h = 0; h < static_cast<int>(unitsAfter.size()) && !escapeExists; ++h)
						{
							const auto& hand = unitsAfter[h];
							if (hand.isOnBoard) continue;
							if (hand.team != defender) continue;
							int dr[BoardSize][BoardSize];
							ComputeDropMoveRange(dr, unitsAfter, hand.type, defender);
							if (dr[by][bx] >= 0)
							{
								// 打って塞げるなら
								Array<Unit> tmp = unitsAfter;
								for (auto& t : tmp) {
									if (t.isOnBoard) continue;
									if (t.team != defender) continue;
									if (ToDemoted(t.type) != ToDemoted(hand.type)) continue;
									t.isOnBoard = true; t.pos.x = bx; t.pos.y = by; t.type = ToDemoted(hand.type); break;
								}
								Point kpos2 = ResetSelectedTarget;
								for (const auto& t : tmp) if (t.isOnBoard && t.team == defender && (t.type == UnitType::King || t.type == UnitType::King1)) { kpos2 = t.pos; break; }
								if (kpos2.x >= 0 && !IsSquareAttackedBy(tmp, kpos2.x, kpos2.y, team))
								{
									escapeExists = true;
									break;
								}
							}
						}
					} // end for s
					if (escapeExists) break;
				} // end for attackers

				// いかなる逃げ手も見つからなければ詰み → 打ち歩詰めとして禁止
				if (!escapeExists)
				{
					moveRange[ty][tx] = ResetSelectedIndex;
				}
			}
		}
	}
}

