#pragma once
#include "pch.h"

using namespace DirectX;

#define MAX_LIGHT_COUNT 3

namespace Core
{
	struct Material
	{
		// material name for looking up
		std::string name;

		// index of constant buffer
		UINT materialCBIndex;

		// diffuse index of Srv heap
		UINT diffuseSrvHeapIndex;

		UINT specularSrvHeapIndex;

		UINT numFramesDirty;

		// specualr equation's pow
		int specualrShiness;

		// ambient light strength
		float ambientAlbedo;
	};

	struct Light
	{
		// light color
		XMFLOAT3 lightColor;

		// for falloff function
		float falloffStart;

		// light direction
		XMFLOAT3 lightDirection;

		// for falloff function
		float falloffEnd;

		// light position
		XMFLOAT3 lightPosition;

		// spot equation's pow
		float spotPower;
	};
}
