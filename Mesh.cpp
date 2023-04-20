#include "pch.h"
#include "Mesh.h"
#include "FrameResource.h"

#ifdef PROCEDURALGEOMETRY

ProceduralGeometry::MeshData ProceduralGeometry::CreateGrid(float width, float depth, uint32_t m, uint32_t n)
{
	MeshData meshData;

	uint32_t vertexCount = m * n;
	uint32_t faceCount = (m - 1) * (n - 1) * 2;

	//
	// Create the vertices.
	//

	float halfWidth = 0.5f * width;
	float halfDepth = 0.5f * depth;

	float dx = width / (n - 1);
	float dz = depth / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	meshData.vertices.resize(vertexCount);
	for (uint32_t i = 0; i < m; ++i)
	{
		float z = halfDepth - i * dz;
		for (uint32_t j = 0; j < n; ++j)
		{
			float x = -halfWidth + j * dx;

			meshData.vertices[i * n + j].position = XMFLOAT3(x, 0.0f, z);
			meshData.vertices[i * n + j].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			meshData.vertices[i * n + j].tangentU = XMFLOAT3(1.0f, 0.0f, 0.0f);

			// Stretch texture over grid.
			meshData.vertices[i * n + j].texC.x = j * du;
			meshData.vertices[i * n + j].texC.y = i * dv;
		}
	}

	//
	// Create the indices.
	//

	meshData.indices32.resize(faceCount * 3); // 3 indices per face

	// Iterate over each quad and compute indices.
	uint32_t k = 0;
	for (uint32_t i = 0; i < m - 1; ++i)
	{
		for (uint32_t j = 0; j < n - 1; ++j)
		{
			meshData.indices32[k] = i * n + j;
			meshData.indices32[k + 1] = i * n + j + 1;
			meshData.indices32[k + 2] = (i + 1) * n + j;

			meshData.indices32[k + 3] = (i + 1) * n + j;
			meshData.indices32[k + 4] = i * n + j + 1;
			meshData.indices32[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}

	return meshData;
}

void ProceduralGeometry::CreateLand(
	ComPtr<ID3D12Device> device,
	ComPtr<ID3D12GraphicsCommandList> cmdList,
	std::unordered_map<std::string, std::unique_ptr<Mesh>>& geometries,
	std::unordered_map<std::string, std::unique_ptr<Mesh::Draw>>& draws)
{
	MeshData grid = CreateGrid(160.f, 160.f, 50, 50);

	std::vector<InstanceVertex> vertices(grid.vertices.size());
	std::vector<std::uint16_t> indices = grid.GetIndices16();

	for (UINT i = 0; i < grid.vertices.size(); ++i)
	{
		auto& p = grid.vertices[i].position;

		auto GetHillsHeight = [&]() -> float
		{
			return 0.3f * (p.z * sinf(0.1f * p.x) + p.x * cosf(0.1f * p.z));
		};

		vertices[i].pos = p;
		vertices[i].pos.y = GetHillsHeight();

		if (vertices[i].pos.y < -10.f)
		{
			vertices[i].color = XMFLOAT4(1.f, 0.9f, 0.62f, 1.f);
		}
		else if (vertices[i].pos.y < 5.f)
		{
			vertices[i].color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.f);
		}
		else if (vertices[i].pos.y < 12.f)
		{
			vertices[i].color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.f);
		}
		else if (vertices[i].pos.y < 20.f)
		{
			vertices[i].color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
		}
		else
		{
			vertices[i].color = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
		}
	}

	auto pGeo = std::make_unique<Mesh>();

	pGeo->name = "Land";
	pGeo->vbSize = (uint32_t)vertices.size() * sizeof(InstanceVertex);
	pGeo->vbStride = sizeof(InstanceVertex);
	pGeo->ibSize = (uint32_t)indices.size() * sizeof(uint16_t);
	pGeo->ibFormat = DXGI_FORMAT_R16_UINT;

	ThrowIfFailed(D3DCreateBlob(pGeo->vbSize, &pGeo->vertexBufferCPU));
	CopyMemory(pGeo->vertexBufferCPU->GetBufferPointer(), vertices.data(), pGeo->vbSize);

	ThrowIfFailed(D3DCreateBlob(pGeo->ibSize, &pGeo->indexBufferCPU));
	CopyMemory(pGeo->indexBufferCPU->GetBufferPointer(), indices.data(), pGeo->ibSize);

	pGeo->vertexBufferGPU = CreateDefaultBuffer(device.Get(), cmdList.Get(), vertices.data(), pGeo->vbSize, pGeo->vertexUploadBuffer);
	pGeo->indexBufferGPU = CreateDefaultBuffer(device.Get(), cmdList.Get(), indices.data(), pGeo->ibSize, pGeo->indexUploadBuffer);

	auto landDraw = std::make_unique<Mesh::Draw>();
	landDraw->baseVertex = 0;
	landDraw->indexCount = (uint32_t)indices.size();
	landDraw->startIndex = 0;

	geometries["Land"] = std::move(pGeo);
	draws["Land"] = std::move(landDraw);
}

#endif // 