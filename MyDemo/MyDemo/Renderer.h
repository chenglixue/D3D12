#pragma once

#include "pch.h"
#include "Mesh.h"

struct Renderer
{
	XMFLOAT4X4 world = MathHelper::Identity4x4();
	/*	
		dirty flags.
		Represents a change in the data associated with an object.
		Because each frame resource has an object constant buffer, so we should set like this "numFramesDirty = FrameCount"
	*/
	uint32_t numFramesDirty;

	uint32_t objectIndex = 0;
	
	Mesh* Geo = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	uint32_t indexCount = 0;
	uint32_t startIndex = 0;
	uint32_t baseVertex = 0;
};