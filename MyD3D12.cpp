
#include "pch.h"
#include "MyD3D12.h"
#include "DDSTextureLoader.h"
#include "UploadBuffer.h"

using namespace DirectX;

MyD3D12::MyD3D12(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_frameIndex(0),
    m_fenceValue(0),
    m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
    m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
    m_rtvDescriptorSize(0),
    m_cbvSrvDescriptorSize(0),
    m_passCBVOffset(0),
    m_frameCounter(0),
    m_currentFrameResourceIndex(0),
    m_pCurrentFrameResource(nullptr)
{

}

void MyD3D12::OnInit()
{                    
    m_camera.Init({ 0, 0, 5});

    LoadPipeline();
    LoadAssets();
}

// Load the rendering pipeline dependencies.
void MyD3D12::LoadPipeline()
{
    UINT dxgiFactoryFlags = 0;


#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    ComPtr<IDXGIFactory4> factory;  //DXGI factory is used to create device
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    if (m_useWarpDevice)    //create software adapter
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));    //enum adapter

        ThrowIfFailed(D3D12CreateDevice(
            warpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,     //Specifies the highest version is D3D12
            IID_PPV_ARGS(&m_device)
        ));
    }
    else   //create hardware adapter
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter);    //acquir first available hardware adapter that supports Direct3D 12

        ThrowIfFailed(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(m_device.GetAddressOf())
        ));
    }

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
    NAME_D3D12_OBJECT(m_commandQueue);  //Associates a name with the device object. This name is for use in debug diagnostics and tools

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;  //number of swap chain buffers
    swapChainDesc.Width = m_width;  //buffer width
    swapChainDesc.Height = m_height;    //buffer hidth
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;      //Buffer format
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;    //Buffer usage
    //specify bit-block transfer(bitblt) model and specify that DXGI discard the contents of the back buffer after call
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1; //number of multisamples per pixel

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it
        Win32Application::GetHwnd(), //HWND handle that is associated with the swap chain that CreateSwapChainForHwnd creates
        &swapChainDesc,     
        nullptr,
        nullptr,
        &swapChain
        ));

    // This sample does not support fullscreen transitions.
    ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain.As(&m_swapChain));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
}

