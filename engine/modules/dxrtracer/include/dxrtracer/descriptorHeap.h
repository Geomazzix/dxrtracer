#pragma once

namespace dxray
{
	/**
	 * @brief Small wrapper class to keep track of descriptor heap data as the utility provided by d3d12 is too loosely coupled.
	 */
	class DescriptorHeap final
	{
	public:
		DescriptorHeap();
		DescriptorHeap(ComPtr<ID3D12Device> a_device, D3D12_DESCRIPTOR_HEAP_TYPE a_type, u32 a_capacity, bool a_shaderVisible = true);
		~DescriptorHeap() = default;

		D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(u32 a_slot) const;
		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(u32 a_slot) const;

		[[nodiscard]] u32 GetCapacity() const;
		[[nodiscard]] u32 GetDescriptorSize() const;
		[[nodiscard]] bool IsShaderVisible() const;

		void Bind(ComPtr<ID3D12GraphicsCommandList>& a_commandList) const;

	private:
		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12DescriptorHeap> m_heap;
		D3D12_DESCRIPTOR_HEAP_TYPE m_type;
		u32 m_descriptorSize;
		u32 m_capacity;
		bool m_shaderVisible;
	};
}