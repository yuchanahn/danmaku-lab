#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <vector>
#include <wrl/client.h>
#include <xaudio2.h>

class AudioSystem {
public:
  AudioSystem();
  ~AudioSystem();

  AudioSystem(const AudioSystem &) = delete;
  AudioSystem &operator=(const AudioSystem &) = delete;
  AudioSystem(AudioSystem &&) = delete;
  AudioSystem &operator=(AudioSystem &&) = delete;

  void PlayShot();
  void PlayBgm();
  void StopBgm();

private:
  struct AudioClip {
    WAVEFORMATEX format{};
    std::vector<std::uint8_t> pcm;
  };

  static constexpr std::size_t kShotVoiceCount = 8;

  static AudioClip LoadPcmWav(const std::filesystem::path &path);
  static std::filesystem::path GetAssetPath(const wchar_t *fileName);
  void DestroyVoices() noexcept;

  Microsoft::WRL::ComPtr<IXAudio2> xaudio2_;
  IXAudio2MasteringVoice *masteringVoice_ = nullptr;
  AudioClip shotClip_;
  std::array<IXAudio2SourceVoice *, kShotVoiceCount> shotVoices_{};
  AudioClip bgmClip_;
  IXAudio2SourceVoice *bgmVoice_ = nullptr;
  bool bgmPlaying_ = false;
};
