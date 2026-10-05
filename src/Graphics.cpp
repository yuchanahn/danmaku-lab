#include "Graphics.h"
#include <d2d1helper.h>

#include <cstdint>
#include <d3d11.h>
#include <stdexcept>
#include <winuser.h>

Graphics::Graphics(HWND window, UINT width, UINT height) {
  CreateDebugTextResources();
  CreateDevice(window, width, height);
  shaderCache_.Initialize(device_.Get());
  CreateSpriteResources();
  CreateSpriteConstantBuffer();
  CreateFrameConstantBuffer();
  textureCache_.Initialize(device_.Get());
  CreateSpriteSamplers();
  CreateAlphaBlendState();
  CreateAdditiveBlendState();
  SetViewport(width, height);
}

void Graphics::ThrowIfFailed(HRESULT hr, const char *message) {
  if (FAILED(hr)) {
    throw std::runtime_error(message);
  }
}

void Graphics::CreateDevice(HWND window, UINT width, UINT height) {
  DXGI_SWAP_CHAIN_DESC swapChainDesc{};
  swapChainDesc.BufferDesc.Width = width;
  swapChainDesc.BufferDesc.Height = height;
  swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  swapChainDesc.SampleDesc.Count = 1;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.BufferCount = 2;
  swapChainDesc.OutputWindow = window;
  swapChainDesc.Windowed = TRUE;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
  flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  constexpr std::array featureLevels{
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0,
  };

  D3D_FEATURE_LEVEL selectedFeatureLevel{};
  HRESULT hr = D3D11CreateDeviceAndSwapChain(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels.data(),
      static_cast<UINT>(featureLevels.size()), D3D11_SDK_VERSION,
      &swapChainDesc, &swapChain_, &device_, &selectedFeatureLevel, &context_);

#if defined(_DEBUG)
  if (hr == DXGI_ERROR_SDK_COMPONENT_MISSING) {
    flags &= ~D3D11_CREATE_DEVICE_DEBUG;
    hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels.data(),
        static_cast<UINT>(featureLevels.size()), D3D11_SDK_VERSION,
        &swapChainDesc, &swapChain_, &device_, &selectedFeatureLevel,
        &context_);
  }
#endif

  ThrowIfFailed(hr, "Failed to create the DirectX 11 device and swap chain.");

  if (selectedFeatureLevel < D3D_FEATURE_LEVEL_11_0) {
    throw std::runtime_error(
        "DirectX feature level 11.0 or newer is required.");
  }

  CreateRenderTarget();
}

void Graphics::CreateDebugTextResources() {
  ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                                  d2dFactory_.GetAddressOf()),
                "Failed to create the Direct2D factory.");

  ThrowIfFailed(
      DWriteCreateFactory(
          DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
          reinterpret_cast<IUnknown **>(dwriteFactory_.GetAddressOf())),
      "Failed to create the DirectWrite factory.");

  ThrowIfFailed(dwriteFactory_->CreateTextFormat(
                    L"Consolas", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
                    DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 16.0f,
                    L"en-us", &debugTextFormat_),
                "Failed to create the debug text format.");
  debugTextFormat_->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
}

void Graphics::CreateDebugTextRenderTarget() {
  Microsoft::WRL::ComPtr<IDXGISurface> backBufferSurface;
  ThrowIfFailed(swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBufferSurface)),
                "Failed to get the DXGI surface for debug text.");

  const auto properties = D2D1::RenderTargetProperties(
      D2D1_RENDER_TARGET_TYPE_DEFAULT,
      D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));
  ThrowIfFailed(
      d2dFactory_->CreateDxgiSurfaceRenderTarget(
          backBufferSurface.Get(), &properties, &debugTextRenderTarget_),
      "Failed to create the Direct2D debug text render target.");

  ThrowIfFailed(debugTextRenderTarget_->CreateSolidColorBrush(
                    D2D1::ColorF(D2D1::ColorF::White), &debugTextBrush_),
                "Failed to create the debug text brush.");
}

