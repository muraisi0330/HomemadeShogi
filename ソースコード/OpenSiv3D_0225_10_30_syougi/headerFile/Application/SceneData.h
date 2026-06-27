#pragma once
#include <Siv3D.hpp>
#include "Game/Game.h"
#include "AI/AI.h"
#include "AI/BasicTestAI.h"
#include "AI/SolidDefenseAI.h"
#include "AI/ShidenAI.h"
#include "Render/GameRenderer.h"
#include "Controller/GUIController.h"
#include "Resource/ResourceManager.h"
#include "Resource/Sound.h"

enum class SceneState {
	Start,
	Settings,
	Game,
};
enum class Player {
	Human,
	TestAI,
	SolidDefenseAI,
	IterativeDeepening,
};
struct GameData
{
	// ドメインロジック
	Game game;
	// AI ロジック
	std::unique_ptr<IShogiAI> blueAI;
	std::unique_ptr<IShogiAI> redAI;
	Sound soundManager{};

	// 描画責務
	GameRenderer renderer{ game };
	// 入力 / GUI 責務
	GUIController controller{ game };

	double light = 1.0;// 画面の明るさ調整
	Player bluePlayerMode = Player::Human;
	Player redPlayerMode = Player::IterativeDeepening;
};
// プレイヤーの種類を文字列に変換するヘルパー関数
inline  String GetPlayerName(Player p) {
	switch (p) {
	case Player::Human: return U"人間";
	case Player::TestAI: return U"清流";
	case Player::SolidDefenseAI: return U"金剛"; // 強いAIの名前
	case Player::IterativeDeepening: return U"紫電"; // 強いAIの名前
	default: return U"不明";
	}
}

// 次のプレイヤーモードに切り替えるヘルパー関数
inline  Player CyclePlayerMode(Player p) {
	switch (p) {
	case Player::Human: return Player::TestAI;
	case Player::TestAI: return Player::SolidDefenseAI;
	case Player::SolidDefenseAI: return Player::IterativeDeepening;
	case Player::IterativeDeepening: return Player::Human;
	default: return Player::Human;
	}
}
class AIFactory {
public:
	static std::unique_ptr<IShogiAI> Create(Player mode) {
		switch (mode) {
		case Player::TestAI:           return std::make_unique<BasicTestAI>();
		case Player::SolidDefenseAI:   return std::make_unique<SolidDefenseAI>();
		case Player::IterativeDeepening: return std::make_unique<ShidenAI>();
		default: return nullptr; // Human等の場合
		}
	}
};

using App = SceneManager<SceneState, GameData>;
