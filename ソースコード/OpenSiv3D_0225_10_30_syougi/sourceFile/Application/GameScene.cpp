#include "stdafx.h"
#include "Application/StartScene.h"



GameScene::GameScene(const InitData& init)
	: IScene{ init }
{
	getData().game.SetOnGameEvent([&](GameEvent e) {
		getData().soundManager.HandleGameEvent(e);
   });
	getData().game.ResetState();
	getData().game.ResetTimer();
	getData().soundManager.PlayBGM();
	getData().soundManager.HandleGameEvent(GameEvent::GameStart);
	getData().blueAI = AIFactory::Create(getData().bluePlayerMode);
	getData().redAI = AIFactory::Create(getData().redPlayerMode);
	getData().renderer.SetSenteName(GetPlayerName(getData().bluePlayerMode));
	getData().renderer.SetGoteName(GetPlayerName(getData().redPlayerMode));
}

void GameScene::update() {
	getData().game.UpdateElapsedTime();
	getData().controller.ClearGUI();
	// 1) 入力と状態更新
	getData().controller.UpdateInputs();
	// 手番のチームを取得
	Team currentTeam = getData().game.GetCurrentTeam();
	// 現在のチームに応じたAIを取得
	IShogiAI* currentAI = (currentTeam == Team::Blue) ? getData().blueAI.get() : getData().redAI.get();

	if (currentAI) {
		// AIのターン：AIに任せる
		currentAI->MakeMove(getData().game, currentTeam);
	}
	else {
		// 人間のターン：コントローラー経由の入力を受け付ける
		getData().controller.ProcessBoardClick();
		getData().controller.HandleUnitSelection();
		getData().controller.EvaluatePromotions();
	}
	if (getData().controller.ReturnToHomeButton()) {
		getData().soundManager.StopBGM();
		changeScene(SceneState::Start, 0s);
	}

}

void GameScene::draw() const {
	// 2) 描画
	getData().renderer.DrawBackground();
	getData().renderer.DrawBoard();
	getData().renderer.DrawMoveRangeOverlay();
	getData().renderer.UpdateHUD();
	getData().renderer.DrawUnits();
	getData().renderer.DrawPromotionDialog();
	getData().renderer.DrawVictoryScreen();
	getData().renderer.ApplyScreenLight(getData().light);
}