void Graphics::ReleaseDebugTextRenderTarget() {
  debugTextBrush_.Reset();
  debugTextRenderTarget_.Reset();
}

void Graphics::CreateSpriteResources() {

  constexpr std::array vertices{
      SpriteVertex{{-0.5f, 0.5f}, {0.0f, 0.0f}},  // 왼쪽 위
      SpriteVertex{{0.5f, 0.5f}, {1.0f, 0.0f}},   // 오른쪽 위
      SpriteVertex{{0.5f, -0.5f}, {1.0f, 1.0f}},  // 오른쪽 아래
      SpriteVertex{{-0.5f, -0.5f}, {0.0f, 1.0f}}, // 왼쪽 아래
  };

  constexpr std::array<std::uint16_t, 6> indices{0, 1, 2, 0, 2, 3};

  D3D11_BUFFER_DESC vertexBufferDesc{};
  vertexBufferDesc.ByteWidth = sizeof(vertices);
  vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
  vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

  D3D11_SUBRESOURCE_DATA vertexData{};
  vertexData.pSysMem = vertices.data();

  ThrowIfFailed(
      device_->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer_),
      "Failed to create the sprite vertex buffer.");

  D3D11_BUFFER_DESC indexBufferDesc{};
  indexBufferDesc.ByteWidth = sizeof(indices);
  indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
  indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

  D3D11_SUBRESOURCE_DATA indexData{};
  indexData.pSysMem = indices.data();

  ThrowIfFailed(
      device_->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer_),
      "Failed to create the quad index buffer.");

  constexpr D3D11_INPUT_ELEMENT_DESC inputElements[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
  };

  ID3DBlob *vertexBytecode = shaderCache_.GetSpriteVertexBytecode();

  ThrowIfFailed(device_->CreateInputLayout(
                    inputElements, static_cast<UINT>(std::size(inputElements)),
                    vertexBytecode->GetBufferPointer(),
                    vertexBytecode->GetBufferSize(), &inputLayout_),
                "Failed to create the sprite input layout.");

  static_assert(sizeof(SpriteTintConstants) == 80);

  D3D11_BUFFER_DESC constantBufferDesc{};
  constantBufferDesc.ByteWidth = sizeof(SpriteTintConstants);
  constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  constexpr SpriteTintConstants initialConstants{
      {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f, 0.0f, {}, {0.0f, 0.0f, 1.0f, 1.0f}};
  D3D11_SUBRESOURCE_DATA constantData{};
  constantData.pSysMem = &initialConstants;

  ThrowIfFailed(device_->CreateBuffer(&constantBufferDesc, &constantData,
                                      &spriteTintConstantBuffer_),
                "Failed to create the sprite tint constant buffer.");
}

void Graphics::CreateSpriteSamplers() {
  D3D11_SAMPLER_DESC desc{};
  desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  desc.MaxAnisotropy = 1;
  desc.MaxLOD = D3D11_FLOAT32_MAX;

  desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
  ThrowIfFailed(device_->CreateSamplerState(&desc, &pointClampSampler_),
                "Failed to create the point-clamp sprite sampler.");

  desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  ThrowIfFailed(device_->CreateSamplerState(&desc, &linearClampSampler_),
                "Failed to create the linear-clamp sprite sampler.");

  desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

  desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
  ThrowIfFailed(device_->CreateSamplerState(&desc, &pointWrapSampler_),
                "Failed to create the point-wrap sprite sampler.");

  desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  ThrowIfFailed(device_->CreateSamplerState(&desc, &linearWrapSampler_),
                "Failed to create the linear-wrap sprite sampler.");
}

void Graphics::CreateAlphaBlendState() {
  D3D11_BLEND_DESC desc{};
  auto &target = desc.RenderTarget[0];
  target.BlendEnable = TRUE;
  target.SrcBlend = D3D11_BLEND_SRC_ALPHA;
  target.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  target.BlendOp = D3D11_BLEND_OP_ADD;
  target.SrcBlendAlpha = D3D11_BLEND_ONE;
  target.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
  target.BlendOpAlpha = D3D11_BLEND_OP_ADD;
  target.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

  ThrowIfFailed(device_->CreateBlendState(&desc, &alphaBlendState_),
                "Failed to create the alpha blend state.");
}