// Load the sample assets.
void MyD3D12::LoadAssets()
{
    BuildRootSignature();

    BuildShaderAndInputLayout();

    BuildPSO();
    // Create the command list.
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
    NAME_D3D12_OBJECT(m_commandList);

    BuildModel();

    LoadTexture();

    BuildMaterial();

    BuildDescriptorHeaps();

    BuildRTVDSV();

    BuildCBVSRV();

    BuildRenderer();

    BuildFrameResources();

    // Close the command list and execute it to begin the initial GPU setup.
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Create synchronization objects and wait until assets have been uploaded to the GPU
    {
        ThrowIfFailed(m_device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValue++;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if(m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        
        // Signal and increment the fence value.
        const UINT64 fenceToWaitFor = m_fenceValue;
        ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fenceToWaitFor));
        m_fenceValue++;

        // Wait until the fence is completed.
        ThrowIfFailed(m_fence->SetEventOnCompletion(fenceToWaitFor, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

// Update frame-based values.
void MyD3D12::OnUpdate()
{
    m_timer.Tick(NULL);

    if (m_frameCounter == 500)
    {
        // Update window text with FPS value.
        wchar_t fps[64];
        swprintf_s(fps, L"%ufps", m_timer.GetFramesPerSecond());
        SetCustomWindowText(fps);
        m_frameCounter = 0;
    }

    m_frameCounter++;

    // Get current GPU progress against submitted workload. Resources still scheduled 
    // for GPU execution cannot be modified or else undefined behavior will result
    const UINT64 lastCompletedFence = m_fence->GetCompletedValue();

    // Move to the next frame resource
    m_currentFrameResourceIndex = (m_currentFrameResourceIndex + 1) % FrameCount;
    m_pCurrentFrameResource = m_frameResources[m_currentFrameResourceIndex].get();

    // Make sure that this frame resource isn't still in use by the GPU.
    // If it is, wait for it to complete
    if (m_pCurrentFrameResource->fenceValue != 0 && m_pCurrentFrameResource->fenceValue > lastCompletedFence)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(m_pCurrentFrameResource->fenceValue, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    m_camera.Update(static_cast<float>(m_timer.GetElapsedSeconds()));

    m_pCurrentFrameResource->UpdateObjectConstantBuffers(m_allRenderers);

    m_pCurrentFrameResource->UpdatePassConstantBuffers(m_camera.GetViewMatrix(), m_camera.GetProjectionMatrix(1.f, m_aspectRatio), m_camera.GetPosition());

    m_pCurrentFrameResource->UpdateMaterialConstantBuffers(m_materials);
}

// Render the scene.
void MyD3D12::OnRender()
{
    PIXBeginEvent(m_commandQueue.Get(), 0, L"Render");

    // Record all the commands we need to render the scene into the command list
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    PIXEndEvent(m_commandQueue.Get());

    // Present the frame
    ThrowIfFailed(m_swapChain->Present(0, 0));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Signal and increment the fence value.
    m_pCurrentFrameResource->fenceValue = m_fenceValue;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValue));
    m_fenceValue++;
}

void MyD3D12::OnDestroy()
{
    /* 
        Ensure that the GPU is no longer referencing resources that are about to be
        cleaned up by the destructor
    */
    {
        const UINT64 fence = m_fenceValue;
        const UINT64 lastCompletedFence = m_fence->GetCompletedValue();

        // Signal and increment the fence value.
        ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValue));
        m_fenceValue++;

        // Wait until the previous frame is finished.
        if (lastCompletedFence < fence)
        {
            ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }

        m_fence.Reset();
    }
}

void MyD3D12::BuildShaderAndInputLayout()
{
    m_shaders["MainVS"] = CompileShader(GetAssetFullPath(L"MainVS.hlsl").c_str(), nullptr, "main", "vs_5_1");
    m_shaders["MainPS"] = CompileShader(GetAssetFullPath(L"MainPS.hlsl").c_str(), nullptr, "main", "ps_5_1");

    m_shaders["CubemapVS"] = CompileShader(GetAssetFullPath(L"CubemapVS.hlsl").c_str(), nullptr, "main", "vs_5_1");
    m_shaders["CubemapPS"] = CompileShader(GetAssetFullPath(L"CubemapPS.hlsl").c_str(), nullptr, "main", "ps_5_1");

    m_shaders["Outline_VS"] = CompileShader(GetAssetFullPath(L"Outline_VS.hlsl").c_str(), nullptr, "main", "vs_5_1");
    m_shaders["Outline_PS"] = CompileShader(GetAssetFullPath(L"Outline_PS.hlsl").c_str(), nullptr, "main", "ps_5_1");

    m_inputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

void MyD3D12::BuildRootSignature()
{
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

    CD3DX12_DESCRIPTOR_RANGE ranges[2];
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 0);  // register t0, t1, t2
    ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);

    CD3DX12_ROOT_PARAMETER rootParameters[5];
    rootParameters[0].InitAsConstantBufferView(0);  // register b0
    rootParameters[1].InitAsConstantBufferView(1);  // register b1
    rootParameters[2].InitAsConstantBufferView(2);  // register b2
    rootParameters[3].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[4].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = Core::GetStaticSamplers();

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(_countof(rootParameters), rootParameters, staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature = nullptr;
    ComPtr<ID3DBlob> error = nullptr;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    NAME_D3D12_OBJECT(m_rootSignature);
}

void MyD3D12::BuildPSO()
{
    //
    // draw cubemap
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC cubemapPSODesc = {};

    cubemapPSODesc.InputLayout = { m_inputLayout.data(), (uint32_t)m_inputLayout.size() };
    cubemapPSODesc.pRootSignature = m_rootSignature.Get();
    cubemapPSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    cubemapPSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    cubemapPSODesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    cubemapPSODesc.SampleMask = UINT_MAX;  //set the sampling for each pixel(Multiple sampling takes up to 32 samples)
    cubemapPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    cubemapPSODesc.NumRenderTargets = 1;   //The number of render target formats in the RTVFormats member
    cubemapPSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    cubemapPSODesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    cubemapPSODesc.SampleDesc.Count = 1;

    // forbide back culling 
    cubemapPSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    // let cubemap z = 1 pass z-test, otherwise it'll be failed in z-test because data of zbuffer is 1
    cubemapPSODesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    cubemapPSODesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    cubemapPSODesc.VS =
    {
        reinterpret_cast<BYTE*>(m_shaders["CubemapVS"]->GetBufferPointer()),
        m_shaders["CubemapVS"]->GetBufferSize()
    };
    cubemapPSODesc.PS =
    {
        reinterpret_cast<BYTE*>(m_shaders["CubemapPS"]->GetBufferPointer()),
        m_shaders["CubemapPS"]->GetBufferSize()
    };

    ThrowIfFailed(m_device->CreateGraphicsPipelineState(&cubemapPSODesc, IID_PPV_ARGS(&m_PSOs["cubemap"])));

    NAME_D3D12_OBJECT(m_PSOs["cubemap"]);


    // 
    // Describe and create the graphics pipeline state object (PSO).
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePSODesc = cubemapPSODesc;

    opaquePSODesc.VS =
    {
        reinterpret_cast<BYTE*>(m_shaders["MainVS"]->GetBufferPointer()),
        m_shaders["MainVS"]->GetBufferSize()
    };
    opaquePSODesc.PS =
    {
        reinterpret_cast<BYTE*>(m_shaders["MainPS"]->GetBufferPointer()),
        m_shaders["MainPS"]->GetBufferSize()
    };
    opaquePSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    opaquePSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    /*
    D3D12_RENDER_TARGET_BLEND_DESC opaqueBlendDesc = {};
    opaqueBlendDesc.BlendEnable = TRUE;
    opaqueBlendDesc.LogicOpEnable = FALSE;
    opaqueBlendDesc.SrcBlend = D3D12_BLEND_SRC_COLOR;
    opaqueBlendDesc.DestBlend = D3D12_BLEND_DEST_COLOR;
    opaqueBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
    opaqueBlendDesc.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
    opaqueBlendDesc.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
    opaqueBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    opaqueBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
    opaqueBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    opaquePSODesc.BlendState.RenderTarget[0] = opaqueBlendDesc;
    */

    {
        // set depth & stencil test
        D3D12_DEPTH_STENCIL_DESC outlineDesc;
        outlineDesc.DepthEnable = TRUE; // enable depth test
        outlineDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;    // Turn off writes to the depth-stencil buffer
        outlineDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS; // if less pass the depth test

        outlineDesc.StencilEnable = TRUE;
        outlineDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;   // no mask
        outlineDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK; // no mask  

        outlineDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS; // pass stencil test
        outlineDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;      // no change
        outlineDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP; // no change
        outlineDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;   // use stencil reference to replace data of stencil buffer

        // default
        outlineDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        outlineDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        outlineDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        outlineDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;

        opaquePSODesc.DepthStencilState = outlineDesc;
    }

    ThrowIfFailed(m_device->CreateGraphicsPipelineState(&opaquePSODesc, IID_PPV_ARGS(&m_PSOs["opaque"])));

    NAME_D3D12_OBJECT(m_PSOs["opaque"]);

    //
    // draw outline of model
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC outlinePSODesc = opaquePSODesc;

    D3D12_DEPTH_STENCIL_DESC outlineDesc;
    outlineDesc.DepthEnable = FALSE; // forbie depth test
    outlineDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;    // Turn on writes to the depth-stencil buffer
    outlineDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS; // if less pass the depth test

    outlineDesc.StencilEnable = TRUE;
    outlineDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;   // no mask
    outlineDesc.StencilWriteMask = 0x00; // forbie stencil write

    outlineDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL; // pass stencil test
    outlineDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;      // no change
    outlineDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP; // no change
    outlineDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;   // no change

    // default
    outlineDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    outlineDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    outlineDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    outlineDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;

    outlinePSODesc.VS =
    {
        reinterpret_cast<BYTE*>(m_shaders["Outline_VS"]->GetBufferPointer()),
        m_shaders["Outline_VS"]->GetBufferSize()
    };

    outlinePSODesc.PS =
    {
        reinterpret_cast<BYTE*>(m_shaders["Outline_PS"]->GetBufferPointer()),
        m_shaders["Outline_PS"]->GetBufferSize()
    };

    ThrowIfFailed(m_device->CreateGraphicsPipelineState(&outlinePSODesc, IID_PPV_ARGS(&m_PSOs["Outline"])));

    NAME_D3D12_OBJECT(m_PSOs["Outline"]);
}

void MyD3D12::BuildModel()
{
    m_models.push_back(std::make_unique<Model::ModelLoader>("ModelFile/diablo3_pose/diablo3_pose.obj"));
    assert(m_models.size() != 0);

    Model::GeometryGenerator geoGen;
    Model::GeometryGenerator::MeshData box = geoGen.CreateBox(1.f, 1.f, 1.f, 5);
    
    std::vector<Model::Vertex> totalVertex;
    std::vector<uint32_t> totalIndex;

    for (size_t i = 0; i < m_models.size(); ++i)
    {
        auto& pCurrModel = m_models[i];

        auto& modelVertices = pCurrModel->GetVertices();
        auto& modelIndices = pCurrModel->GetIndices();
        assert(modelVertices.size() != 0);
        assert(modelIndices.size() != 0);

        auto pModelDraw = std::make_unique<Model::Geometrie::Draw>();
        pModelDraw->baseVertex = (UINT)totalVertex.size();
        pModelDraw->startIndex = (UINT)totalIndex.size();
        pModelDraw->indexCount = (UINT)modelIndices.size();

        m_draws.push_back(std::move(pModelDraw));

        totalVertex.insert(totalVertex.end(), modelVertices.begin(), modelVertices.end());
        totalIndex.insert(totalIndex.end(), modelIndices.begin(), modelIndices.end());
    }

    // GeometryGenerator'draw
    {
        auto pDraw = std::make_unique<Model::Geometrie::Draw>();

        pDraw->baseVertex = (UINT)totalVertex.size();
        pDraw->startIndex = (UINT)totalIndex.size();
        pDraw->indexCount = (UINT)box.Indices32.size();

        m_draws.push_back(std::move(pDraw));

        totalVertex.insert(totalVertex.end(), box.Vertices.begin(), box.Vertices.end());
        totalIndex.insert(totalIndex.end(), box.Indices32.begin(), box.Indices32.end());
    }

    const uint32_t vbSize = (uint32_t)totalVertex.size() * sizeof(Model::Vertex);
    const uint32_t ibSize = (uint32_t)totalIndex.size() * sizeof(uint32_t);

    auto pGeo = std::make_unique<Model::Geometrie>();
    pGeo->name = "Geometrie";
    pGeo->vbSize = vbSize;
    pGeo->ibSize = ibSize;
    pGeo->vbStride = sizeof(Model::Vertex);
    pGeo->ibFormat = DXGI_FORMAT_R32_UINT;
    pGeo->ibOffset = 0;
    pGeo->vbOffset = 0;
    
    ThrowIfFailed(D3DCreateBlob(vbSize, &pGeo->vertexBufferCPU));
    CopyMemory(pGeo->vertexBufferCPU->GetBufferPointer(), totalVertex.data(), vbSize);

    ThrowIfFailed(D3DCreateBlob(ibSize, &pGeo->indexBufferCPU));
    CopyMemory(pGeo->indexBufferCPU->GetBufferPointer(), totalIndex.data(), ibSize);

    pGeo->vertexBufferGPU = Core::CreateDefaultBuffer(m_device.Get(), m_commandList.Get(), totalVertex.data(), vbSize, pGeo->vertexUploadBuffer);

    pGeo->indexBufferGPU = Core::CreateDefaultBuffer(m_device.Get(), m_commandList.Get(), totalIndex.data(), ibSize, pGeo->indexUploadBuffer);

    m_geometries.push_back(std::move(pGeo));

    assert(m_geometries.size() != 0);
}

void MyD3D12::LoadTexture()
{
    // diffuse dds
    for (size_t i = 0; i < m_models.size(); ++i)
    {
        auto pTexture = std::make_unique<Core::Texture>();

        auto textureName = m_models[i]->GetTextureName();
        auto textureFileName = m_models[i]->GetDirectory() + textureName + "_diffuse.dds";
        OutputDebugStringA((textureName + "_diffuse" + '\n').c_str());
        OutputDebugStringA((textureFileName + '\n').c_str());

        pTexture->name = textureName;
        pTexture->fileName = Util::UTF8ToWideString(textureFileName);

        ThrowIfFailed(LoadDDSTextureFromFile(
            m_device.Get(),
            pTexture->fileName.c_str(),
            &pTexture->defaultTexture,
            pTexture->ddsData,
            pTexture->subResources
        ));

        Core::CreateD3DResource(m_device.Get(), m_commandList.Get(), pTexture->subResources, pTexture->defaultTexture, pTexture->uploadTexture);

        m_diffuseTextures.push_back(std::move(pTexture));
    }

    // specular dds
    for (size_t i = 0; i < m_models.size(); ++i)
    {
        auto pTexture = std::make_unique<Core::Texture>();

        auto textureName = m_models[i]->GetTextureName();
        auto textureFileName = m_models[i]->GetDirectory() + textureName + "_specular.dds";
        OutputDebugStringA((textureName + "_specular" + '\n').c_str());
        OutputDebugStringA((textureFileName + '\n').c_str());

        pTexture->name = textureName;
        pTexture->fileName = Util::UTF8ToWideString(textureFileName);

        ThrowIfFailed(LoadDDSTextureFromFile(
            m_device.Get(),
            pTexture->fileName.c_str(),
            &pTexture->defaultTexture,
            pTexture->ddsData,
            pTexture->subResources
        ));

        Core::CreateD3DResource(m_device.Get(), m_commandList.Get(), pTexture->subResources, pTexture->defaultTexture, pTexture->uploadTexture);

        m_specularTextures.push_back(std::move(pTexture));
    }

    // normalmap dds
    for (size_t i = 0; i < m_models.size(); ++i)
    {
        auto pTexture = std::make_unique<Core::Texture>();

        auto textureName = m_models[i]->GetTextureName();
        auto textureFileName = m_models[i]->GetDirectory() + textureName + "_normal.dds";
        OutputDebugStringA((textureName + "_normal" + '\n').c_str());
        OutputDebugStringA((textureFileName + '\n').c_str());

        pTexture->name = textureName;
        pTexture->fileName = Util::UTF8ToWideString(textureFileName);

        ThrowIfFailed(LoadDDSTextureFromFile(
            m_device.Get(),
            pTexture->fileName.c_str(),
            &pTexture->defaultTexture,
            pTexture->ddsData,
            pTexture->subResources
        ));

        Core::CreateD3DResource(m_device.Get(), m_commandList.Get(), pTexture->subResources, pTexture->defaultTexture, pTexture->uploadTexture);

        m_normalmapTextures.push_back(std::move(pTexture));
    }

    assert(m_normalmapTextures.size() != 0);

    // sky box
    {
        auto pSkybox = std::make_unique<Core::Texture>();

        pSkybox->name = "skybox";
        pSkybox->fileName = Util::UTF8ToWideString("ModelFile/skybox/skybox.dds");
        OutputDebugStringA("ModelFile/skybox2/skybox.dds");

        ThrowIfFailed(LoadDDSTextureFromFile(
            m_device.Get(),
            pSkybox->fileName.c_str(),
            &pSkybox->defaultTexture,
            pSkybox->ddsData,
            pSkybox->subResources
        ));

        Core::CreateD3DResource(m_device.Get(), m_commandList.Get(), pSkybox->subResources, pSkybox->defaultTexture, pSkybox->uploadTexture);

        m_cubemaps[pSkybox->name] = std::move(pSkybox);
    }
}

void MyD3D12::BuildMaterial()
{
    // model's material
    {
        auto pMaterial = std::make_unique<Core::Material>();

        std::string temp = m_models[0]->GetTextureName();
        pMaterial->name = temp.substr(0, temp.find_last_of('_'));
        pMaterial->materialCBIndex = 0;
        pMaterial->diffuseSrvHeapIndex = 0;
        pMaterial->specularSrvHeapIndex = 1;
        pMaterial->normalmapSrvHeapIndex = 2;
        pMaterial->numFramesDirty = FrameCount;
        pMaterial->specualrShiness = 64;
        pMaterial->ambientAlbedo = 0.1;

        m_materials.push_back(std::move(pMaterial));
    }

    // cubemap's material
    {
        auto pMaterial = std::make_unique<Core::Material>();

        pMaterial->name = "skybox";
        pMaterial->materialCBIndex = 1;
        pMaterial->diffuseSrvHeapIndex = 3;
        pMaterial->numFramesDirty = FrameCount;
        pMaterial->specualrShiness = 64;
        pMaterial->ambientAlbedo = 0.1;

        m_materials.push_back(std::move(pMaterial));
    }

    assert(m_materials.size() != 0);
}

void MyD3D12::BuildRenderer()
{
    size_t i = 0;
    for (; i < m_models.size(); ++i)
    {
        auto pRenderer = std::make_unique<Core::Renderer>();

        XMStoreFloat4x4(&pRenderer->world, DirectX::XMMatrixRotationY(MathHelper::Pi));
        pRenderer->objectIndex = i;
        pRenderer->numFramesDirty = FrameCount;
        pRenderer->pGeometry = m_geometries[0].get();
        pRenderer->pMaterail = m_materials[i].get();
        pRenderer->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        pRenderer->indexCount = m_draws[i]->indexCount;
        pRenderer->startIndex = m_draws[i]->startIndex;
        pRenderer->baseVertex = m_draws[i]->baseVertex;

        m_renderLayers[(int)Core::RenderLayer::Opaque].push_back(pRenderer.get());
        m_allRenderers.push_back(std::move(pRenderer));
    }
    
    {
        auto pRenderer = std::make_unique<Core::Renderer>();

        XMStoreFloat4x4(&pRenderer->world, DirectX::XMMatrixScaling(5000.f, 5000.f, 5000.f));
        pRenderer->objectIndex = i;
        pRenderer->numFramesDirty = FrameCount;
        pRenderer->pGeometry = m_geometries[0].get();
        pRenderer->pMaterail = m_materials[i].get();
        pRenderer->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        pRenderer->indexCount = m_draws[i]->indexCount;
        pRenderer->startIndex = m_draws[i]->startIndex;
        pRenderer->baseVertex = m_draws[i]->baseVertex;

        m_renderLayers[(int)Core::RenderLayer::Sky].push_back(pRenderer.get());
        m_allRenderers.push_back(std::move(pRenderer));
    }

    assert(m_allRenderers.size() != 0);
    assert(m_renderLayers[(int)Core::RenderLayer::Sky].size() != 0);
}

void MyD3D12::BuildFrameResources()
{
    for (UINT i = 0; i < FrameCount; ++i)
    {
        m_frameResources.push_back(std::make_unique<Core::FrameResource>(m_device.Get(), 1, m_allRenderers.size(), m_materials.size()));
    }

    assert(m_frameResources.size() != 0);
}

void MyD3D12::BuildDescriptorHeaps()
{
    // create a render target view (RTV) descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

    // Describe and create a depth stencil view (DSV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

    // create a const buffer view(CBV) descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC cbvSrvHeapDesc = {};
    cbvSrvHeapDesc.NumDescriptors = m_diffuseTextures.size() + m_specularTextures.size() + m_normalmapTextures.size() + m_cubemaps.size();
    cbvSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(m_device->CreateDescriptorHeap(&cbvSrvHeapDesc, IID_PPV_ARGS(&m_cbvSrvHeap)));
    NAME_D3D12_OBJECT(m_cbvSrvHeap);

    // Once we create the RTV descriptor heap, we need to get the size of the RTV descriptor type size on the GPU
    // There is no guarentee that a descriptor type on one GPU is the same size as a descriptor on another GPU
    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_cbvSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void MyD3D12::BuildRTVDSV()
{
    // Create rtv 
    {
        // get a handle to the first descriptor in the descriptor heap
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame
        for (UINT n = 0; n < FrameCount; n++)
        {
            // get a pointer to the buffer in the swap chain
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            // create a descriptor that points to the resource and store it in a descriptor handle
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            // increment the rtv handle by the rtv descriptor size we got above
            rtvHandle.Offset(1, m_rtvDescriptorSize);

            NAME_D3D12_OBJECT_INDEXED(m_renderTargets, n);
        }
    }

    //Create dsv
    {

        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D24_UNORM_S8_UINT, m_width, m_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE),
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D24_UNORM_S8_UINT, 1.0f, 0), // Performance tip: Tell the runtime at resource creation the desired clear value.
            IID_PPV_ARGS(&m_depthStencil)
        ));

        NAME_D3D12_OBJECT(m_depthStencil);

        D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
        depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;
        depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

        m_device->CreateDepthStencilView(m_depthStencil.Get(), &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
    }
}

