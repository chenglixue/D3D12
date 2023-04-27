/*  
	Precompiled header files
	pch.h : include file for standard system include files,
	or project specific include files that are used frequently, 
	but are changed infrequently
*/

#pragma once

#include <windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <D3d12SDKLayers.h>
#include "d3dx12.h"

// helper function
#include "MathHelper.h"
#include "Common.h"

#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <array>
#include <numeric>
#include <memory>
#include <wrl/client.h>
#include <shellapi.h>
#include <cstdint>

//shader debug
#include "pix3.h"
#include <shlobj.h>
#include <strsafe.h>
