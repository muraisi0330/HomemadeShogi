#include "stdafx.h"
#include "Application/StartScene.h"

CreditScene::CreditScene(const InitData& init)
	: IScene{ init }
{
}
void CreditScene::update()
{
		if (getData().controller.StartSceneMoveButton()) {
		  changeScene(SceneState::Start);
	     }
		if (getData().controller.LicenseButton()) {
			LicenseManager::ShowInBrowser();
		}
}
void CreditScene::draw() const
{
	getData().renderer.CreditScreenDraw();
	getData().renderer.ApplyScreenLight(getData().light);
}