void MyD3D12::BuildCBVSRV()
{
    // create srv descriptor
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE hSRVDescriptor(m_cbvSrvHeap->GetCPUDescriptorHandleForHeapStart());

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MostDetailedMip = 0;  // the index of the most detailed mipmap level to use
        srvDesc.Texture2D.ResourceMinLODClamp = 0.f;    // min mipmap level that you can access. 0.0f means access all of mipmap levels

        for (size_t i = 0; i < m_models.size(); ++i)
        {
            // diffuse texture
            {
                auto pCurrTexture = m_diffuseTextures[i]->defaultTexture;

                srvDesc.Format = pCurrTexture->GetDesc().Format;
                srvDesc.Texture2D.MipLevels = pCurrTexture->GetDesc().MipLevels;   // combine with MostDetailedMip

                m_device->CreateShaderResourceView(pCurrTexture.Get(), &srvDesc, hSRVDescriptor);

                hSRVDescriptor.Offset(1, m_cbvSrvDescriptorSize);
            }

            // specular texture
            {
                auto pCurrTexture = m_specularTextures[i]->defaultTexture;

                srvDesc.Format = pCurrTexture->GetDesc().Format;
                srvDesc.Texture2D.MipLevels = pCurrTexture->GetDesc().MipLevels;   // combine with MostDetailedMip

                m_device->CreateShaderResourceView(pCurrTexture.Get(), &srvDesc, hSRVDescriptor);

                hSRVDescriptor.Offset(1, m_cbvSrvDescriptorSize);
            }

            // normalmap texture
            {
                auto pCurrTexture = m_normalmapTextures[i]->defaultTexture;

                srvDesc.Format = pCurrTexture->GetDesc().Format;
                srvDesc.Texture2D.MipLevels = pCurrTexture->GetDesc().MipLevels;   // combine with MostDetailedMip

                m_device->CreateShaderResourceView(pCurrTexture.Get(), &srvDesc, hSRVDescriptor);

                hSRVDescriptor.Offset(1, m_cbvSrvDescriptorSize);
            }
        }

        // cubemap texture
        {
            auto pCurrTexture = m_cubemaps["skybox"]->defaultTexture;

            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
            srvDesc.TextureCube.MostDetailedMip = 0;
            srvDesc.TextureCube.MipLevels = pCurrTexture->GetDesc().MipLevels;
            srvDesc.Format = pCurrTexture->GetDesc().Format;

            m_device->CreateShaderResourceView(pCurrTexture.Get(), &srvDesc, hSRVDescriptor);

            hSRVDescriptor.Offset(1, m_cbvSrvDescriptorSize);
        }
    }
}

