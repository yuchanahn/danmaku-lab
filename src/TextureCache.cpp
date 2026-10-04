#include "TextureCache.h"

#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <windows.h>

namespace {
void ThrowIfFailed(HRESULT hr, const char *message) {
  if (FAILED(hr)) {
    throw std::runtime_error(message);
  }
}
} // namespace

void TextureCache::Initialize(ID3D11Device *device) {
  if (device == nullptr) {
    throw std::invalid_argument("TextureCache requires a valid D3D11 device.");
  }

  device_ = device;
  ThrowIfFailed(CoCreateInstance(CLSID_WICImagingFactory, nullptr,
                                 CLSCTX_INPROC_SERVER,
                                 IID_PPV_ARGS(&wicFactory_)),
                "Failed to create the WIC imaging factory.");

  constexpr std::array<std::array<std::uint8_t, 16>, 3> pixels{{
      {255, 0, 0, 255,      // Checker: red
       0, 255, 0, 255,      // green
       0, 0, 255, 255,      // blue
       255, 255, 255, 255}, // white
      {255, 220, 40, 255,   // Yellow: four opaque texels
       255, 220, 40, 255, 255, 220, 40, 255, 255, 220, 40, 255},
      {255, 255, 255, 255, // White: solid white for debug/background shapes
       255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
  }};

  D3D11_TEXTURE2D_DESC desc{};
  desc.Width = 2;
  desc.Height = 2;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_IMMUTABLE;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

  for (std::size_t i = 0; i < pixels.size(); ++i) {
    D3D11_SUBRESOURCE_DATA data{};
    data.pSysMem = pixels[i].data();
    data.SysMemPitch = 2 * 4;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    ThrowIfFailed(device->CreateTexture2D(&desc, &data, &texture),
                  "Failed to create a sprite texture.");
    ThrowIfFailed(
        device->CreateShaderResourceView(texture.Get(), nullptr, &textures_[i]),
        "Failed to create a sprite texture SRV.");
  }
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>
TextureCache::LoadTextureFromFile(const std::filesystem::path &path) const {
  Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
  ThrowIfFailed(wicFactory_->CreateDecoderFromFilename(
                    path.c_str(), nullptr, GENERIC_READ,
                    WICDecodeMetadataCacheOnLoad, &decoder),
                "Failed to open the texture file with WIC.");

  Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
  ThrowIfFailed(decoder->GetFrame(0, &frame),
                "Failed to decode the first texture frame.");

  UINT width = 0;
  UINT height = 0;
  ThrowIfFailed(frame->GetSize(&width, &height),
                "Failed to read the texture dimensions.");

  Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
  ThrowIfFailed(wicFactory_->CreateFormatConverter(&converter),
                "Failed to create the WIC format converter.");
  ThrowIfFailed(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA,
                                      WICBitmapDitherTypeNone, nullptr, 0.0,
                                      WICBitmapPaletteTypeCustom),
                "Failed to convert the texture to RGBA8.");

  constexpr UINT kBytesPerPixel = 4;
  const UINT rowPitch = width * kBytesPerPixel;
  std::vector<std::uint8_t> pixels(static_cast<std::size_t>(rowPitch) * height);
  ThrowIfFailed(converter->CopyPixels(nullptr, rowPitch,
                                      static_cast<UINT>(pixels.size()),
                                      pixels.data()),
                "Failed to copy decoded texture pixels.");

  D3D11_TEXTURE2D_DESC desc{};
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_IMMUTABLE;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

  D3D11_SUBRESOURCE_DATA initialData{};
  initialData.pSysMem = pixels.data();
  initialData.SysMemPitch = rowPitch;

  Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
  ThrowIfFailed(device_->CreateTexture2D(&desc, &initialData, &texture),
                "Failed to create the file texture.");

  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;
  ThrowIfFailed(
      device_->CreateShaderResourceView(texture.Get(), nullptr, &textureView),
      "Failed to create the file texture SRV.");
  return textureView;
}

std::filesystem::path TextureCache::GetTexturePath(SpriteTextureId id) const {
  if (id != SpriteTextureId::Player) {
    throw std::invalid_argument("This texture ID has no file-backed texture.");
  }

  wchar_t executablePath[MAX_PATH]{};
  if (GetModuleFileNameW(nullptr, executablePath, MAX_PATH) == 0) {
    throw std::runtime_error("Failed to locate the executable path.");
  }

  return std::filesystem::path(executablePath).parent_path() / L"assets" /
         L"player_test.png";
}

ID3D11ShaderResourceView *TextureCache::Get(SpriteTextureId id) {
  const auto index = static_cast<std::size_t>(id);
  if (index >= textures_.size()) {
    throw std::invalid_argument("Invalid sprite texture ID.");
  }

  if (!textures_[index]) {
    textures_[index] = LoadTextureFromFile(GetTexturePath(id));
  }

  return textures_[index].Get();
}
