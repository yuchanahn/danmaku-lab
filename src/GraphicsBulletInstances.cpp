#include "Graphics.h"

#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <type_traits>

std::string Graphics::GetAdapterName() const {
  Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
  Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
  ThrowIfFailed(device_.As(&dxgiDevice), "Failed to query DXGI device.");
  ThrowIfFailed(dxgiDevice->GetAdapter(&adapter), "Failed to query active adapter.");
  DXGI_ADAPTER_DESC desc{};
  ThrowIfFailed(adapter->GetDesc(&desc), "Failed to query adapter description.");
  const int bytes = WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1,
                                       nullptr, 0, nullptr, nullptr);
  if (bytes <= 0)
    throw std::runtime_error("Failed to encode adapter description.");
  std::string name(static_cast<std::size_t>(bytes), '\0');
  WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, name.data(), bytes,
                      nullptr, nullptr);
  name.pop_back();
  return name;
}

void Graphics::DrawBulletInstances(
    std::span<const BulletSpriteInstanceData> instances,
    SpriteBlendMode blendMode) {
  if (instances.empty())
    return;

  UploadBulletInstances(instances);
  BindBulletInstancePipeline(blendMode);

  context_->DrawIndexedInstanced(6, static_cast<UINT>(instances.size()), 0, 0,
                                 0);

  BindSpritePipeline();
}

void Graphics::CreateBulletInstanceBuffer() {
  static_assert(std::is_trivially_copyable_v<BulletSpriteInstanceData>);
  static_assert(std::is_standard_layout_v<BulletSpriteInstanceData>);

  D3D11_BUFFER_DESC desc{};
  desc.ByteWidth = static_cast<UINT>(sizeof(BulletSpriteInstanceData) *
                                     kBulletInstanceCapacity);
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  ThrowIfFailed(device_->CreateBuffer(&desc, nullptr, &bulletInstanceBuffer_),
                "Failed to create the bullet instance buffer.");
}

void Graphics::UploadBulletInstances(
    std::span<const BulletSpriteInstanceData> instances) {
  if (instances.empty())
    return;
  if (instances.size() > kBulletInstanceCapacity) {
    throw std::length_error("Bullet instance batch exceeds buffer capacity.");
  }

  D3D11_MAPPED_SUBRESOURCE mapped{};
  ThrowIfFailed(context_->Map(bulletInstanceBuffer_.Get(), 0,
                              D3D11_MAP_WRITE_DISCARD, 0, &mapped),
                "Failed to map the bullet instance buffer.");

  std::memcpy(mapped.pData, instances.data(),
              sizeof(BulletSpriteInstanceData) * instances.size());

  context_->Unmap(bulletInstanceBuffer_.Get(), 0);
}

void Graphics::CreateBulletInstanceInputLayout() {
  constexpr D3D11_INPUT_ELEMENT_DESC elements[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, kQuadVertexSlot,
       static_cast<UINT>(offsetof(SpriteVertex, position)),
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, kQuadVertexSlot,
       static_cast<UINT>(offsetof(SpriteVertex, uv)),
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"CENTER", 0, DXGI_FORMAT_R32G32_FLOAT, kBulletInstanceSlot,
       static_cast<UINT>(offsetof(BulletSpriteInstanceData, center)),
       D3D11_INPUT_PER_INSTANCE_DATA, 1},
      {"SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, kBulletInstanceSlot,
       static_cast<UINT>(offsetof(BulletSpriteInstanceData, size)),
       D3D11_INPUT_PER_INSTANCE_DATA, 1},
      {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, kBulletInstanceSlot,
       static_cast<UINT>(offsetof(BulletSpriteInstanceData, tint)),
       D3D11_INPUT_PER_INSTANCE_DATA, 1},
      {"ROTATION", 0, DXGI_FORMAT_R32G32_FLOAT, kBulletInstanceSlot,
       static_cast<UINT>(offsetof(BulletSpriteInstanceData, rotationXAxis)),
       D3D11_INPUT_PER_INSTANCE_DATA, 1},
      {"ROTATION", 1, DXGI_FORMAT_R32G32_FLOAT, kBulletInstanceSlot,
       static_cast<UINT>(offsetof(BulletSpriteInstanceData, rotationYAxis)),
       D3D11_INPUT_PER_INSTANCE_DATA, 1},
      {"SHAPE", 0, DXGI_FORMAT_R32_FLOAT, kBulletInstanceSlot,
       static_cast<UINT>(offsetof(BulletSpriteInstanceData, shape)),
       D3D11_INPUT_PER_INSTANCE_DATA, 1},
  };

  const auto bytecode = shaderCache_.GetBulletInstanceVertexBytecode();
  ThrowIfFailed(device_->CreateInputLayout(
                    elements, static_cast<UINT>(std::size(elements)),
                    bytecode->GetBufferPointer(), bytecode->GetBufferSize(),
                    &bulletInstanceInputLayout_),
                "Failed to create the bullet instance input layout.");
}

void Graphics::BindBulletInstancePipeline(SpriteBlendMode blendMode) {
  ID3D11Buffer *buffers[] = {vertexBuffer_.Get(), bulletInstanceBuffer_.Get()};
  constexpr UINT strides[] = {sizeof(SpriteVertex),
                              sizeof(BulletSpriteInstanceData)};
  constexpr UINT offsets[] = {0, 0};

  context_->IASetInputLayout(bulletInstanceInputLayout_.Get());
  context_->IASetVertexBuffers(kQuadVertexSlot,
                               static_cast<UINT>(std::size(buffers)), buffers,
                               strides, offsets);
  context_->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R16_UINT, 0);
  context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context_->VSSetShader(shaderCache_.GetBulletInstanceVertexShader(), nullptr,
                        0);
  context_->VSSetConstantBuffers(kFrameSlot, 1,
                                 frameConstantBuffer_.GetAddressOf());
  context_->PSSetShader(shaderCache_.GetBulletInstancePixelShader(), nullptr,
                        0);
  BindSpriteTexture(SpriteTextureId::White);
  BindSpriteSampler(SpriteSamplerMode::Point, SpriteAddressMode::Clamp);
  BindBlendState(blendMode);
}
