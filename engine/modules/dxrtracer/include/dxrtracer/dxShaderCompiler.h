#pragma once
#include <core/string.h>
#include <dxcapi.h>
#include <d3d12shader.h>
#include "dxrtracer/shaderCompiler.h"

namespace dxray
{
	/*!
	 * @brief Capable of individual or directory shader compilation.
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
		 * @brief The input needed for the shader compiler to compile a shader file (can contain multiple shaders).
		 */
		struct ShaderCompilationInput
		{
			std::vector<MacroDefinition> MacroDefinitions;
			const DxcBuffer* SourceBuffer;
			Path ShaderFile;
			Path ShaderBinDirectory;
			WString TargetProfile;
			WString EntryPoint;
			EOptimizeLevel OptimizationLevel;
			u8 WarningsAreErrors : 1;
			u8 SaveSymbols : 1;						//Saved as .pdb.
			u8 SaveReflection : 1;					//Saved as .ref
		};

	public:
		DxShaderCompiler();
		~DxShaderCompiler() = default;
	
		void CompileShadersInDirectory(const Path& a_shaderDirectory, const Path& a_shaderCacheDirectory, const ShaderCompilationOptions& a_options);
		bool CompileShaderFile(const Path& a_filePath, const Path& a_binDirectory, const ShaderCompilationOptions& a_options, ShaderCompilationOutput& a_compileResult);
		bool GetShaderReflection(const DataBlob& a_shaderReflectionBlob, ComPtr<ID3D12ShaderReflection>& a_shaderReflection);
		//#Todo: For multi threading purposes - include a way to pass bufferSource through, instead of having to rely on the DXC API to read file contents.

	private:
		bool CompileShaderSource(const ShaderCompilationInput& a_compilerInput, ShaderCompilationOutput& a_compilerOutput);
		static WString DxcHashToWideString(const u8* a_pDigets);

		ComPtr<IDxcCompiler3> m_compiler;
		ComPtr<IDxcUtils> m_utilities;
		ComPtr<IncludeHandler> m_includeHandler;
	};
}