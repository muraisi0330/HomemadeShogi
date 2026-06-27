# CONTRIBUTING.md

## ガイドライン: マジックナンバー排除

このプロジェクトでは「マジックナンバー（意味を持たない生の数値リテラル）」の使用を禁止します。数値は必ず意味のある名前付き定数に置き換え、該当するヘッダに定義してください。

理由:
- 可読性の向上
- 保守性の向上（値の一元管理）
- バグ防止（意図しない値の多重定義や不整合を回避）

ルール:
1. 定数は `constexpr` を使って宣言し、可能なら型を明示してください（例: `constexpr int KILLER_TABLE_PLY = 32;`）。
2. 定数は既存の `Config::AI` 名前空間内に置くか、用途別に `Constants*.h` に追加してください。重複定義を避けてください。
3. 配列サイズ、ループ境界、ビット幅、ハッシュスロット数などの数値は必ず定数で定義してください。
4. 魔法数字を置換する際は小さなコミットに分け、各コミットでビルドとテストを実行してください。
5. 定数名は用途が分かる名前にすること（例: `ZOBRIST_HAND_SLOTS`, `KILLER_TABLE_PLY`, `BOARD_SQUARES`）。
6. 既に意味を持つ定数が存在する場合はそれを再利用し、新規定義は行わないでください。

実装例:
- `ShidenAI.h` の `for (int i = 0; i < 32; ++i)` を `for (int i = 0; i < AIConstants::KILLER_TABLE_PLY; ++i)` に置換
- `zobristHand` の第三次元で使われている `20` を `Config::AI::ZOBRIST_HAND_SLOTS` に置換

新規ヘッダの例 (`ConstantsAI.h`):
```cpp
#pragma once

#include <cstdint>

namespace AIConstants {
    // キラームーブ用の深さ（PLY）上限。ShidenAI の killerMoves 配列サイズと整合させる。
    constexpr int KILLER_TABLE_PLY = 32;

    // 持ち駒 Zobrist ハッシュ用のスロット数（Config::AI に同等の定義がない場合）
    constexpr int ZOBRIST_HAND_SLOTS = 20;

    // 盤面のマス数（NumTiles と整合）
    constexpr int BOARD_SQUARES = 9 * 9;
}
```

適用手順:
1. まず `ConstantsAI.h` を追加し、必要な定数を定義する（小さな単位で行う）。
2. 定数を使用するソースにヘッダを追加し、マジックナンバーを置換する。
3. コンパイルとテストを実行し、問題がないことを確認する。
4. コードレビューで既存定数との重複や不整合がないかを確認する。

補足:
- `Config::AI` に既に多数の定数が定義されています。可能ならその名前空間を共通参照元として利用してください。
- 大規模なリファクタ（名前空間移動など）を行う場合は、設計ノートに変更を残してください。

この CONTRIBUTING.md はプロジェクトのコーディング規約の一部として管理してください。