#include "stdafx.h"
#include <Siv3D.hpp>
#include "Resource/ResourceManager.h"
#include "Application/StartScene.h"

// Main はヘッダで定義した Game / GameRenderer / GUIController を利用するのみ
void Main()
{

	App manager;
	// シーン登録
	manager.add<StartScene>(SceneState::Start);
	manager.add<SettingsScene>(SceneState::Settings);
	manager.add<GameScene>(SceneState::Game);
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

