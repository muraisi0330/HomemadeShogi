#include "stdafx.h"
#include "Application/StartScene.h"



	 StartScene::StartScene(const InitData& init)
		: IScene{ init }
	{
		 getData().soundManager.PlayHomeBGM();
	}
	 void StartScene::GameStart() {
		 getData().soundManager.StopHomeBGM();
			 changeScene(SceneState::Game);
	 }
	 void StartScene::SettingsScreen() {
			 changeScene(SceneState::Settings, 0s);
	 }

	void StartScene::update()  {
		
		// ゲームスタート
		if (getData().controller.GameSceneMoveButton()) {
			GameStart();
		}
		// 設定画面へ
		if (getData().controller.SettingsSceneMoveButton()) {
			SettingsScreen();
		}
		
		if (getData().controller.ExitButton()) {
			System::Exit();
		}
		if (getData().controller.LicenseButton()) {
			//LicenseManager::ShowInBrowser();
			changeScene(SceneState::Credit);
		}
		
	}

	void StartScene::draw() const{
		getData().renderer.HomeScreenDraw();
		getData().renderer.ApplyScreenLight(getData().light);
	}
