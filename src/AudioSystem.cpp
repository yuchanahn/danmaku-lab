#include "AudioSystem.h"

#include <cstring>
#include <fstream>
#include <stdexcept>
#include <windows.h>

namespace {
void ThrowIfFailed(HRESULT hr, const char *message) {
  if (FAILED(hr)) {
    throw std::runtime_error(message);
  }
}

template <typename T> void ReadValue(std::ifstream &stream, T &value) {
  stream.read(reinterpret_cast<char *>(&value), sizeof(value));
  if (!stream) {
    throw std::runtime_error("Unexpected end of WAV file.");
  }
}

void ReadFourCC(std::ifstream &stream, char (&fourCC)[4]) {
  stream.read(fourCC, sizeof(fourCC));
  if (!stream) {
    throw std::runtime_error("Unexpected end of WAV file.");
  }
}
} // namespace

AudioSystem::AudioSystem() {
  try {
    ThrowIfFailed(XAudio2Create(&xaudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR),
                  "Failed to create XAudio2.");
    ThrowIfFailed(xaudio2_->CreateMasteringVoice(&masteringVoice_),
                  "Failed to create the XAudio2 mastering voice.");

    shotClip_ = LoadPcmWav(GetAssetPath(L"shot_test.wav"));

    for (auto &voice : shotVoices_) {
      ThrowIfFailed(xaudio2_->CreateSourceVoice(&voice, &shotClip_.format),
                    "Failed to create a shot source voice.");
      ThrowIfFailed(voice->Start(), "Failed to start a shot source voice.");
    }

    bgmClip_ = LoadPcmWav(GetAssetPath(L"bgm_test.wav"));
    ThrowIfFailed(xaudio2_->CreateSourceVoice(&bgmVoice_, &bgmClip_.format),
                  "Failed to create the BGM source voice.");
  } catch (...) {
    DestroyVoices();
    throw;
  }
}

AudioSystem::~AudioSystem() { DestroyVoices(); }

void AudioSystem::DestroyVoices() noexcept {
  for (auto *&voice : shotVoices_) {
    if (voice != nullptr) {
      voice->DestroyVoice();
      voice = nullptr;
    }
  }
  if (bgmVoice_ != nullptr) {
    bgmVoice_->DestroyVoice();
    bgmVoice_ = nullptr;
  }
  if (masteringVoice_ != nullptr) {
    masteringVoice_->DestroyVoice();
    masteringVoice_ = nullptr;
  }
}

void AudioSystem::PlayShot() {
  for (auto *voice : shotVoices_) {
    XAUDIO2_VOICE_STATE state{};
    voice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
    if (state.BuffersQueued != 0) {
      continue;
    }

    XAUDIO2_BUFFER buffer{};
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.AudioBytes = static_cast<UINT32>(shotClip_.pcm.size());
    buffer.pAudioData = shotClip_.pcm.data();
    ThrowIfFailed(voice->SubmitSourceBuffer(&buffer),
                  "Failed to submit the shot audio buffer.");
    return;
  }
}

void AudioSystem::PlayBgm() {
  if (bgmPlaying_) {
    return;
  }

  XAUDIO2_BUFFER buffer{};
  buffer.Flags = XAUDIO2_END_OF_STREAM;
  buffer.AudioBytes = static_cast<UINT32>(bgmClip_.pcm.size());
  buffer.pAudioData = bgmClip_.pcm.data();
  buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

  ThrowIfFailed(bgmVoice_->SubmitSourceBuffer(&buffer),
                "Failed to submit the BGM audio buffer.");
  ThrowIfFailed(bgmVoice_->Start(), "Failed to start the BGM source voice.");
  bgmPlaying_ = true;
}

void AudioSystem::StopBgm() {
  if (!bgmPlaying_) {
    return;
  }

  ThrowIfFailed(bgmVoice_->Stop(), "Failed to stop the BGM source voice.");
  ThrowIfFailed(bgmVoice_->FlushSourceBuffers(),
                "Failed to clear the BGM source voice buffers.");
  bgmPlaying_ = false;
}

AudioSystem::AudioClip
AudioSystem::LoadPcmWav(const std::filesystem::path &path) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream) {
    throw std::runtime_error("Failed to open the WAV file.");
  }

  char riff[4]{};
  char wave[4]{};
  std::uint32_t riffSize = 0;
  ReadFourCC(stream, riff);
  ReadValue(stream, riffSize);
  ReadFourCC(stream, wave);
  (void)riffSize;

  if (std::memcmp(riff, "RIFF", 4) != 0 ||
      std::memcmp(wave, "WAVE", 4) != 0) {
    throw std::runtime_error("Invalid WAV RIFF header.");
  }

  AudioClip clip;
  bool foundFormat = false;
  bool foundData = false;

  while (stream && (!foundFormat || !foundData)) {
    char chunkId[4]{};
    std::uint32_t chunkSize = 0;
    ReadFourCC(stream, chunkId);
    ReadValue(stream, chunkSize);

    if (std::memcmp(chunkId, "fmt ", 4) == 0) {
      if (chunkSize < 16) {
        throw std::runtime_error("Invalid WAV format chunk.");
      }

      ReadValue(stream, clip.format.wFormatTag);
      ReadValue(stream, clip.format.nChannels);
      ReadValue(stream, clip.format.nSamplesPerSec);
      ReadValue(stream, clip.format.nAvgBytesPerSec);
      ReadValue(stream, clip.format.nBlockAlign);
      ReadValue(stream, clip.format.wBitsPerSample);
      clip.format.cbSize = 0;

      if (chunkSize > 16) {
        stream.seekg(static_cast<std::streamoff>(chunkSize - 16), std::ios::cur);
      }
      foundFormat = true;
    } else if (std::memcmp(chunkId, "data", 4) == 0) {
      clip.pcm.resize(chunkSize);
      stream.read(reinterpret_cast<char *>(clip.pcm.data()), chunkSize);
      if (!stream) {
        throw std::runtime_error("Failed to read WAV PCM data.");
      }
      foundData = true;
    } else {
      stream.seekg(static_cast<std::streamoff>(chunkSize), std::ios::cur);
    }

    if ((chunkSize & 1u) != 0u) {
      stream.seekg(1, std::ios::cur);
    }
  }

  if (!foundFormat || !foundData || clip.format.wFormatTag != WAVE_FORMAT_PCM ||
      clip.pcm.empty()) {
    throw std::runtime_error("Only non-empty PCM WAV files are supported.");
  }

  return clip;
}

std::filesystem::path AudioSystem::GetAssetPath(const wchar_t *fileName) {
  wchar_t executablePath[MAX_PATH]{};
  if (GetModuleFileNameW(nullptr, executablePath, MAX_PATH) == 0) {
    throw std::runtime_error("Failed to locate the executable path.");
  }

  return std::filesystem::path(executablePath).parent_path() / L"assets" /
         fileName;
}
