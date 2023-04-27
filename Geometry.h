#pragma once

using Microsoft::WRL::ComPtr;

using namespace DirectX;

namespace Model
{
	struct Geometrie
	{
		std::string name;

		ComPtr<ID3DBlob> vertexBufferCPU;
		ComPtr<ID3DBlob> indexBufferCPU;

		ComPtr<ID3D12Resource> vertexBufferGPU;
		ComPtr<ID3D12Resource> indexBufferGPU;

		ComPtr<ID3D12Resource> vertexUploadBuffer;
		ComPtr<ID3D12Resource> indexUploadBuffer;

		float bounds[4];		// A bounding sphere
		uint32_t vbOffset;		// BufferLocation - Buffer.GpuVirtualAddress
		uint32_t vbSize;		// SizeInBytes
		uint32_t vbStride;		// StrideInBytes
		uint32_t ibOffset;		// BufferLocation - Buffer.GpuVirtualAddress
		uint32_t ibSize;		// SizeInBytes
		DXGI_FORMAT ibFormat;	// DXGI_FORMAT

		struct Draw
		{
			uint32_t indexCount;		// Number of indices = 3 * number of triangles
			uint32_t startIndex;		// Offset to first index in index buffer
			uint32_t baseVertex;		// Offset to first vertex in vertex buffer
		};

		D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const;

		D3D12_INDEX_BUFFER_VIEW IndexBufferView() const;

		void Destory()
		{
			vertexUploadBuffer = nullptr;
			indexUploadBuffer = nullptr;
		}
	};

	inline D3D12_VERTEX_BUFFER_VIEW Geometrie::VertexBufferView() const
	{
		D3D12_VERTEX_BUFFER_VIEW VBView;
		VBView.BufferLocation = vertexBufferGPU->GetGPUVirtualAddress();
		VBView.SizeInBytes = vbSize;
		VBView.StrideInBytes = vbStride;

		return VBView;
	}

	inline D3D12_INDEX_BUFFER_VIEW Geometrie::IndexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW IBView;
		IBView.BufferLocation = indexBufferGPU->GetGPUVirtualAddress();
		IBView.Format = ibFormat;
		IBView.SizeInBytes = ibSize;

		return IBView;
	}
}