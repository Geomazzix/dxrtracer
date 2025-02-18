#include "dxrtracer/dxShaderCompiler.h"
#include <core/fileSystem/fileIO.h>

#define DXC_CHECK(hr) DXRAY_ASSERT(SUCCEEDED(hr))

namespace dxray
{
	// --- DxShaderCompiler::IncludeHandler ---

	DxShaderCompiler::IncludeHandler::IncludeHandler(ComPtr<IDxcUtils> a_pDxcUtilities) :
		m_dxcUtilities(a_pDxcUtilities)
	{ }

	HRESULT STDMETHODCALLTYPE DxShaderCompiler::IncludeHandler::LoadSource(_In_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource)
	{
		ComPtr<IDxcBlobEncoding> pEncoding;
		const WString path = pFilename;

		//Check for duplicate includes.
		if (m_includedFiles.find(path) != m_includedFiles.end())
		{
			static const char nullStr[] = " ";
			m_dxcUtilities->CreateBlobFromPinned(nullStr, ARRAYSIZE(nullStr), DXC_CP_ACP, pEncoding.GetAddressOf());
			*ppIncludeSource = pEncoding.Detach();
			return S_OK;
		}

		//If found, load the file add it to the included files and return the load result.
		HRESULT hr = m_dxcUtilities->LoadFile(pFilename, nullptr, pEncoding.GetAddressOf());
		if (SUCCEEDED(hr))
		{
			m_includedFiles.insert(path);
			*ppIncludeSource = pEncoding.Detach();
		}

		return hr;
	}

	HRESULT DxShaderCompiler::IncludeHandler::QueryInterface(REFIID riid, void** ppvObject)
	{
		if (riid == __uuidof(IDxcIncludeHandler) || riid == __uuidof(IUnknown)) 
		{
			*ppvObject = static_cast<IDxcIncludeHandler*>(this);
			AddRef();
			return S_OK;
		}

		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	ULONG DxShaderCompiler::IncludeHandler::AddRef()
	{
		InterlockedIncrement(&m_refCount);
		return m_refCount;
	}

	ULONG DxShaderCompiler::IncludeHandler::Release()
	{
		InterlockedDecrement(&m_refCount);
		if (m_refCount == 0) 
		{
			delete this;
		}

		return m_refCount;
	}


	// --- HlslCompiler ---

	DxShaderCompiler::DxShaderCompiler() :
		m_library(nullptr),
		m_compiler(nullptr),
		m_utilities(nullptr),
		m_includeHandler(nullptr)
	{
		HRESULT result = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_library));
		if (FAILED(result))
		{
			DXRAY_CRITICAL("Could not create the DXC library instance.");
			return;
		}

