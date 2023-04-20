#pragma once

using Microsoft::WRL::ComPtr;

// GPUBuffer helper function 

template<typename T>
class UploadBuffer
{
public:
    UploadBuffer(ID3D12Device* device, uint32_t elementCount, bool isConstantBuffer = false) :
        m_mappedData(nullptr),
        m_elementByteSize(0),
        m_isConstantBuffer(isConstantBuffer)
    {
        m_elementByteSize = sizeof(T);

        /* 
            Constant buffer elements need to be multiples of 256 bytes. 
            typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC 
            {
                UINT64 OffsetInBytes; // multiple of 256
                UINT   SizeInBytes;   // multiple of 256
            } D3D12_CONSTANT_BUFFER_VIEW_DESC;
        */
        if (m_isConstantBuffer)
        {
            m_elementByteSize = CalcConstantBufferByteSize(sizeof(T));
        }

        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(m_elementByteSize * elementCount),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_uploadBuffer)
        ));

        ThrowIfFailed(m_uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData)));
    }

    ~UploadBuffer()
    {
        if (m_uploadBuffer != nullptr)
        {
            m_uploadBuffer->Unmap(0, nullptr);
        }

        m_mappedData = nullptr;
    }

    ID3D12Resource* Resource() const
    {
        return m_uploadBuffer.Get();
    }

    void CopyData(int elementIndex, const T& data)
    {
        memcpy(&m_mappedData[elementIndex * m_elementByteSize], &data, sizeof(T));
    }


private:
    ComPtr<ID3D12Resource> m_uploadBuffer;
    BYTE* m_mappedData;
    uint32_t m_elementByteSize;
    bool m_isConstantBuffer;
};


inline uint32_t CalcConstantBufferByteSize(uint32_t byteSize)
{
    // Constant buffers must be a multiple of the minimum hardware
    // allocation size (usually 256 bytes).  So round up to nearest
    // multiple of 256.  We do this by adding 255 and then masking off
    // the lower 2 bytes which store all bits < 256.
    // Example: Suppose byteSize = 300.
    // (300 + 255) & ~255
    // 555 & ~255
    // 0x022B & ~0x00ff
    // 0x022B & 0xff00
    // 0x0200
    // 512
    return (byteSize + 255) & ~255;
}

ComPtr<ID3D12Resource> CreateDefaultBuffer(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* cmdList,
    const void* initData,
    uint64_t byteSize,
    ComPtr<ID3D12Resource>& uploadBuffer
);



