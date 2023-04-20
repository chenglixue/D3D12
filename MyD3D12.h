#pragma once

#pragma comment(lib, "delayimp")

#include "DXSample.h"
#include "FpsCamera.h"
#include "StepTimer.h"
#include "FrameResource.h"
#include "Renderer.h"

using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

class MyD3D12 : public DXSample
{
public:
    friend class ModelLoader;
    MyD3D12(UINT width, UINT height, std::wstring name);

    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();
    virtual void OnKeyDown(UINT8 key);
    virtual void OnKeyUp(UINT8 key);

private:
    static const UINT FrameCount = 3;

    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12Resource> m_depthStencil;
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> m_PSOs;
    std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
    std::unordered_map<std::string, ComPtr<ID3DBlob>> m_shaders;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;

    // App resources
    UINT m_rtvDescriptorSize;
    UINT m_cbvDescriptorSize;
    UINT m_passCBVOffset;
    std::unordered_map<std::string, std::unique_ptr<Geometrie>> m_geometries;
    std::unordered_map<std::string, std::unique_ptr<Geometrie::Draw>> m_draws;
    StepTimer m_timer;
    FpsCamera m_camera;
    bool m_isWireFrame;

    //Frame resources
    std::vector<std::unique_ptr<FrameResource>> m_frameResources;
    FrameResource* m_pCurrentFrameResource;
    UINT m_currentFrameResourceIndex;

    // renderer resources
    std::vector<std::unique_ptr<Renderer>> m_allRenderers;
    std::vector<Renderer*> m_opaqueRenderers;
    std::vector<Renderer*> m_transparentRenderers;
     
    // Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    UINT m_frameCounter;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();

    void BuildDescriptorHeaps();
    void BuildRootSignature();
    void BuildShaderAndInputLayout();
    void BuildPSO();
    void BuildRTVDSV();
    void BuildModel();
    void BuildRenderer();
    void BuildCBV();
    void BuildFrameResources();
};
