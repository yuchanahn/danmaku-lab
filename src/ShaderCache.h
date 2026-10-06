#pragma once

#include <d3d11.h>
#include <d3dcommon.h>
#include <wrl/client.h>

class ShaderCache {
public:
  void Initialize(ID3D11Device *device);

  ID3D11VertexShader *GetSpriteVertexShader() const noexcept;
  ID3D11PixelShader *GetSpritePixelShader() const noexcept;
  ID3DBlob *GetSpriteVertexBytecode() const noexcept;
  ID3D11VertexShader *GetBulletInstanceVertexShader() const noexcept;
  ID3D11PixelShader *GetBulletInstancePixelShader() const noexcept;
  ID3DBlob *GetBulletInstanceVertexBytecode() const noexcept;

private:
  Microsoft::WRL::ComPtr<ID3D11VertexShader> spriteVertexShader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> spritePixelShader_;
  Microsoft::WRL::ComPtr<ID3DBlob> spriteVertexBytecode_;
  Microsoft::WRL::ComPtr<ID3D11VertexShader> bulletInstanceVertexShader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> bulletInstancePixelShader_;
  Microsoft::WRL::ComPtr<ID3DBlob> bulletInstanceVertexBytecode_;
};