void Graphics::CreateAdditiveBlendState() {
  D3D11_BLEND_DESC desc{};
  auto &target = desc.RenderTarget[0];
  target.BlendEnable = TRUE;
  target.SrcBlend = D3D11_BLEND_SRC_ALPHA;
  target.DestBlend = D3D11_BLEND_ONE;
  target.BlendOp = D3D11_BLEND_OP_ADD;
  target.SrcBlendAlpha = D3D11_BLEND_ZERO;
  target.DestBlendAlpha = D3D11_BLEND_ONE;
  target.BlendOpAlpha = D3D11_BLEND_OP_ADD;
  target.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

  ThrowIfFailed(device_->CreateBlendState(&desc, &additiveBlendState_),
                "Failed to create the additive blend state.");
}

void Graphics::BindBlendState(SpriteBlendMode blendMode) {
  ID3D11BlendState *blendState = nullptr;
  switch (blendMode) {
  case SpriteBlendMode::Alpha:
    blendState = alphaBlendState_.Get();
    break;
  case SpriteBlendMode::Additive:
    blendState = additiveBlendState_.Get();
    break;
  default:
    throw std::invalid_argument("Invalid sprite blend mode.");
  }

  context_->OMSetBlendState(blendState, nullptr, 0xFFFFFFFFu);
}

void Graphics::BindSpriteTexture(SpriteTextureId texture) {
  ID3D11ShaderResourceView *textureView = textureCache_.Get(texture);
  context_->PSSetShaderResources(kSpriteTextureSlot, 1, &textureView);
}

void Graphics::BindSpriteSampler(SpriteSamplerMode samplerMode,
                                 SpriteAddressMode addressMode) {
  ID3D11SamplerState *samplerState = nullptr;

  if (addressMode == SpriteAddressMode::Clamp) {
    switch (samplerMode) {
    case SpriteSamplerMode::Point:
      samplerState = pointClampSampler_.Get();
      break;
    case SpriteSamplerMode::Linear:
      samplerState = linearClampSampler_.Get();
      break;
    default:
      throw std::invalid_argument("Invalid sprite sampler type.");
    }
  } else if (addressMode == SpriteAddressMode::Wrap) {
    switch (samplerMode) {
    case SpriteSamplerMode::Point:
      samplerState = pointWrapSampler_.Get();
      break;
    case SpriteSamplerMode::Linear:
      samplerState = linearWrapSampler_.Get();
      break;
    default:
      throw std::invalid_argument("Invalid sprite sampler type.");
    }
  } else {
    throw std::invalid_argument("Invalid sprite address mode.");
  }

  context_->PSSetSamplers(kSpriteSamplerSlot, 1, &samplerState);
}

void Graphics::BindSpritePipeline() {
  context_->IASetInputLayout(inputLayout_.Get());

  UINT stride = sizeof(SpriteVertex);
  UINT offset = 0;
  context_->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride,
                               &offset);
  context_->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R16_UINT, 0);

  context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  context_->VSSetShader(shaderCache_.GetSpriteVertexShader(), nullptr, 0);
  context_->VSSetConstantBuffers(kSpriteTransformSlot, 1,
                                 spriteConstantBuffer_.GetAddressOf());
  context_->PSSetShader(shaderCache_.GetSpritePixelShader(), nullptr, 0);
  context_->PSSetConstantBuffers(kSpriteTintSlot, 1,
                                 spriteTintConstantBuffer_.GetAddressOf());
}

void Graphics::UpdateSpriteTintConstants() {
  D3D11_MAPPED_SUBRESOURCE mapped{};

  ThrowIfFailed(context_->Map(spriteTintConstantBuffer_.Get(), 0,
                              D3D11_MAP_WRITE_DISCARD, 0, &mapped),
                "Failed to map sprite tint constant buffer.");

  *static_cast<SpriteTintConstants *>(mapped.pData) = spriteTintConstants_;

  context_->Unmap(spriteTintConstantBuffer_.Get(), 0);
}

