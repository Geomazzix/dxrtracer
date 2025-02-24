#pragma once
#include <core/string.h>
#include <dxcapi.h>
#include "dxrtracer/shaderCompiler.h"

namespace dxray
{
	/*!
	 * @brief Capable of individual or directory shader compilation.
	 * #Todo: For multi threading purposes - include a way to pass bufferSource through, instead of having to rely on the DXC API to read file contents.
	 */
	class DxShaderCompiler final
	{
		/*!
		 * @brief Include handler used to handle in-shader includes.
		 */
		class IncludeHandler final : public IDxcIncludeHandler
		{
		public:
			IncludeHandler(ComPtr<IDxcUtils> a_pDxcUtilities);
			~IncludeHandler() = default;

			HRESULT STDMETHODCALLTYPE LoadSource(_In_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override;
			HRESULT QueryInterface(REFIID riid, void** ppvObject) override;
			ULONG AddRef() override;
			ULONG Release() override;

		private:
			std::unordered_set<WString> m_includedFiles;
			ComPtr<IDxcUtils> m_dxcUtilities;
			volatile ULONG m_refCount;
		};

		/*!
		 * @brief The input needed for the shader compiler to compile a single shader.
		 */
		struct ShaderCompilationInput
		{
			std::vector<MacroDefinition> MacroDefinitions;
			Path FilePath;
			WString TargetProfile;
			WString EntryPoint;
			EOptimizeLevel OptimizationLevel;
			bool ShouldKeepDebugData;
		};

	public:
		DxShaderCompiler();
		~DxShaderCompiler() = default;
	
		ShaderCompilationOutput CompileShader(const Path& a_filePath, const ShaderCompilationOptions& a_options);
		void CompileShadersInDirectory(const Path& a_shaderDirectory, const Path& a_shaderCacheDirectory, const ShaderCompilationOptions& a_options);

	private:
		bool CompileShaderSource(const DxcBuffer* a_pSourceBuffer, const ShaderCompilationInput& a_compilerInput, ShaderCompilationOutput& a_compilerOutput);

		ComPtr<IDxcCompiler3> m_compiler;
		ComPtr<IDxcUtils> m_utilities;
		ComPtr<IncludeHandler> m_includeHandler;
	};
}