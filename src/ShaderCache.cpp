#include "ShaderCache.h"

#include <d3dcompiler.h>
#include <filesystem>
#include <stdexcept>
#include <windows.h>

namespace {
void ThrowIfFailed(HRESULT hr, const char *message) {
  if (FAILED(hr)) {
    throw std::runtime_error(message);
  }
}

Microsoft::WRL::ComPtr<ID3DBlob>
CompileShader(const std::filesystem::path &path, const char *entryPoint,
              const char *target) {
  UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
  flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  Microsoft::WRL::ComPtr<ID3DBlob> bytecode;
  Microsoft::WRL::ComPtr<ID3DBlob> errors;
  const HRESULT hr = D3DCompileFromFile(
      path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint,
      target, flags, 0, &bytecode, &errors);

  if (FAILED(hr)) {
    const char *message =
        errors ? static_cast<const char *>(errors->GetBufferPointer())
               : "Failed to compile a sprite shader.";
    throw std::runtime_error(message);
  }

  return bytecode;
}
} // namespace

void ShaderCache::Initialize(ID3D11Device *device) {
  if (device == nullptr) {
    throw std::invalid_argument("ShaderCache requires a valid D3D11 device.");
  }

  wchar_t executablePath[MAX_PATH]{};
  if (GetModuleFileNameW(nullptr, executablePath, MAX_PATH) == 0) {
    throw std::runtime_error("Failed to locate the executable path.");
  }

  const auto shaderPath = std::filesystem::path(executablePath).parent_path() /
                          L"shaders" / L"Sprite.hlsl";

  spriteVertexBytecode_ = CompileShader(shaderPath, "VSMain", "vs_5_0");
  const auto pixelBytecode = CompileShader(shaderPath, "PSMain", "ps_5_0");

  ThrowIfFailed(device->CreateVertexShader(
                    spriteVertexBytecode_->GetBufferPointer(),
                    spriteVertexBytecode_->GetBufferSize(), nullptr,
                    &spriteVertexShader_),
                "Failed to create the sprite vertex shader.");

  ThrowIfFailed(device->CreatePixelShader(pixelBytecode->GetBufferPointer(),
                                          pixelBytecode->GetBufferSize(),
                                          nullptr, &spritePixelShader_),
                "Failed to create the sprite pixel shader.");

  bulletInstanceVertexBytecode_ =
      CompileShader(shaderPath, "BulletInstanceVSMain", "vs_5_0");
  const auto bulletPixelBytecode =
      CompileShader(shaderPath, "BulletInstancePSMain", "ps_5_0");
  ThrowIfFailed(device->CreateVertexShader(
                    bulletInstanceVertexBytecode_->GetBufferPointer(),
                    bulletInstanceVertexBytecode_->GetBufferSize(), nullptr,
                    &bulletInstanceVertexShader_),
                "Failed to create the bullet instance vertex shader.");
  ThrowIfFailed(device->CreatePixelShader(
                    bulletPixelBytecode->GetBufferPointer(),
                    bulletPixelBytecode->GetBufferSize(), nullptr,
                    &bulletInstancePixelShader_),
                "Failed to create the bullet instance pixel shader.");
}

ID3D11VertexShader *ShaderCache::GetSpriteVertexShader() const noexcept {
  return spriteVertexShader_.Get();
}

ID3D11PixelShader *ShaderCache::GetSpritePixelShader() const noexcept {
  return spritePixelShader_.Get();
}

ID3DBlob *ShaderCache::GetSpriteVertexBytecode() const noexcept {
  return spriteVertexBytecode_.Get();
}

ID3D11VertexShader *ShaderCache::GetBulletInstanceVertexShader() const noexcept {
  return bulletInstanceVertexShader_.Get();
}

ID3D11PixelShader *ShaderCache::GetBulletInstancePixelShader() const noexcept {
  return bulletInstancePixelShader_.Get();
}

ID3DBlob *ShaderCache::GetBulletInstanceVertexBytecode() const noexcept {
  return bulletInstanceVertexBytecode_.Get();
}
