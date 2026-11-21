#pragma once
#include "Types/types.h"

//音量バランス調整
namespace SoundConfig {
	namespace Sound {
		//駒打ち音音量バランス調整
		inline  double PIECE_DROP_VOLUME_ADJUST = 2;
		//成り音音量バランス調整
		inline  double PROMOTION_VOLUME_ADJUST = 1;
		//勝利音音量バランス調整
		inline  double WIN_VOLUME_ADJUST = 2;
		//起動音音量バランス調整
		inline  double STARTUP_VOLUME_ADJUST = 0.4;
	}
}


// Sound モジュール：効果音関連の処理をまとめる
class Sound
{
public:
	Sound();

	// 駒打ち音を再生
	void PlayPieceDropSound()noexcept;
	// 成り音を再生
	void PlayPromotionSound() noexcept;
	// 勝利音を再生
	void PlayWinSound() noexcept;
	// 起動音を再生
	void PlayStartupSound() noexcept;
	//BGM再生
	void PlayBGM() noexcept;
	void StopBGM() noexcept;
	//ホームBGM再生
	void PlayHomeBGM() noexcept;
	void StopHomeBGM() noexcept;

	void HandleGameEvent(GameEvent e);
	//音量設定
	void SetEffectVolume(const double volume) noexcept;
	void SetBGMVolume(const double volume) noexcept;

	double GetEffectVolume() noexcept;
	double GetBGMVolume() noexcept;
private:
	// 音量変数
	double effectVolume = 1;
	double bgmVolume = 1;
};
