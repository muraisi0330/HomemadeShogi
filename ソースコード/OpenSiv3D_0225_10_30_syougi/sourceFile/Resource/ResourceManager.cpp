#include "stdafx.h"
#include "Resource/ResourceManager.h"

namespace ResourceManager
{
	void RegisterAll()
	{
		// フォント
		FontAsset::Register(U"UnitNameFont", 20, Typeface::Bold);// ユニット名表示用フォント
		FontAsset::Register(U"UnitNameFont1", 8, Typeface::Bold);// GUI 用小フォント
		FontAsset::Register(U"UnitNameFont3", 15, Typeface::Bold);// 駒名表示用フォント
		FontAsset::Register(U"WinFont", 70, Typeface::Bold);// ウィン画面用フォント

		// テクスチャ（ファイル名は既存コードに合わせる）
		TextureAsset::Register(U"将棋台", U"example/ita.png");
		TextureAsset::Register(U"看板", U"example/ita1.png");
		TextureAsset::Register(U"掛け軸", U"example/kakeziku.png");// 掛け軸
		TextureAsset::Register(U"駒", U"example/koma.png");// 駒
		// 補助：背景やタイトル等があればここに登録（Main.cpp の定義と合わせる）
		TextureAsset::Register(U"ホーム背景", U"example/sa.png");// タイトル背景
		TextureAsset::Register(U"タイトル", U"example/asdf.png");
		TextureAsset::Register(U"畳", U"example/tatami.png");// 畳テクスチャ

		// オーディオ（mp3 はストリームで登録している既存コードに合わせる）
		AudioAsset::Register(U"駒打ち", Audio::Stream, U"example/将棋の駒を打つ.mp3");// 駒打ち
		AudioAsset::Register(U"勝利音", Audio::Stream, U"example/和太鼓でカカッ.mp3");// 勝利音
		AudioAsset::Register(U"成り音", Audio::Stream, U"example/拍子木2.mp3");// 成り音
		AudioAsset::Register(U"起動音", Audio::Stream, U"example/尺八.mp3");// 起動音
		AudioAsset::Register(U"ホームBGM", Audio::Stream, U"example/bgm2.mp3");// ホームBGM
		AudioAsset::Register(U"ゲームBGM", Audio::Stream, U"example/bgm1.mp3");// ゲームBGM
	}

	void UnregisterAll()
	{
		// 必要ならアンロードする処理を追加
	}
}