void MyD3D12::OnKeyUp(UINT8 key)
{
    m_camera.OnKeyUp(key);
}

void MyD3D12::OnKeyDown(UINT8 key)
{
    m_camera.OnKeyDown(key);
}

void MyD3D12::PopulateCommandList()
{
    /*
        Command list allocators can only be reset when the associated
        command lists have finished execution on the GPU
        apps should use fences to determine GPU execution progress
    */
    ThrowIfFailed(m_pCurrentFrameResource->commandAllocator->Reset());

    /* 
        when ExecuteCommandList() is called on a particular command list 
        command list can then be reset at any time and must be before re-recording
    */
    ThrowIfFailed(m_commandList->Reset(m_pCurrentFrameResource->commandAllocator.Get(), m_PSOs["cubemap"].Get()));

    // Set necessary state
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    ID3D12DescriptorHeap* ppHeaps[] = { m_cbvSrvHeap.Get() };
    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
    m_commandList->OMSetRenderTargets(1, &rtvHandle, TRUE, &dsvHandle);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);

    CD3DX12_GPU_DESCRIPTOR_HANDLE cubemapTextureDesc(m_cbvSrvHeap->GetGPUDescriptorHandleForHeapStart());
    cubemapTextureDesc.Offset(m_diffuseTextures.size() + m_specularTextures.size() + m_normalmapTextures.size(), m_cbvSrvDescriptorSize);
    m_commandList->SetGraphicsRootDescriptorTable(4, cubemapTextureDesc);

    PIXBeginEvent(m_commandList.Get(), 0, L"Populate FrameResource");

    m_pCurrentFrameResource->PopulateCommandList(m_commandList.Get(), m_renderLayers[(int)Core::RenderLayer::Sky], m_cbvSrvHeap.Get(), m_passCBVOffset, m_frameIndex, m_cbvSrvDescriptorSize);

    m_commandList->OMSetStencilRef(1);
    m_commandList->SetPipelineState(m_PSOs["opaque"].Get());
    m_pCurrentFrameResource->PopulateCommandList(m_commandList.Get(), m_renderLayers[(int)Core::RenderLayer::Opaque], m_cbvSrvHeap.Get(), m_passCBVOffset, m_frameIndex, m_cbvSrvDescriptorSize);

    /*
    m_commandList->SetPipelineState(m_PSOs["Outline"].Get());
    m_pCurrentFrameResource->PopulateCommandList(m_commandList.Get(), m_renderLayers[(int)Core::RenderLayer::Opaque], m_cbvSrvHeap.Get(), m_passCBVOffset, m_frameIndex, m_cbvSrvDescriptorSize);
    */
    PIXEndEvent(m_commandList.Get());

    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());
}
