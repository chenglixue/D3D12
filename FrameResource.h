#pragma once

#include "DXSampleHelper.h"
#include "UploadBuffer.h"
#include "Renderer.h"
#include "Light.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace Core
{
    struct ObjectConstantBuffer
    {
        XMFLOAT4X4 world;
    };

    struct MaterialConstantBuffer
    {
        int specualrShiness;

        float ambientStrength;

        float pad1;
        float pad2;
    };

    struct PassConstantBuffer
    {
        XMFLOAT4X4 view;
        XMFLOAT4X4 inverseView;
        XMFLOAT4X4 projection;
        XMFLOAT4X4 inverseProjection;
        XMFLOAT4X4 viewProjection;
        XMFLOAT4X4 inverseViewProjection;

        XMFLOAT3 eyeWorldPosition;

        float pad1;

        Light lights[MAX_LIGHT_COUNT];
    };

    struct FrameResource
    {
        ComPtr<ID3D12CommandAllocator> commandAllocator;
        std::unique_ptr<UploadBuffer<ObjectConstantBuffer>> objectUploadCB;
        std::unique_ptr<UploadBuffer<PassConstantBuffer>> passUploadCB;
        std::unique_ptr<UploadBuffer<MaterialConstantBuffer>> materialUploadCB;
        UINT64 fenceValue;

        FrameResource(ID3D12Device* pDevice, uint32_t passCount, uint32_t objectCount, uint32_t materialCount);
        ~FrameResource();

        void PopulateCommandList(ID3D12GraphicsCommandList* pCommandList, std::vector<Renderer*>& renderers, ID3D12DescriptorHeap* pCbvSrvDescriptorHeap,
            UINT passCBVOffset, UINT frameResourceIndex, UINT CbvSrvDescriptorSize);

        void XM_CALLCONV UpdateObjectConstantBuffers(std::vector<std::unique_ptr<Renderer>>& allRenderers);
        void XM_CALLCONV UpdatePassConstantBuffers(XMMATRIX& view, XMMATRIX& projection, XMFLOAT3& eyeWorldPosition);
        void XM_CALLCONV UpdateMaterialConstantBuffers(std::vector<std::unique_ptr<Material>>& materials);
    };
}
