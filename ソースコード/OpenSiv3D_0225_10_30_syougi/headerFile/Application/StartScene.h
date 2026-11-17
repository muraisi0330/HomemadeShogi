#pragma once
#include "Application/SceneData.h"


class StartScene : public App::Scene {
public:
	StartScene(const InitData& init);
	void update() override;// 更新
	void draw() const override;// 描画
	void GameStart();// ゲームスタート
	void SettingsScreen();// 設定画面へ
	void drawFadeIn(double t) const override
	{
		draw();
		//桜の襖が徐々に開く演出
		getData().renderer.DrawFadeIn(t);
	}

	void drawFadeOut(double t) const override
	{
		draw();
		
		getData().renderer.DrawFadeOut(t);
	}
};
class SettingsScene : public App::Scene {
public:
	SettingsScene(const InitData& init);
	void update() override;
	void HomeScreen();
	void Setting();
	void PlayerSettings();
	void draw() const override;
	void drawFadeIn(double t) const override
	{
		draw();
		//桜の襖が徐々に開く演出
		getData().renderer.DrawFadeIn(t);
	}

	void drawFadeOut(double t) const override
	{
		draw();

		getData().renderer.DrawFadeOut(t);
	}
};
class GameScene : public App::Scene {
public:
	GameScene(const InitData& init);

	void update() override;
	void draw() const override;
	void drawFadeIn(double t) const override
	{
		draw();
		//桜の襖が徐々に開く演出
		getData().renderer.DrawFadeIn(t);
	}

	void drawFadeOut(double t) const override
	{
		draw();

		getData().renderer.DrawFadeOut(t);
	}
private:
};
//クレジットシーン
class CreditScene : public App::Scene {
	public:
	CreditScene(const InitData& init);
	void update() override;
	void draw() const override;
	void drawFadeIn(double t) const override
	{
		draw();
		//桜の襖が徐々に開く演出
		getData().renderer.DrawFadeIn(t);
	}

	void drawFadeOut(double t) const override
	{
		draw();

		getData().renderer.DrawFadeOut(t);
	}
};
