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
			tangent(t),
			texCoord(uv) {}
		Vertex(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v) :
			position(px, py, pz),
			normal(nx, ny, nz),
			tangent(tx, ty, tz),
			texCoord(u, v) {}

		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT2 texCoord;
		XMFLOAT3 tangent;
	};

	struct Mesh
	{
		Mesh() = default;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		Mesh(std::vector<Vertex>& vertices, std::vector<UINT>& indices)
		{
			this->vertices = vertices;
			this->indices = indices;
		}
	};

	class GeometryGenerator
	{
	public:

		using uint16 = std::uint16_t;
		using uint32 = std::uint32_t;

		struct MeshData
		{
			std::vector<Vertex> Vertices;
			std::vector<uint32> Indices32;

			std::vector<uint16>& GetIndices16()
			{
				if (mIndices16.empty())
				{
					mIndices16.resize(Indices32.size());
					for (size_t i = 0; i < Indices32.size(); ++i)
						mIndices16[i] = static_cast<uint16>(Indices32[i]);
				}

				return mIndices16;
			}

		private:
			std::vector<uint16> mIndices16;
		};

		MeshData CreateBox(float width, float height, float depth, uint32 numSubdivisions);

	private:
		Vertex MidPoint(const Vertex& v0, const Vertex& v1);
		void Subdivide(MeshData& meshData);
	};
}