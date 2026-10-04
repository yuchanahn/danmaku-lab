#pragma once

#include "SpriteDrawData.h"
#include <array>
#include <d3d11.h>
#include <filesystem>
#include <wincodec.h>
#include <wrl/client.h>

class TextureCache {
public:
  void Initialize(ID3D11Device *device);
  ID3D11ShaderResourceView *Get(SpriteTextureId id);

private:
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>
  LoadTextureFromFile(const std::filesystem::path &path) const;
  std::filesystem::path GetTexturePath(SpriteTextureId id) const;

  Microsoft::WRL::ComPtr<ID3D11Device> device_;
  Microsoft::WRL::ComPtr<IWICImagingFactory> wicFactory_;
  std::array<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>, 4> textures_;
};
