#pragma once

#include "ShaderCache.h"
#include "SpriteDrawData.h"
#include "TextureCache.h"
#include <array>
#include <d2d1.h>
#include <d3d11.h>
#include <dwrite.h>
#include <string_view>
#include <windows.h>
#include <wrl/client.h>

struct Vertex {
  float x;
  float y;
};

struct SpriteVertex {
  std::array<float, 2> position;
  std::array<float, 2> uv;
};

class Graphics {
public:
  Graphics(HWND window, UINT width, UINT height);
  ~Graphics() = default;

  Graphics(const Graphics &) = delete;
  Graphics &operator=(const Graphics &) = delete;
  Graphics(Graphics &&) = delete;
  Graphics &operator=(Graphics &&) = delete;

  void Resize(UINT width, UINT height);
  void BeginFrame(double gameTimeSeconds, double realTimeSeconds);
  void DrawSprite(const SpriteDrawData &sprite);
  void DrawDebugText(std::wstring_view text);
  void DrawUiPanel(std::wstring_view text,
                   const std::array<float, 4> &bounds);
  void EndFrame();

private:
  struct SpriteTintConstants {
    std::array<float, 4> tint;
    float timeSource;
    float shape;
    std::array<float, 2> padding;
    std::array<float, 4> uvRect;
    float dissolveProgress = 0.0f;
    float dissolveNoiseScale = 14.0f;
    float dissolveEdgeWidth = 0.08f;
    float dissolveEdgeStrength = 2.0f;
    std::array<float, 4> dissolveEdgeColor{0.15f, 0.8f, 1.0f, 1.0f};
  };

  struct FrameConstants {
    float gameTimeSeconds;
    float realTimeSeconds;
    std::array<float, 2> padding;
  };

  // HLSL register declarations and these slots form the shader interface.
  static constexpr UINT kSpriteTextureSlot = 0;   // t0
  static constexpr UINT kSpriteSamplerSlot = 0;   // s0
  static constexpr UINT kSpriteTransformSlot = 0; // VS b0
  static constexpr UINT kSpriteTintSlot = 1;      // PS b1
  static constexpr UINT kFrameSlot = 2;           // PS b2

  struct SpriteConstants {
    std::array<float, 2> center;
    std::array<float, 2> size;
    std::array<float, 2> screenSize;
    float rotationRadians;
    float padding;
  };

  static void ThrowIfFailed(HRESULT hr, const char *message);

  void CreateDevice(HWND window, UINT width, UINT height);
  void CreateDebugTextResources();
  void CreateDebugTextRenderTarget();
  void ReleaseDebugTextRenderTarget();
  void CreateRenderTarget();
  void CreateSpriteResources();
  void CreateSpriteSamplers();
  void CreateAlphaBlendState();
  void CreateAdditiveBlendState();
  void BindBlendState(SpriteBlendMode blendMode);
  void BindSpriteTexture(SpriteTextureId texture);
  void BindSpriteSampler(SpriteSamplerMode samplerMode,
                         SpriteAddressMode addressMode);
  void BindSpritePipeline();
  void UpdateSpriteTintConstants();
  void CreateFrameConstantBuffer();
  void UpdateFrameConstants();
  void CreateSpriteConstantBuffer();
  void UpdateSpriteConstants();
  void SetSpriteTransform(float centerX, float centerY, float width, float height,
                          float rotationRadians);
  void SetViewport(UINT width, UINT height);
  void ReleaseRenderTarget();
  void ClearBackBuffer(const std::array<float, 4> &color);

  Microsoft::WRL::ComPtr<ID3D11Device> device_;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
  Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_;
  Microsoft::WRL::ComPtr<ID2D1Factory> d2dFactory_;
  Microsoft::WRL::ComPtr<IDWriteFactory> dwriteFactory_;
  Microsoft::WRL::ComPtr<IDWriteTextFormat> debugTextFormat_;
  Microsoft::WRL::ComPtr<ID2D1RenderTarget> debugTextRenderTarget_;
  Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> debugTextBrush_;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer_;
  ShaderCache shaderCache_;
  TextureCache textureCache_;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> pointClampSampler_;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> linearClampSampler_;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> pointWrapSampler_;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> linearWrapSampler_;
  Microsoft::WRL::ComPtr<ID3D11BlendState> alphaBlendState_;
  Microsoft::WRL::ComPtr<ID3D11BlendState> additiveBlendState_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> spriteTintConstantBuffer_;
  SpriteTintConstants spriteTintConstants_{{1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, 0.0f, {},
                                           {0.0f, 0.0f, 1.0f, 1.0f}};
  Microsoft::WRL::ComPtr<ID3D11Buffer> frameConstantBuffer_;
  FrameConstants frameConstants_{};
  Microsoft::WRL::ComPtr<ID3D11Buffer> spriteConstantBuffer_;
  SpriteConstants spriteConstants_{};
};
