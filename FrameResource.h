#pragma once

#include "DXSampleHelper.h"
#include "UploadBuffer.h"
#include "Renderer.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct ObjectConstantBuffer
{
    XMFLOAT4X4 world;
};

struct PassConstantBuffer
{
    XMFLOAT4X4 view;
    XMFLOAT4X4 inverseView;
    XMFLOAT4X4 projection;
    XMFLOAT4X4 inverseProjection;
    XMFLOAT4X4 viewProjection;
    XMFLOAT4X4 inverseViewProjection;
};

struct FrameResource
{
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    std::unique_ptr<UploadBuffer<ObjectConstantBuffer>> objectUploadCB;
    std::unique_ptr<UploadBuffer<PassConstantBuffer>> passUploadCB;
    UINT64 fenceValue;

    FrameResource(ID3D12Device* pDevice, uint32_t passCount, uint32_t objectCount);
    ~FrameResource();

    void PopulateCommandList(ID3D12GraphicsCommandList* pCommandList, std::vector<Renderer*>& renderers, ID3D12DescriptorHeap* pCBVDescriptorHeap,
        UINT passCBVOffset, UINT frameResourceIndex, UINT CBVDescriptorSize);

    void XM_CALLCONV UpdateObjectConstantBuffers(std::vector<std::unique_ptr<Renderer>>& allRenderers);
    void XM_CALLCONV UpdatePassConstantBuffers(XMMATRIX& view, XMMATRIX& projection);
};
