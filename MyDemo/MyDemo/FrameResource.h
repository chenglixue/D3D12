#pragma once

#include "DXSampleHelper.h"
#include "GPUBuffer.h"
#include "Renderer.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct ObjectConstantBuffer
{
    XMFLOAT4X4 world = MathHelper::Identity4x4();
};

struct PassConstantBuffer
{
    XMFLOAT4X4 view = MathHelper::Identity4x4();
    XMFLOAT4X4 inverseView = MathHelper::Identity4x4();
    XMFLOAT4X4 projection = MathHelper::Identity4x4();
    XMFLOAT4X4 inverseProjection = MathHelper::Identity4x4();
    XMFLOAT4X4 viewProjection = MathHelper::Identity4x4();
    XMFLOAT4X4 inverseViewProjection = MathHelper::Identity4x4();
};

struct InstanceVertex
{
    XMFLOAT3 pos;
    XMFLOAT4 color;
};

struct FrameResource
{
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    std::unique_ptr<UploadBuffer<ObjectConstantBuffer>> objectUploadCB;
    std::unique_ptr<UploadBuffer<PassConstantBuffer>> passUploadCB;
    UINT64 fenceValue;

    FrameResource(ID3D12Device* pDevice, uint32_t passCount, uint32_t objectCount);
    ~FrameResource();

    void PopulateCommandList(ID3D12GraphicsCommandList* pCommandList, std::vector<Renderer*>& renderers);

    void XM_CALLCONV UpdateObjectConstantBuffers(std::vector<std::unique_ptr<Renderer>>& allRenderers);
    void XM_CALLCONV UpdatePassConstantBuffers(XMMATRIX& view, XMMATRIX& projection);
};
