#pragma once
#include "Constants/CommonConstants.h"
#include <cstdint>
#include <array>
#include <vector>
#include <cassert>
#include <bit> // C++20 std::popcount and std::countr_zero



// Bitboard for 9x9 board: use 81 bits stored in two uint64_t (low: bits 0..63, high: bits 64..80)
struct Bitboard {
    uint64_t lo = 0; // bits 0..63
    uint64_t hi = 0; // bits 64..127 (we use only 64..80)
	//盤面のマス数
	int32 static constexpr BOARD_SIZE = BoardSize * BoardSize;
	static constexpr uint64_t HI_MASK = 0x1FFFFULL; // 17ビット分 (81-64)

    constexpr Bitboard() noexcept = default;
    constexpr explicit Bitboard(uint64_t l, uint64_t h = 0) noexcept : lo(l), hi(h) {}

    constexpr bool test(int sq) const noexcept {
        if (sq < 0 || sq >= BOARD_SIZE) return false;
        if (sq < 64) return (lo >> sq) & 1ULL;
        return (hi >> (sq - 64)) & 1ULL;
    }
    constexpr void set(int sq) noexcept {
        if (sq < 64) lo |= (1ULL << sq);
        else hi |= (1ULL << (sq - 64));
    }
    constexpr void reset(int sq) noexcept {
        if (sq < 64) lo &= ~(1ULL << sq);
        else hi &= ~(1ULL << (sq - 64));
    }
    constexpr void clear() noexcept { lo = 0; hi = 0; }
    constexpr bool any() const noexcept { return lo || hi; }

    constexpr Bitboard operator|(const Bitboard& o) const noexcept { return Bitboard(lo | o.lo, hi | o.hi); }
    constexpr Bitboard operator&(const Bitboard& o) const noexcept { return Bitboard(lo & o.lo, hi & o.hi); }
    constexpr Bitboard operator^(const Bitboard& o) const noexcept { return Bitboard(lo ^ o.lo, hi ^ o.hi); }
	constexpr Bitboard operator~() const noexcept {return Bitboard(~lo, (~hi) & HI_MASK);}

    bool operator==(const Bitboard& o) const noexcept { return lo == o.lo && hi == o.hi; }
};

// helpers
inline int popcount64(uint64_t x) { return static_cast<int>(std::popcount(x)); }
inline int popcount(const Bitboard& b) { return popcount64(b.lo) + popcount64(b.hi); }

// LSB helper: returns index of least-significant set bit (0..80) or -1 if empty
inline int lsb_index(const Bitboard& b) noexcept {
    if (b.lo) {
        return static_cast<int>(std::countr_zero(b.lo)); // 0..63
    }
    if (b.hi) {
        return 64 + static_cast<int>(std::countr_zero(b.hi)); // 64..
    }
    return -1;
}
// pop lowest bit and return its index, or -1 if empty
inline int pop_lsb(Bitboard& b) noexcept {
    if (b.lo) {
        unsigned long tz = static_cast<unsigned long>(std::countr_zero(b.lo));
        b.lo &= (b.lo - 1);
        return static_cast<int>(tz);
    }
    if (b.hi) {
        unsigned long tz = static_cast<unsigned long>(std::countr_zero(b.hi));
        b.hi &= (b.hi - 1);
        return 64 + static_cast<int>(tz);
    }
    return -1;
}

// map (x,y) -> square index 0..80 (row-major)
constexpr inline int SQ(int x, int y) noexcept { return y * 9 + x; }

// build bitboard from list of coordinates
inline Bitboard make_bb_from_coords(const std::vector<std::pair<int,int>>& coords) {
    Bitboard b;
    for (auto [x,y]: coords) {
        if (x < 0 || x >= 9 || y < 0 || y >= 9) continue;
        b.set(SQ(x,y));
    }
    return b;
}
