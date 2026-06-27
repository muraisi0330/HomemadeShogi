#pragma once
#include "Board/Bitboard.h"
#include "Board/BitboardTables.h"
#include "Board/SlidingAttacks.h"
#include <vector>

// Candidate / Move 型は既存プロジェクトと整合させてください。
// ここでは概念コード：Candidate { int unitIndex; Point target; bool isDrop; bool promoteDecision; int score; }

struct Candidate { int unitIndex; int targetSq; bool isDrop; bool promoteDecision; int score; };

// 例: evasion/capture 生成のスケルトン（ビットボード利用）
inline void GenerateEvasionsFromBitboards(
    int kingSq,
    const Bitboard& ownOcc,
    const Bitboard& enemyOcc,
    const Bitboard& enemyAttackMask, // 盤上全体の敵の利き（ビットボード）
    const SlidingTable& sliding,
    std::vector<Candidate>& out)
{
    // 1) king moves: king attack mask minus squares covered by enemy
    Bitboard possible = KING_ATTACKS[kingSq] & ~enemyAttackMask & ~ownOcc;
    // push each set bit as candidate (unitIndex must be assigned by caller)
    for (int sq=0; sq<81; ++sq) if (possible.test(sq)) {
        out.push_back({ /*unitIndex*/ -1, sq, false, false, 0 });
    }
    // 2) captures/blockers: compute squares that block checks (this requires information about check rays)
    // Implementation detail: caller must supply checker bit(s) and ray(s). This is a skeleton for how to use bitboards.
}

// 捕獲手のみ生成（例）
inline void GenerateCapturesBitboard(
    int fromSq, int unitType, const Bitboard& ownOcc, const Bitboard& enemyOcc,
    const SlidingTable& sliding, std::vector<Candidate>& out)
{
    // non-sliders via table (pawn/king/knight) use precomputed attack masks
    // example for knight (assume we know team/forward externally)
    // for sliding piece: get attacks via sliding.rook_attacks_from or bishop_attacks_from using occupancy (own+enemy)
}
