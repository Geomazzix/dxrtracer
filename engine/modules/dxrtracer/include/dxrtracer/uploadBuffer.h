#pragma once

namespace dxray
{
	// #note_renderer: Abstract this function into a proper upload heap.
	extern void CreateReadBackBuffer(ComPtr<ID3D12Device>& a_device, ComPtr<ID3D12Resource>& a_resource, const void* a_pData, const usize a_sizeInBytes);
}