#include "dxrtracer/uploadBuffer.h"

namespace dxray
{
	void CreateReadBackBuffer(ComPtr<ID3D12Device>& a_device, ComPtr<ID3D12Resource>& a_resource, const void* a_pData, const usize a_sizeInBytes)
	{
		//A buffer in d3d12 is represented as a 1 dimensional aligned resource.
		const D3D12_RESOURCE_DESC bufferDesc =
		{
			.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
			.Alignment = 0,
			.Width = a_sizeInBytes,
			.Height = 1,
			.DepthOrArraySize = 1,
			.MipLevels = 1,
			.Format = DXGI_FORMAT_UNKNOWN,
			.SampleDesc = { 1, 0 },
			.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
			.Flags = D3D12_RESOURCE_FLAG_NONE
		};

		const D3D12_HEAP_PROPERTIES uploadHeapProperties =
		{
			.Type = D3D12_HEAP_TYPE_UPLOAD
		};

		D3D12_CHECK(a_device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&a_resource)));
		if (a_resource != nullptr)
		{
			void* mappedAddr = nullptr;
			a_resource->Map(0, nullptr, &mappedAddr);
			memcpy(mappedAddr, a_pData, a_sizeInBytes);
			a_resource->Unmap(0, nullptr);
		}
	}
}