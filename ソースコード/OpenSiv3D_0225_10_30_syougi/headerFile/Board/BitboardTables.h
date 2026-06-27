#pragma once
#include "Board/Bitboard.h"
#include <array>

// 盤面サイズ
constexpr int BOARD_SQ = 81;

// 近接駒（歩・王・金・銀・桂など）の攻撃マスクを constexpr で作る。
// 注意: 歩は進行方向で異なるので team 毎に別配列を用意。

constexpr inline bool in_board(int x, int y) { return x >= 0 && x < 9 && y >= 0 && y < 9; }

// 青(先手) は上へ (-1), 赤(後手) は下へ (+1)
constexpr std::array<Bitboard, BOARD_SQ> MakeKingAttacks() {
    std::array<Bitboard, BOARD_SQ> out{};
    for (int y=0;y<9;++y) for (int x=0;x<9;++x) {
        Bitboard b;
        int sq = SQ(x,y);
        for (int dy=-1;dy<=1;++dy) for (int dx=-1;dx<=1;++dx) {
            if (dx==0 && dy==0) continue;
            int nx = x + dx, ny = y + dy;
            if (in_board(nx,ny)) b.set(SQ(nx,ny));
        }
        out[sq] = b;
    }
    return out;
}

constexpr std::array<Bitboard, BOARD_SQ> MakePawnAttacksForward(int forward) {
    std::array<Bitboard, BOARD_SQ> out{};
    for (int y=0;y<9;++y) for (int x=0;x<9;++x) {
        Bitboard b;
        int ny = y + forward;
        if (in_board(x,ny)) b.set(SQ(x,ny));
        out[SQ(x,y)] = b;
    }
    return out;
}

// 桂馬攻撃（先手基準 forward=-1）
constexpr std::array<Bitboard, BOARD_SQ> MakeKnightAttacks(int forward) {
    std::array<Bitboard, BOARD_SQ> out{};
    for (int y=0;y<9;++y) for (int x=0;x<9;++x) {
        Bitboard b;
        int ny = y + 2 * forward;
		if (in_board(x - 1, ny)) b.set(SQ(x - 1, ny));
        if (in_board(x+1,ny)) b.set(SQ(x+1, ny));
        out[SQ(x,y)] = b;
    }
    return out;
}

// 銀/金 等は複雑だが同様に作れる。ここでは王/歩/桂 の例を提供。

constexpr auto KING_ATTACKS = MakeKingAttacks();
constexpr auto PAWN_ATTACKS_BLUE = MakePawnAttacksForward(-1);
constexpr auto PAWN_ATTACKS_RED  = MakePawnAttacksForward(+1);
constexpr auto KNIGHT_ATTACKS_BLUE = MakeKnightAttacks(-1);
constexpr auto KNIGHT_ATTACKS_RED  = MakeKnightAttacks(+1);
