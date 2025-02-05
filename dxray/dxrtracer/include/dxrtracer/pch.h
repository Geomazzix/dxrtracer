#pragma once

#include <core/debug.h>
#include <core/time/stopwatch.h>
#include <core/valueTypes.h>
#include <core/vath/vath.h>

#include <vector>
#include <format>
#include <memory>
#include <queue>

#if defined(PLATFORM_WINDOWS)

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#endif
#include <windows.h>
#include <wrl.h>
#include <shellapi.h>

#include <directx/d3dx12.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <d3dcompiler.h>

using Microsoft::WRL::ComPtr;

#ifndef CONFIG_RELEASE
#define D3D12_CHECK(hr) DXRAY_ASSERT(SUCCEEDED(hr))
#define D3D12_NAME_OBJECT(pObject, nameString)  pObject->SetName(nameString.c_str())
#include <dxgidebug.h>
#else
#define D3D12_CHECK(hr) //Added for readability, would get resolved regardless.
#define D3D12_NAME_OBJECT(pObject, nameString)
#endif

#else
#error "This project only runs on Windows with a DX12_2 compatable graphics card."
#endif