#pragma once
#include <Siv3D.hpp>

namespace ResourceManager
{
	// アセットを一括登録（アプリ起動時に一度呼ぶ）
	void RegisterAll();

	// 必要ならアセット解放処理をここに追加
	void UnregisterAll();
}
