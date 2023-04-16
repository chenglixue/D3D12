#include "pch.h"
#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* pDevice, uint32_t passCount, uint32_t objectCount) :
    fenceValue(0)
{

    // The command allocator is used by the main sample class when 
    // resetting the command list in the main update loop. Each frame 
    // resource needs a command allocator because command allocators 
    // cannot be reused until the GPU is done executing the commands 
    // associated with it.
    ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
    
    objectUploadCB = std::make_unique<UploadBuffer<ObjectConstantBuffer>> (pDevice, objectCount, true);
    passUploadCB = std::make_unique<UploadBuffer<PassConstantBuffer>>(pDevice, passCount, true);
}

FrameResource::~FrameResource()
{

}

void FrameResource::PopulateCommandList(
    ID3D12GraphicsCommandList* pCommandList,
    std::vector<Renderer*>& renderers)
{
    auto currPassUploadCB = this->passUploadCB->Resource();
    pCommandList->SetGraphicsRootConstantBufferView(1, currPassUploadCB->GetGPUVirtualAddress());

    auto objectCBSize = CalcConstantBufferByteSize(sizeof(ObjectConstantBuffer));

    auto currObjectUploadCB = this->objectUploadCB->Resource();

    for (size_t i = 0; i < renderers.size(); ++i)
    {
        auto currRenderer = renderers[i];

        // if use descriptor table
        //ID3D12DescriptorHeap* ppHeaps[] = { pCbvSrvDescriptorHeap };
        //pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

        pCommandList->IASetPrimitiveTopology(currRenderer->PrimitiveType);
        pCommandList->IASetVertexBuffers(0, 1, &currRenderer->Geo->VertexBufferView());
        pCommandList->IASetIndexBuffer(&currRenderer->Geo->IndexBufferView()); 

        D3D12_GPU_VIRTUAL_ADDRESS objectUploadAddress = currObjectUploadCB->GetGPUVirtualAddress();
        objectUploadAddress += currRenderer->objectIndex * objectCBSize;

        pCommandList->SetGraphicsRootConstantBufferView(0, objectUploadAddress);

        pCommandList->DrawIndexedInstanced(currRenderer->indexCount, 1, currRenderer->startIndex, currRenderer->baseVertex, 0);
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

void XM_CALLCONV FrameResource::UpdatePassConstantBuffers(XMMATRIX& view, XMMATRIX& projection)
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
    
    auto currentPassCB = this->passUploadCB.get();
    currentPassCB->CopyData(0, passConstantBuffer);
}