void Graphics::CreateFrameConstantBuffer() {
  static_assert(sizeof(FrameConstants) == 16);
  D3D11_BUFFER_DESC desc{};
  desc.ByteWidth = sizeof(FrameConstants);
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  ThrowIfFailed(device_->CreateBuffer(&desc, nullptr, &frameConstantBuffer_),
                "Failed to create the frame constant buffer.");
}

void Graphics::UpdateFrameConstants() {
  D3D11_MAPPED_SUBRESOURCE mapped{};
  ThrowIfFailed(context_->Map(frameConstantBuffer_.Get(), 0,
                              D3D11_MAP_WRITE_DISCARD, 0, &mapped),
                "Failed to map the frame constant buffer.");
  *static_cast<FrameConstants *>(mapped.pData) = frameConstants_;
  context_->Unmap(frameConstantBuffer_.Get(), 0);
}

void Graphics::SetSpriteTransform(float centerX, float centerY, float width,
                                  float height, float rotationRadians) {
  spriteConstants_.center[0] = centerX;
  spriteConstants_.center[1] = centerY;
  spriteConstants_.size[0] = width;
  spriteConstants_.size[1] = height;
  spriteConstants_.rotationRadians = rotationRadians;
}

void Graphics::CreateSpriteConstantBuffer() {
  static_assert(sizeof(SpriteConstants) == 32);
  static_assert(sizeof(SpriteConstants) % 16 == 0);

  D3D11_BUFFER_DESC desc{};
  desc.ByteWidth = sizeof(SpriteConstants);
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  ThrowIfFailed(device_->CreateBuffer(&desc, nullptr, &spriteConstantBuffer_),
                "Failed to create the sprite constant buffer.");
}

void Graphics::UpdateSpriteConstants() {
  D3D11_MAPPED_SUBRESOURCE mapped{};
  ThrowIfFailed(context_->Map(spriteConstantBuffer_.Get(), 0,
                              D3D11_MAP_WRITE_DISCARD, 0, &mapped),
                "Failed to map the sprite constant buffer.");
  *static_cast<SpriteConstants *>(mapped.pData) = spriteConstants_;
  context_->Unmap(spriteConstantBuffer_.Get(), 0);
}

void Graphics::SetViewport(UINT width, UINT height) {
  spriteConstants_.screenSize = {static_cast<float>(width),
                                 static_cast<float>(height)};
  D3D11_VIEWPORT viewport{};
  viewport.Width = static_cast<float>(width);
  viewport.Height = static_cast<float>(height);
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;
  context_->RSSetViewports(1, &viewport);
}

void Graphics::CreateRenderTarget() {
  Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
  ThrowIfFailed(swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer)),
                "Failed to get the DirectX 11 back buffer.");

  ThrowIfFailed(device_->CreateRenderTargetView(backBuffer.Get(), nullptr,
                                                &renderTargetView_),
                "Failed to create the DirectX 11 render target view.");
  CreateDebugTextRenderTarget();
}

void Graphics::ReleaseRenderTarget() {
  ReleaseDebugTextRenderTarget();
  context_->OMSetRenderTargets(0, nullptr, nullptr);
  renderTargetView_.Reset();
}

void Graphics::Resize(UINT width, UINT height) {
  if (!swapChain_ || width == 0 || height == 0) {
    return;
  }

  ReleaseRenderTarget();

  ThrowIfFailed(
      swapChain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0),
      "Failed to resize the DirectX 11 swap chain.");

  CreateRenderTarget();
  SetViewport(width, height);
}

void Graphics::ClearBackBuffer(const std::array<float, 4> &color) {
  context_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), nullptr);
  context_->ClearRenderTargetView(renderTargetView_.Get(), color.data());
}

