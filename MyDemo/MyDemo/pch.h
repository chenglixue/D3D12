/*  
	Precompiled header files
	pch.h : include file for standard system include files,
	or project specific include files that are used frequently, 
	but are changed infrequently
*/

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include "d3dx12.h"
#include "MathHelper.h"

#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <array>
#include <numeric>
#include <memory>
#include <wrl.h>
#include <shellapi.h>
#include <cstdint>

//shader debug
#include "pix3.h"
#include <filesystem>
#include <shlobj.h>
