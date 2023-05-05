#pragma once

#include "DXSample.h"
#include "FpsCamera.h"
#include "StepTimer.h"
#include "FrameResource.h"
#include "Renderer.h"
#include "Texture.h"
#include "ModelLoad.h"
#include "Light.h"
#include "Geometry.h"

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

    virtual void OnInit() override;
    virtual void OnUpdate() override;
    virtual void OnRender() override;
    virtual void OnDestroy() override;
    virtual void OnKeyDown(UINT8 key) override;
    virtual void OnKeyUp(UINT8 key) override;

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
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    ComPtr<ID3D12DescriptorHeap> m_cbvSrvHeap;
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> m_PSOs;
    std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
    std::unordered_map<std::string, ComPtr<ID3DBlob>> m_shaders;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;

    // App resources
    UINT m_rtvDescriptorSize;
    UINT m_cbvSrvDescriptorSize;
    UINT m_passCBVOffset;
    std::vector<std::unique_ptr<Model::ModelLoader>> m_models;
    std::vector<std::unique_ptr<Core::Texture>> m_diffuseTextures;
    std::vector<std::unique_ptr<Core::Texture>> m_specularTextures;
    std::unordered_map<std::string, std::unique_ptr<Core::Texture>> m_cubemaps;
    std::vector<std::unique_ptr<Core::Material>> m_materials;
    std::vector<std::unique_ptr<Model::Geometrie>> m_geometries;
    std::vector<std::unique_ptr<Model::Geometrie::Draw>> m_draws;

    // renderer resources
    std::vector<std::unique_ptr<Core::Renderer>> m_allRenderers;
    std::vector<Core::Renderer*> m_renderLayers[(int)Core::RenderLayer::Count];

    // timer and camera
    Util::StepTimer m_timer;
    Core::FpsCamera m_camera;

    //Frame resources
    std::vector<std::unique_ptr<Core::FrameResource>> m_frameResources;
    Core::FrameResource* m_pCurrentFrameResource;
    UINT m_currentFrameResourceIndex;
     
    // Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    UINT m_frameCounter;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();

    void BuildRootSignature();
    void BuildShaderAndInputLayout();
    void BuildPSO();
    void LoadTexture();
    void BuildModel();
    void BuildMaterial();
    void BuildRenderer();
    void BuildFrameResources();
    void BuildDescriptorHeaps();
    void BuildRTVDSV();
    void BuildCBVSRV();
};
