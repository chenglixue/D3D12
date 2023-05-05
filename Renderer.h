#pragma once

#include "pch.h"
#include "Geometry.h"
#include "Light.h"

namespace Core
{
	struct Renderer
	{
		Renderer() = default;

		XMFLOAT4X4 world;

		/*
			dirty flags.
			Represents a change in the data associated with an object.
			Because each frame resource has an object constant buffer, so we should set like this "numFramesDirty = FrameCount"
		*/
		uint32_t numFramesDirty;

		uint32_t objectIndex;

		Model::Geometrie* pGeometry;

		Material* pMaterail;

		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType;

		uint32_t indexCount;
		uint32_t startIndex;
		uint32_t baseVertex;
	};

	enum class RenderLayer : int
	{
		Opaque = 0,
		Sky = 1,
		Count
	};
}