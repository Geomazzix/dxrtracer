#include "dxrtracer/descriptorHeap.h"

namespace dxray
{
	DescriptorHeap::DescriptorHeap() :
		m_device(nullptr),
		m_heap(nullptr),
		m_type(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
		m_descriptorSize(0u),
		m_capacity(0u),
		m_shaderVisible(false)
	{}

	DescriptorHeap::DescriptorHeap(ComPtr<ID3D12Device> a_device, D3D12_DESCRIPTOR_HEAP_TYPE a_type, u32 a_capacity, bool a_shaderVisible) :
		m_device(a_device),
		m_heap(nullptr),
		m_type(a_type),
		m_descriptorSize(0u),
		m_capacity(a_capacity),
		m_shaderVisible(a_shaderVisible)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc;
		desc.Type = m_type;
		desc.Flags = m_shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;
		desc.NumDescriptors = m_capacity;

		D3D12_CHECK(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap)));
		m_descriptorSize = m_device->GetDescriptorHandleIncrementSize(m_type);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCpuHandle(u32 a_slot) const
	{
		assert(a_slot < m_capacity);
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(a_slot), m_descriptorSize);
	}
	
	D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGpuHandle(u32 a_slot) const
	{
		assert(a_slot < m_capacity);
		assert(m_shaderVisible);
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_heap->GetGPUDescriptorHandleForHeapStart(), static_cast<INT>(a_slot), m_descriptorSize);
	}
	
	u32 DescriptorHeap::GetCapacity() const
	{
		return m_capacity;
	}
	
	u32 DescriptorHeap::GetDescriptorSize() const
	{
		return m_descriptorSize;
	}
	
	bool DescriptorHeap::IsShaderVisible() const
	{
		return m_shaderVisible;
	}
	
	void DescriptorHeap::Bind(ComPtr<ID3D12GraphicsCommandList>& a_commandList) const
	{
		assert(m_shaderVisible && "Cannot bind a non-shader-visible heap");
		ID3D12DescriptorHeap* heaps[] = { m_heap.Get() };
		a_commandList->SetDescriptorHeaps(1, heaps);
	}
}