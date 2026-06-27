#include "stdafx.h"
#include "Resource/Sound.h"


Sound::Sound() {
	const TOMLReader toml{ U"example/ExternalConfigurationFile/Sound.toml" };
	if (not toml) {
		ShidenAIConfig::Sound::PIECE_DROP_VOLUME_ADJUST =2;
		ShidenAIConfig::Sound::PROMOTION_VOLUME_ADJUST =1;
		ShidenAIConfig::Sound::WIN_VOLUME_ADJUST = 2;
		ShidenAIConfig::Sound::STARTUP_VOLUME_ADJUST = 0.4;
		return;
	}
	ShidenAIConfig::Sound::PIECE_DROP_VOLUME_ADJUST = toml[U"Sound.VolumeAdjust.PieceDrop"].get<double>();
	ShidenAIConfig::Sound::PROMOTION_VOLUME_ADJUST = toml[U"Sound.VolumeAdjust.Promotion"].get<double>();
	ShidenAIConfig::Sound::WIN_VOLUME_ADJUST = toml[U"Sound.VolumeAdjust.Win"].get<double>();
	ShidenAIConfig::Sound::STARTUP_VOLUME_ADJUST = toml[U"Sound.VolumeAdjust.Startup"].get<double>();
}

	void Sound::PlayPieceDropSound()noexcept {
		AudioAsset(U"駒打ち").setVolume(ShidenAIConfig::Sound::PIECE_DROP_VOLUME_ADJUST* effectVolume).play();// 駒打ち音を再生
	}
	void Sound::PlayWinSound()noexcept {
		AudioAsset(U"勝利音").setVolume(ShidenAIConfig::Sound::WIN_VOLUME_ADJUST * effectVolume).play();
	}
	void Sound::PlayPromotionSound()noexcept {
		AudioAsset(U"成り音").setVolume(ShidenAIConfig::Sound::PROMOTION_VOLUME_ADJUST * effectVolume).play();
	}
	void Sound::PlayStartupSound()noexcept {
		AudioAsset(U"起動音").setVolume(ShidenAIConfig::Sound::STARTUP_VOLUME_ADJUST * effectVolume).play();
	}
	void Sound::HandleGameEvent(GameEvent e)
	{
		switch (e)
		{
		case GameEvent::PieceMoved:
		case GameEvent::PieceDropped:
			PlayPieceDropSound();
			break;
		case GameEvent::Promoted:
			PlayPromotionSound();
			break;
		case GameEvent::GameWon:
			PlayWinSound();
			break;
		case GameEvent::GameStart:
			PlayStartupSound();
			break;
		}
	}
	
	void Sound::PlayHomeBGM()noexcept {
		AudioAsset(U"ホームBGM").setVolume(bgmVolume).play();
	}
	void Sound::PlayBGM()noexcept {
		AudioAsset(U"ゲームBGM").setVolume(bgmVolume).play();
	}
	void Sound::StopBGM()noexcept {
		AudioAsset(U"ゲームBGM").stop();
	}
	void Sound::StopHomeBGM()noexcept {
		AudioAsset(U"ホームBGM").stop();
	}

	void Sound::SetBGMVolume(double v) noexcept {
		bgmVolume = v;
	}
	void Sound::SetEffectVolume(double v) noexcept {
		effectVolume = v;
	}
	double Sound::GetBGMVolume()  noexcept {
		return bgmVolume;
	}
	double Sound::GetEffectVolume()  noexcept {
		return effectVolume;
	}
