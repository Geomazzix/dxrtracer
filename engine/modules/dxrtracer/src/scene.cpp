#include "dxrtracer/scene.h"
#include "dxrtracer/modelLoader.h"
#include "dxrtracer/uploadBuffer.h"

namespace dxray
{
	Scene::Scene() :
		m_isDirty(true)
	{
	}

	void Scene::Tick(const float a_dt)
	{
		using namespace DirectX;

		if (m_isDirty)
		{
			return;
		}

		// Disable when rendering sponza
		static fp32 time = 0.0f;
		time += a_dt;

		auto mesh = XMMatrixScaling(2, 2, 2);
		mesh *= XMMatrixRotationRollPitchYaw(time / 2, time / 3, time / 5);
		mesh *= XMMatrixTranslation(-1.5, 2, 2);
		XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(&m_sceneObjectInstances[0].Transform), mesh);

		auto mirror = XMMatrixScaling(3, 3, 3);
		mirror *= XMMatrixRotationX(-1.8f);
		mirror *= XMMatrixRotationY(XMScalarSinEst(time) / 8 + 1);
		mirror *= XMMatrixTranslation(2, 2, 2);
		XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(&m_sceneObjectInstances[1].Transform), mirror);

		auto floor = XMMatrixScaling(3, 3, 3);
		floor *= XMMatrixScaling(5, 5, 5);
		floor *= XMMatrixTranslation(0, 0, 2);
		XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(&m_sceneObjectInstances[2].Transform), floor);
		// end disable when rendering sponza

		m_isDirty = true;
	}

	void Scene::AddSceneObjectInstance(const AccelerationStructure& a_blasData)
	{
		// #Todo: Unloading instances/models is not supported while this integer is counting upwards. 
				// Change this to an argument to allow support for this as the indices need to be managed.
		static u32 objectCount = 0;

		D3D12_RAYTRACING_INSTANCE_DESC desc;

		// #Note: Sponza code - no scene hierarchy is in-place, this sponza is only 1 parent node which scales the model down significantly.
		//XMStoreFloat3x4(reinterpret_cast<DirectX::XMFLOAT3X4*>(&desc.Transform), DirectX::XMMatrixScaling(0.008f, 0.008f, 0.008f));

		desc.InstanceID = objectCount++;
		desc.InstanceMask = 1;
		desc.AccelerationStructure = a_blasData.Buffer->GetGPUVirtualAddress();
		m_sceneObjectInstances.push_back(desc);

		m_isDirty = true;
	}

	void Scene::UpdateTlas(ComPtr<ID3D12Device> a_device, ComPtr<ID3D12GraphicsCommandList>& a_cmdList, AccelerationStructure& a_tlas, ComPtr<ID3D12Resource>& a_instanceBuffer)
	{
		if (!m_isDirty)
		{
			return;
		}

		const D3D12_RESOURCE_DESC bufferDesc =
		{
			.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
			.Alignment = 0,
			.Width = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * m_sceneObjectInstances.size(),
			.Height = 1,
			.DepthOrArraySize = 1,
			.MipLevels = 1,
			.Format = DXGI_FORMAT_UNKNOWN,
			.SampleDesc = { 1, 0 },
			.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
			.Flags = D3D12_RESOURCE_FLAG_NONE
		};

		const D3D12_HEAP_PROPERTIES heapProps =
		{
			.Type = D3D12_HEAP_TYPE_UPLOAD
		};

		D3D12_CHECK(a_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&a_instanceBuffer)));
		if (!a_instanceBuffer)
		{
			return;
		}

		void* mappedSceneObjectsAddr = nullptr;
		D3D12_CHECK(a_instanceBuffer->Map(0, nullptr, &mappedSceneObjectsAddr));
		D3D12_RAYTRACING_INSTANCE_DESC* const instances = static_cast<D3D12_RAYTRACING_INSTANCE_DESC* const>(mappedSceneObjectsAddr);

		for (usize descIdx = 0; descIdx < m_sceneObjectInstances.size(); descIdx++)
		{
			instances[descIdx] = m_sceneObjectInstances[descIdx];
		}

		if (mappedSceneObjectsAddr)
		{
			a_instanceBuffer->Unmap(0, nullptr);
		}

		const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlasInputs =
		{
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
			.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE,
			.NumDescs = static_cast<u32>(m_sceneObjectInstances.size()),
			.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
			.InstanceDescs = a_instanceBuffer->GetGPUVirtualAddress()
		};

		CreateAccelerationStructure(a_device, a_cmdList, a_tlas, tlasInputs);

		D3D12_NAME_OBJECT(a_tlas.Scratch, std::format(L"WorldTlasScratch"));
		D3D12_NAME_OBJECT(a_tlas.Buffer, std::format(L"WorldTlas"));
		m_isDirty = false;
	}
}