void Graphics::BeginFrame(double gameTimeSeconds, double realTimeSeconds) {
  constexpr std::array clearColor = {0.68f, 0.70f, 0.73f, 1.0f};

  ClearBackBuffer(clearColor);
  frameConstants_.gameTimeSeconds = static_cast<float>(gameTimeSeconds);
  frameConstants_.realTimeSeconds = static_cast<float>(realTimeSeconds);
  UpdateFrameConstants();
  BindSpritePipeline();
  context_->PSSetConstantBuffers(kFrameSlot, 1,
                                 frameConstantBuffer_.GetAddressOf());
}

void Graphics::DrawSprite(const SpriteDrawData &sprite) {
  BindSpriteTexture(sprite.texture);
  BindSpriteSampler(sprite.samplerMode, sprite.addressMode);
  SetSpriteTransform(sprite.center[0], sprite.center[1], sprite.size[0],
                     sprite.size[1], sprite.rotationRadians);
  UpdateSpriteConstants();
  spriteTintConstants_.tint = sprite.tint;
  spriteTintConstants_.timeSource = static_cast<float>(sprite.timeSource);
  spriteTintConstants_.shape = static_cast<float>(sprite.shape);
  spriteTintConstants_.uvRect = sprite.uvRect;
  spriteTintConstants_.dissolveProgress = sprite.dissolveProgress;
  spriteTintConstants_.dissolveNoiseScale = sprite.dissolveNoiseScale;
  spriteTintConstants_.dissolveEdgeWidth = sprite.dissolveEdgeWidth;
  spriteTintConstants_.dissolveEdgeStrength = sprite.dissolveEdgeStrength;
  spriteTintConstants_.dissolveEdgeColor = sprite.dissolveEdgeColor;
  UpdateSpriteTintConstants();
  BindBlendState(sprite.blendMode);

  context_->DrawIndexed(6, 0, 0);
}

void Graphics::DrawDebugText(std::wstring_view text) {
  if (text.empty()) {
    return;
  }

  context_->OMSetRenderTargets(0, nullptr, nullptr);
  debugTextRenderTarget_->BeginDraw();

  constexpr D2D1_RECT_F panelRect{10.0f, 10.0f, 430.0f, 470.0f};
  debugTextBrush_->SetColor(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.72f));
  debugTextRenderTarget_->FillRectangle(panelRect, debugTextBrush_.Get());

  constexpr D2D1_RECT_F textRect{22.0f, 18.0f, 420.0f, 460.0f};
  debugTextBrush_->SetColor(D2D1::ColorF(D2D1::ColorF::White));
  debugTextRenderTarget_->DrawTextW(
      text.data(), static_cast<UINT32>(text.size()), debugTextFormat_.Get(),
      textRect, debugTextBrush_.Get());

  ThrowIfFailed(debugTextRenderTarget_->EndDraw(),
                "Failed to draw the debug overlay.");
}

void Graphics::EndFrame() {
  ThrowIfFailed(swapChain_->Present(0, 0),
                "Failed to present the DirectX 11 swap chain.");
}

void Graphics::DrawUiPanel(std::wstring_view text,
                           const std::array<float, 4> &bounds) {
  if (text.empty()) {
    return;
  }

  context_->OMSetRenderTargets(0, nullptr, nullptr);
  debugTextRenderTarget_->BeginDraw();
  const D2D1_RECT_F panelRect{bounds[0], bounds[1], bounds[2], bounds[3]};
  debugTextBrush_->SetColor(D2D1::ColorF(0.04f, 0.06f, 0.10f, 0.92f));
  debugTextRenderTarget_->FillRectangle(panelRect, debugTextBrush_.Get());
  const D2D1_RECT_F textRect{bounds[0] + 12.0f, bounds[1] + 10.0f,
                            bounds[2] - 12.0f, bounds[3] - 10.0f};
  debugTextBrush_->SetColor(D2D1::ColorF(D2D1::ColorF::White));
  debugTextRenderTarget_->DrawTextW(
      text.data(), static_cast<UINT32>(text.size()), debugTextFormat_.Get(),
      textRect, debugTextBrush_.Get());
  ThrowIfFailed(debugTextRenderTarget_->EndDraw(),
                "Failed to draw the UI panel.");
}
