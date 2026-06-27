#include "stdafx.h"
#include "Application/StartScene.h"


 SettingsScene::SettingsScene(const InitData& init)
	: IScene{ init }
{
}
 void SettingsScene::HomeScreen() {
		 changeScene(SceneState::Start,0s);
 }
 
 void SettingsScene::Setting() {
	 getData().light = getData().controller.DrawLightControl(getData().light);
	 getData().soundManager.SetEffectVolume(getData().controller.DrawEffectVolumeControl(getData().soundManager.GetEffectVolume()));
	 getData().soundManager.SetBGMVolume(getData().controller.DrawBgmVolumeControl(getData().soundManager.GetBGMVolume()));
 }
 
 void SettingsScene::PlayerSettings() {
	 if (getData().controller.ButtonToSwitch(BlueButtonRect)) {
		 getData().bluePlayerMode = CyclePlayerMode(getData().bluePlayerMode);
	 }
	 if (getData().controller.ButtonToSwitch(RedButtonRect)) {
		 getData().redPlayerMode = CyclePlayerMode(getData().redPlayerMode);
	 }
 }

void SettingsScene::update() {
	getData().soundManager.PlayHomeBGM();
	getData().renderer.SettingsScreenDraw();
	Setting();// 設定画面 GUI 処理
	if (getData().controller.StartSceneMoveButton()) {
		HomeScreen();// ホーム画面へ
	}

	PlayerSettings();
}

void SettingsScene::draw() const {
	getData().renderer.ButtonToSwitch(BlueButtonRect, U"先手：" + GetPlayerName(getData().bluePlayerMode));
	getData().renderer.ButtonToSwitch(RedButtonRect, U"後手：" + GetPlayerName(getData().redPlayerMode));
	getData().renderer.ApplyScreenLight(getData().light);
}
