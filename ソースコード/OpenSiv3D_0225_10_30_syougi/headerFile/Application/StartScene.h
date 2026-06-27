#pragma once
#include "Application/SceneData.h"



class StartScene : public App::Scene {
public:
	StartScene(const InitData& init);
	void update() override;// 更新
	void draw() const override;// 描画
	void GameStart();// ゲームスタート
	void SettingsScreen();// 設定画面へ
};
class SettingsScene : public App::Scene {
public:
	SettingsScene(const InitData& init);
	void update() override;
	void HomeScreen();
	void Setting();
	void PlayerSettings();
	void draw() const override;
};
class GameScene : public App::Scene {
public:
	GameScene(const InitData& init);

	void update() override;
	void draw() const override;
private:
};