		result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler));
		if (FAILED(result))
		{
			DXRAY_CRITICAL("Could not create the DXC compiler instance.");
			return;
		}

		result = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utilities));
		if (FAILED(result))
		{
			DXRAY_CRITICAL("Could not create the DXC utilities instance.");
			return;
		}

		m_includeHandler = new DxShaderCompiler::IncludeHandler(m_utilities);
	}

	ShaderCompilationOutput DxShaderCompiler::CompileShader(const Path& a_filePath, const ShaderCompilationOptions& a_options)
	{
		//Create the buffer to store the raw shader source.
		const WString file = StringEncoder::Utf8ToUnicode(a_filePath.string());
		u32 codePage = DXC_CP_ACP;
		ComPtr<IDxcBlobEncoding> sourceBlob;

		HRESULT result = m_utilities->LoadFile(file.c_str(), &codePage, &sourceBlob);
		if (FAILED(result))
		{
			DXRAY_ERROR("Could not load the shader source.");
			return {};
		};

		const DxcBuffer buffer =
		{
			.Ptr = sourceBlob->GetBufferPointer(),
			.Size = sourceBlob->GetBufferSize(),
			.Encoding = DXC_CP_ACP,
		};

		//Select target profile based on shader file extension
		WString targetProfile;
		static std::unordered_map<Path, WString> targetProfileMapping =
		{
			{ ".vert", L"vs" },
			{ ".geom", L"gs" },
			{ ".frag", L"ps" },
			{ ".comp", L"cs" },
			{ ".rt", L"lib" }
		};

		targetProfile = std::format(L"{}_{}", targetProfileMapping[a_filePath.stem().extension()], StringEncoder::Utf8ToUnicode(ShaderModelVersionToUtf8(a_options.ShaderModel)));

		//Configure the compiler.
		const WString shaderEntryPoint = !a_options.EntryPoint.empty()
			? StringEncoder::Utf8ToUnicode(a_options.EntryPoint)
			: L""; //#Note: Raytrace shaderfiles are libraries and don't have entrypoints.

		const ShaderCompilationInput input =
		{
			.MacroDefinitions = a_options.MacroDefinitions,
			.FilePath = a_filePath,
			.TargetProfile = targetProfile,
			.EntryPoint = shaderEntryPoint,
			.OptimizationLevel = a_options.OptimizeLevel,
			.ShouldKeepDebugData = a_options.ShouldKeepDebugInfo
		};

		ShaderCompilationOutput out;
		if (CompileShaderSource(&buffer, input, out))
		{
			DXRAY_INFO("Compiled shader {} successfully", a_filePath.string());
			return out;
		}

		//#Note: Error could be vague, see todo for enhanced error handling using error code/custom result types.
		DXRAY_ERROR("Failed to compile {}!", a_filePath.string());
		return{};
	}

	void DxShaderCompiler::CompileShadersInDirectory(const Path& a_shaderDirectory, const Path& a_shaderCacheDirectory, const ShaderCompilationOptions& a_options)
	{
		if (!std::filesystem::exists(a_shaderDirectory))
		{
			DXRAY_ERROR("Could not find: {}", a_shaderDirectory.string());
			return;
		}

		for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(a_shaderDirectory))
		{
			//#Note: DxShaderCompiler currently only supports compiling files with the .hlsl extension.
			if (!file.is_regular_file() || file.path().filename().extension() != ".hlsl")
			{
				continue;
			}

			const Path absolutePath = a_shaderDirectory / file;
			const String source = dxray::Read(absolutePath);

			//#note: Could potentially be multithreaded? - not sure how dxc likes this :/
			const ShaderCompilationOutput result = CompileShader(file, a_options);
			dxray::WriteBinary(a_shaderCacheDirectory / absolutePath.stem().replace_extension(".dxil"), result.Data, result.SizeInBytes);
		}
	}

	bool DxShaderCompiler::CompileShaderSource(const DxcBuffer* a_pSourceBuffer, const ShaderCompilationInput& a_compilerInput, ShaderCompilationOutput& a_compilerOutput)
	{
		//#Todo: double check the debug stripped data and how it can be retrieved.
		std::vector<const wchar*> arguments =
		{
			a_compilerInput.FilePath.c_str(),				//(Optional) name of the shader file to be displayed e.g. in an error message
			L"-E", a_compilerInput.EntryPoint.c_str(),		//Shader main entry point - in case of a library is empty.
			L"-T", a_compilerInput.TargetProfile.c_str(),	//Shader target profile
			DXC_ARG_WARNINGS_ARE_ERRORS,					//Force WX flag for warnings are errors.
			L"-Qstrip_debug"								//Strip debug data - retrieved later down compilation.
		};

		//Debug options.
		if (a_compilerInput.ShouldKeepDebugData)
		{
			arguments.push_back(DXC_ARG_DEBUG);
		}

		//Set optimization level.
		switch (a_compilerInput.OptimizationLevel)
		{
		case EOptimizeLevel::O0:
			arguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL0);
			break;
		case EOptimizeLevel::O1:
			arguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL1);
			break;
		case EOptimizeLevel::O2:
			arguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL2);
			break;
		case EOptimizeLevel::O3:
			arguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);
			break;
		default:
			DXRAY_WARN("Unknown optimization level specified, will use O2");
			break;
		}

		for (u32 i = 0; i < a_compilerInput.MacroDefinitions.size(); i++)
		{
			arguments.push_back(L"-D");
			arguments.push_back(std::format(L"{}={}", StringEncoder::Utf8ToUnicode(a_compilerInput.MacroDefinitions[i].Macro), StringEncoder::Utf8ToUnicode(a_compilerInput.MacroDefinitions[i].Value)).c_str());
		}

		// Compile shader
		ComPtr<IDxcResult> compileResult = nullptr;
		DXC_CHECK(m_compiler->Compile(a_pSourceBuffer, arguments.data(), static_cast<u32>(arguments.size()), m_includeHandler.Get(), IID_PPV_ARGS(&compileResult)));

		ComPtr<IDxcBlobUtf8> errorBlob;
		DXC_CHECK(compileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errorBlob), nullptr));
		if (errorBlob != nullptr && errorBlob->GetStringLength() > 0)
		{
			DXRAY_ERROR("Shader compilation failed: {}", static_cast<const char*>(errorBlob->GetBufferPointer()));
			DXRAY_ASSERT(true);
			return false;
		}

		// Store the compilation blob.
		ComPtr<IDxcBlob> code;
		DXC_CHECK(compileResult->GetResult(&code));
		a_compilerOutput.SizeInBytes = static_cast<u32>(code->GetBufferSize());
		a_compilerOutput.Data = malloc(a_compilerOutput.SizeInBytes);
		if (a_compilerOutput.Data)
		{
			std::memcpy(a_compilerOutput.Data, code->GetBufferPointer(), a_compilerOutput.SizeInBytes);
			return true;
		}

		return false;
	}
}