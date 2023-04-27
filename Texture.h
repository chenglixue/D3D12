#pragma once
#include "pch.h"
#include "DXSampleHelper.h"

using Microsoft::WRL::ComPtr;

namespace Core
{
	struct Texture
	{
		std::string name;	// texture name

		std::wstring fileName;	// texture file name

		ComPtr<ID3D12Resource> defaultTexture;	// texture resource for default heap

		std::unique_ptr<uint8_t[]> ddsData;	// for LoadDDSTextureFromFile()

		std::vector<D3D12_SUBRESOURCE_DATA> subResources; // for LoadDDSTextureFromFile()

		ComPtr<ID3D12Resource> uploadTexture;	// texture resource for upload heap to copy date to default texture
	};

	// copy dds data from subresource to defualt buffer and upload buffer is a intermediate object 
	static void CreateD3DResource(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		std::vector<D3D12_SUBRESOURCE_DATA>& subResources,
		ComPtr<ID3D12Resource>& defaultTexture,
		ComPtr<ID3D12Resource>& uploadTexture
	)

	{
		const UINT64 textureUploadBufferSize = GetRequiredIntermediateSize(defaultTexture.Get(), 0, static_cast<UINT>(subResources.size()));

		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);

		auto desc = CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize);

		ThrowIfFailed(device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadTexture.GetAddressOf())
		));

		cmdList->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(defaultTexture.Get(),
				D3D12_RESOURCE_STATE_COMMON,
				D3D12_RESOURCE_STATE_COPY_DEST
			));

		UpdateSubresources(cmdList, defaultTexture.Get(), uploadTexture.Get(), 0, 0, static_cast<UINT>(subResources.size()), subResources.data());

		cmdList->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(defaultTexture.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
			));
	}

	// static sampler
	static std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers()
	{
		const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
			0, // shader register
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
			1,
			D3D12_FILTER_MIN_MAG_MIP_POINT,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

		const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
			2,
			D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP);

		const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
			3,
			D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

		const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
			4,
			D3D12_FILTER_ANISOTROPIC,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP
		);

		const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
			5,
			D3D12_FILTER_ANISOTROPIC,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP
		);

		return { pointWrap, pointClamp, linearWrap, linearClamp, anisotropicWrap, anisotropicClamp };
	}
}