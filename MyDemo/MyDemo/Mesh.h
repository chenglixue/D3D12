#pragma once

using Microsoft::WRL::ComPtr;

using namespace DirectX;

struct Mesh
{
	std::string name;

	ComPtr<ID3DBlob> vertexBufferCPU = nullptr;
	ComPtr<ID3DBlob> indexBufferCPU = nullptr;

	ComPtr<ID3D12Resource> vertexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> indexBufferGPU = nullptr;

	ComPtr<ID3D12Resource> vertexUploadBuffer = nullptr;
	ComPtr<ID3D12Resource> indexUploadBuffer = nullptr;

	float bounds[4];		// A bounding sphere
	uint32_t vbOffset = 0;		// BufferLocation - Buffer.GpuVirtualAddress
	uint32_t vbSize = 0;		// SizeInBytes
	uint32_t vbStride = 0;		// StrideInBytes
	uint32_t vbDepthOffset = 0;	// BufferLocation - Buffer.GpuVirtualAddress
	uint32_t vbDepthSize = 0;	// SizeInBytes
	uint32_t ibOffset = 0;		// BufferLocation - Buffer.GpuVirtualAddress
	uint32_t ibSize = 0;		// SizeInBytes
	DXGI_FORMAT ibFormat = DXGI_FORMAT_R16_UINT;	// DXGI_FORMAT

	struct Draw
	{
		uint32_t indexCount = 0;		// Number of indices = 3 * number of triangles
		uint32_t startIndex = 0;		// Offset to first index in index buffer
		uint32_t baseVertex = 0;		// Offset to first vertex in vertex buffer
	};

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const;

	D3D12_INDEX_BUFFER_VIEW IndexBufferView() const;

	void Destory()
	{
		vertexUploadBuffer = nullptr;
		indexUploadBuffer = nullptr;
	}
};

inline D3D12_VERTEX_BUFFER_VIEW Mesh::VertexBufferView() const
{
	D3D12_VERTEX_BUFFER_VIEW VBView;
	VBView.BufferLocation = vertexBufferGPU->GetGPUVirtualAddress();
	VBView.SizeInBytes = vbSize;
	VBView.StrideInBytes = vbStride;

	return VBView;
}

inline D3D12_INDEX_BUFFER_VIEW Mesh::IndexBufferView() const
{
	D3D12_INDEX_BUFFER_VIEW IBView;
	IBView.BufferLocation = indexBufferGPU->GetGPUVirtualAddress();
	IBView.Format = ibFormat;
	IBView.SizeInBytes = ibSize;

	return IBView;
}


class ProceduralGeometry
{
	struct Vertex
	{
		Vertex() {}
		Vertex(
			const XMFLOAT3& p,
			const XMFLOAT3& n,
			const XMFLOAT3& t,
			const XMFLOAT2& uv) :
			position(p),
			normal(n),
			tangentU(t),
			texC(uv)
		{}

		Vertex(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v) :
			position(px, py, pz),
			normal(nx, ny, nz),
			tangentU(tx, ty, tz),
			texC(u, v)
		{}

		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT3 tangentU;
		XMFLOAT2 texC;
	};

	class MeshData
	{
	public:
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices32;

		std::vector<uint16_t>& GetIndices16()
		{
			if (m_indices16.empty())
			{
				m_indices16.resize(indices32.size());
				for (size_t i = 0; i < indices32.size(); ++i)
					m_indices16[i] = static_cast<uint16_t>(indices32[i]);
			}

			return m_indices16;
		}

	private:
		std::vector<uint16_t> m_indices16;
	};

public:
	MeshData CreateGrid(float width, float depth, uint32_t m, uint32_t n);

	void CreateLand(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> cmdList, std::unordered_map<std::string, std::unique_ptr<Mesh>>& geometries, std::unordered_map<std::string, std::unique_ptr<Mesh::Draw>>& draws);
};