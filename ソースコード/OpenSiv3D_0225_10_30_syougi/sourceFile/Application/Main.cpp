#include "stdafx.h"
#include <Siv3D.hpp>
#include "Resource/ResourceManager.h"
#include "Application/StartScene.h"
using namespace std;

// Main はヘッダで定義した Game / GameRenderer / GUIController を利用するのみ
void Main()
{
	ios::sync_with_stdio(false);
	cin.tie(nullptr);
	App manager;
	// シーン登録
	manager.add<StartScene>(SceneState::Start);
	manager.add<SettingsScene>(SceneState::Settings);
	manager.add<GameScene>(SceneState::Game);
	manager.add<CreditScene>(SceneState::Credit);
	//Window::Resize(1920, 1080);
	// アセットを一括登録（TextureAsset / AudioAsset の登録はここで行う）
	ResourceManager::RegisterAll();
	while (System::Update())
	{
		if (not manager.update())
		{
			break;
		}
	}
	//終了時に必要ならアンロード（現状は省略可能）
	ResourceManager::UnregisterAll();
}

