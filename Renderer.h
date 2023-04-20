#pragma once

#include "pch.h"
#include "Mesh.h"

struct Renderer
{
	XMFLOAT4X4 world;
	/*	
		dirty flags.
		Represents a change in the data associated with an object.
		Because each frame resource has an object constant buffer, so we should set like this "numFramesDirty = FrameCount"
	*/
	uint32_t numFramesDirty;

	uint32_t objectIndex;
	
	Geometrie* pGeo;

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType;

	uint32_t indexCount;
	uint32_t startIndex;
	uint32_t baseVertex ;
};