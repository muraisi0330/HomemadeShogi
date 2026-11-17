#pragma once
#include "Board/Bitboard.h"
#include <array>
#include <vector>
#include <cstdint>
#include <iostream>

// 方向ベクトル (dx,dy)
constexpr std::array<std::pair<int, int>, 8> DIRS = { {
	{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}
} };

// mask を作る（rook や bishop 用）
inline Bitboard sliding_mask(int sq, const std::vector<int>& dir_indices) {
	int x = sq % 9, y = sq / 9;
	Bitboard m;
	for (int di : dir_indices) {
		int dx = DIRS[di].first, dy = DIRS[di].second;
		int nx = x + dx, ny = y + dy;
		while (nx >= 0 && nx < 9 && ny >= 0 && ny < 9) {
			// 端の直前までをマスク（端自身は blocker に依存しない場合は除外する実装もある）
			m.set(SQ(nx, ny));
			nx += dx; ny += dy;
		}
	}
	return m;
}

// mask ビット位置を compress して index にする (occupancy & mask の bit を詰めて 0..(1<<bits)-1 に)
inline uint32_t pack_occupancy(const Bitboard& occ, const Bitboard& mask) {
	// iterate mask bits in increasing sq order and produce index
	uint32_t idx = 0;
	uint32_t bit = 1;
	for (int sq = 0; sq < 81; ++sq) {
		if (!mask.test(sq)) continue;
		if (occ.test(sq)) idx |= bit;
		bit <<= 1;
	}
	return idx;
}

// 既存手法: 各 square の mask に対して全 blocker の組合せを列挙し attack table を作る。
// 9x9 なら（最悪 case）mask length <= 16..24 程度で現実的。
struct SlidingTable {
	// per-square attack table
	std::array<std::vector<Bitboard>, 81> rookTable;
	std::array<std::vector<Bitboard>, 81> bishopTable;
	std::array<Bitboard, 81> rookMask;
	std::array<Bitboard, 81> bishopMask;

	void build_all() {
		for (int sq = 0; sq < 81; ++sq) {
			// rook: dirs 0..3, bishop: dirs 4..7
			rookMask[sq] = sliding_mask(sq, { 0,1,2,3 });
			bishopMask[sq] = sliding_mask(sq, { 4,5,6,7 });
			build_for_square(sq, rookMask[sq], rookTable[sq], /*isRook=*/true);
			build_for_square(sq, bishopMask[sq], bishopTable[sq], /*isRook=*/false);
		}
	}

	void build_for_square(int sq, const Bitboard& mask, std::vector<Bitboard>& table, bool isRook) {
		// count mask bits
		int bits = popcount(mask);
		if (bits < 0 || bits > 31) bits = std::max(0, std::min(bits, 31)); // 安全化（現実的には bits は小さい）
		size_t entries = (size_t)1 << bits;
		table.assign(entries, Bitboard{});
		// enumerate occupancies
		for (size_t idx = 0; idx < entries; ++idx) {
			// set occupancy according to idx into occ
			Bitboard occ; int bi = 0;
			for (int s = 0; s < 81; ++s) {
				if (!mask.test(s)) continue;
				if ((idx >> bi) & 1) occ.set(s);
				++bi;
			}
			// compute attacks under this occ
			Bitboard att;
			int x = sq % 9, y = sq / 9;
			if (isRook) {
				for (int di = 0; di < 4; ++di) {
					int dx = DIRS[di].first, dy = DIRS[di].second;
					int nx = x + dx, ny = y + dy;
					while (nx >= 0 && nx < 9 && ny >= 0 && ny < 9) {
						att.set(SQ(nx, ny));
						if (occ.test(SQ(nx, ny))) break;
						nx += dx; ny += dy;
					}
				}
			}
			else {
				for (int di = 4; di < 8; ++di) {
					int dx = DIRS[di].first, dy = DIRS[di].second;
					int nx = x + dx, ny = y + dy;
					while (nx >= 0 && nx < 9 && ny >= 0 && ny < 9) {
						att.set(SQ(nx, ny));
						if (occ.test(SQ(nx, ny))) break;
						nx += dx; ny += dy;
					}
				}
			}
			uint32_t packed = pack_occupancy(occ, mask);
			if (packed < table.size()) {
				table[packed] = att;
			}
			else {
				// ここは本来起きないはず。デバッグ出力しておく。
				std::cerr << "SlidingTable::build_for_square: packed index out of range sq=" << sq << " packed=" << packed << " tableSize=" << table.size() << "\n";
			}
		}
	}

	// query (安全化: sq 範囲と idx チェック)
	Bitboard rook_attacks_from(int sq, const Bitboard& occ) const {
		if (sq < 0 || sq >= 81) {
			std::cerr << "rook_attacks_from: sq out of range: " << sq << "\n";
			return Bitboard{};
		}
		const auto& tbl = rookTable[sq];
		if (tbl.empty()) return Bitboard{};
		uint32_t idx = pack_occupancy(occ, rookMask[sq]);
		if (idx >= tbl.size()) {
			std::cerr << "rook_attacks_from: idx >= tbl.size(): sq=" << sq << " idx=" << idx << " size=" << tbl.size() << "\n";
			return Bitboard{};
		}
		return tbl[idx];
	}
	Bitboard bishop_attacks_from(int sq, const Bitboard& occ) const {
		if (sq < 0 || sq >= 81) {
			std::cerr << "bishop_attacks_from: sq out of range: " << sq << "\n";
			return Bitboard{};
		}
		const auto& tbl = bishopTable[sq];
		if (tbl.empty()) return Bitboard{};
		uint32_t idx = pack_occupancy(occ, bishopMask[sq]);
		if (idx >= tbl.size()) {
			std::cerr << "bishop_attacks_from: idx >= tbl.size(): sq=" << sq << " idx=" << idx << " size=" << tbl.size() << "\n";
			return Bitboard{};
		}
		return tbl[idx];
	}
};
