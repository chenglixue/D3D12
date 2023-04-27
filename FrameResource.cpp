#include "pch.h"
#include "FrameResource.h"
#include "UploadBuffer.h"

namespace Core
{
    FrameResource::FrameResource(ID3D12Device* pDevice, uint32_t passCount, uint32_t objectCount, uint32_t materialCount) :
        fenceValue(0)
    {

        // The command allocator is used by the main sample class when 
        // resetting the command list in the main update loop. Each frame 
        // resource needs a command allocator because command allocators 
        // cannot be reused until the GPU is done executing the commands 
        // associated with it.
        ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocator.GetAddressOf())));

        objectUploadCB = std::make_unique<UploadBuffer<ObjectConstantBuffer>>(pDevice, objectCount, true);
        passUploadCB = std::make_unique<UploadBuffer<PassConstantBuffer>>(pDevice, passCount, true);
        materialUploadCB = std::make_unique<UploadBuffer<MaterialConstantBuffer>>(pDevice, materialCount, true);
    }

    FrameResource::~FrameResource()
    {
        commandAllocator.Reset();
    }

    void FrameResource::PopulateCommandList(
        ID3D12GraphicsCommandList* pCommandList,
        std::vector<Renderer*>& renderers,
        ID3D12DescriptorHeap* pCbvSrvDescriptorHeap,
        UINT passCBVOffset,
        UINT frameResourceIndex,
        UINT CbvSrvDescriptorSize)
    {
        // populate pass constant buffer
        {
            auto passCB = this->passUploadCB->Resource();
            pCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
        }

        // populate object constant buffer and DrawIndexedInstanced
        {
            auto objectCBSize = CalcConstantBufferByteSize(sizeof(ObjectConstantBuffer));
            auto materialCBSize = CalcConstantBufferByteSize(sizeof(MaterialConstantBuffer));

            auto currObjectUploadCB = this->objectUploadCB->Resource();
            auto currMaterialUploadCB = this->materialUploadCB->Resource();

            for (size_t i = 0; i < renderers.size(); ++i)
            {
                auto currRenderer = renderers[i];

                pCommandList->IASetPrimitiveTopology(currRenderer->PrimitiveType);
                pCommandList->IASetVertexBuffers(0, 1, &currRenderer->pGeometry->VertexBufferView());
                pCommandList->IASetIndexBuffer(&currRenderer->pGeometry->IndexBufferView());

                CD3DX12_GPU_DESCRIPTOR_HANDLE hTex(pCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
                hTex.Offset(currRenderer->pMaterail->diffuseSrvHeapIndex, CbvSrvDescriptorSize);

                D3D12_GPU_VIRTUAL_ADDRESS objectAddress = currObjectUploadCB->GetGPUVirtualAddress() + currRenderer->objectIndex * objectCBSize;
                D3D12_GPU_VIRTUAL_ADDRESS materialAddress = currMaterialUploadCB->GetGPUVirtualAddress() + currRenderer->pMaterail->materialCBIndex * materialCBSize;

                pCommandList->SetGraphicsRootDescriptorTable(0, hTex);
                pCommandList->SetGraphicsRootConstantBufferView(1, objectAddress);
                pCommandList->SetGraphicsRootConstantBufferView(3, materialAddress);

                pCommandList->DrawIndexedInstanced(currRenderer->indexCount, 1, currRenderer->startIndex, currRenderer->baseVertex, 0);
            }
        }
    }

    void XM_CALLCONV FrameResource::UpdateObjectConstantBuffers(std::vector<std::unique_ptr<Renderer>>& allRenderers)
    {
        auto currObjectCB = this->objectUploadCB.get();

        for (auto& e : allRenderers)
        {
            // Only update the cbuffer data if the constants have changed.  
            // This needs to be tracked per frame resource.
            if (e->numFramesDirty > 0)
            {
                XMMATRIX world = XMLoadFloat4x4(&e->world);

                ObjectConstantBuffer objConstants;
                XMStoreFloat4x4(&objConstants.world, XMMatrixTranspose(world));

                currObjectCB->CopyData(e->objectIndex, objConstants);

                e->numFramesDirty--;
            }
        }
    }

    void XM_CALLCONV FrameResource::UpdatePassConstantBuffers(XMMATRIX& view, XMMATRIX& projection, XMFLOAT3& eyeWorldPosition)
    {
        XMMATRIX viewProjection = XMMatrixMultiply(view, projection);
        XMMATRIX inverseView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
        XMMATRIX inverseProjection = XMMatrixInverse(&XMMatrixDeterminant(projection), projection);
        XMMATRIX inverseViewProjection = XMMatrixInverse(&XMMatrixDeterminant(viewProjection), viewProjection);

        PassConstantBuffer passConstantBuffer;
        XMStoreFloat4x4(&passConstantBuffer.view, XMMatrixTranspose(view));
        XMStoreFloat4x4(&passConstantBuffer.projection, XMMatrixTranspose(projection));
        XMStoreFloat4x4(&passConstantBuffer.viewProjection, XMMatrixTranspose(viewProjection));
        XMStoreFloat4x4(&passConstantBuffer.inverseView, XMMatrixTranspose(inverseView));
        XMStoreFloat4x4(&passConstantBuffer.inverseProjection, XMMatrixTranspose(inverseProjection));
        XMStoreFloat4x4(&passConstantBuffer.inverseViewProjection, XMMatrixTranspose(inverseViewProjection));

        passConstantBuffer.eyeWorldPosition = eyeWorldPosition;

        // direction light
        passConstantBuffer.lights[0].lightPosition = { 0.f, 0.f, 0.f };
        passConstantBuffer.lights[0].lightDirection = { 0.f, 0.f, -1.f };
        passConstantBuffer.lights[0].lightColor = { 1.f, 1.f, 1.f };
        passConstantBuffer.lights[0].falloffStart = 1.f;
        passConstantBuffer.lights[0].falloffEnd = 20.f;
        passConstantBuffer.lights[0].spotPower = 2.f;

        // point light
        passConstantBuffer.lights[1].lightPosition = { 0.f, 0.f, -2.f };
        passConstantBuffer.lights[1].lightDirection = { 0.f, 0.f, 1.f };
        passConstantBuffer.lights[1].lightColor = { 1.f, 1.f, 1.f };
        passConstantBuffer.lights[1].falloffStart = 1.f;
        passConstantBuffer.lights[1].falloffEnd = 20.f;
        passConstantBuffer.lights[1].spotPower = 2.f;

        // spot light
        passConstantBuffer.lights[2].lightPosition = { 0.f, 0.f, -2.f };
        passConstantBuffer.lights[2].lightDirection = { 0.f, 0.f, 1.f };
        passConstantBuffer.lights[2].lightColor = { 1.f, 1.f, 1.f };
        passConstantBuffer.lights[2].falloffStart = 1.f;
        passConstantBuffer.lights[2].falloffEnd = 20.f;
        passConstantBuffer.lights[2].spotPower = 2.f;

        auto currentPassCB = this->passUploadCB.get();
        currentPassCB->CopyData(0, passConstantBuffer);
    }

    void XM_CALLCONV FrameResource::UpdateMaterialConstantBuffers(std::vector<std::unique_ptr<Material>>& pMaterials)
    {
        auto currMaterialCB = this->materialUploadCB.get();

        for (auto& e : pMaterials)
        {
            Material* pMaterial = e.get();

            if (pMaterial->numFramesDirty > 0)
            {
                MaterialConstantBuffer materialCB;
                
                materialCB.ambientStrength = pMaterial->ambientAlbedo;
                materialCB.specualrShiness = pMaterial->specualrShiness;

                currMaterialCB->CopyData(pMaterial->materialCBIndex, materialCB);

                pMaterial->numFramesDirty--;
            }
        }
    }
}


