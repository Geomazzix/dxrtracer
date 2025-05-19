#include "dxrtracer/shaderCompiler.h"
#include <core/fileSystem/fileIO.h>
#include <core/vath/vath.h>

#define DXC_CHECK(hr) DXRAY_ASSERT(SUCCEEDED(hr))

namespace dxray
{
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

	public:
		DxShaderCompiler();
		~DxShaderCompiler() = default;

		bool CompileShaderSource(const ShaderCompilationInput& a_compilerInput, ShaderCompilationOutput& a_compilerOutput);
		static WString DxcHashToWideString(const u8* a_pDigets);

	private:
		ComPtr<IDxcCompiler3> m_compiler;
		ComPtr<IDxcUtils> m_utilities;
		ComPtr<IncludeHandler> m_includeHandler;
	};

	DxShaderCompiler DxCompiler;


	// --- DxShaderCompiler::IncludeHandler ---

	DxShaderCompiler::IncludeHandler::IncludeHandler(ComPtr<IDxcUtils> a_pDxcUtilities) :
		m_dxcUtilities(a_pDxcUtilities),
		m_refCount(0)
	{ }

	HRESULT STDMETHODCALLTYPE DxShaderCompiler::IncludeHandler::LoadSource(_In_ LPCWSTR a_pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** a_ppIncludeSource)
	{
		ComPtr<IDxcBlobEncoding> pEncoding;
		const WString path = a_pFilename;

		//Check for duplicate includes.
		if (m_includedFiles.find(path) != m_includedFiles.end())
		{
			static const char nullStr[] = " ";
			m_dxcUtilities->CreateBlobFromPinned(nullStr, ARRAYSIZE(nullStr), DXC_CP_ACP, pEncoding.GetAddressOf());
			*a_ppIncludeSource = pEncoding.Detach();
			return S_OK;
		}

		//If found, load the file add it to the included files and return the load result.
		HRESULT hr = m_dxcUtilities->LoadFile(a_pFilename, nullptr, pEncoding.GetAddressOf());
		if (SUCCEEDED(hr))
		{
			m_includedFiles.insert(path);
			*a_ppIncludeSource = pEncoding.Detach();
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
		if (m_refCount <= 0) 
		{
			delete this;
		}
		return m_refCount;
	}


	// --- DxShaderCompiler ---

	DxShaderCompiler::DxShaderCompiler() :
		m_compiler(nullptr),
		m_utilities(nullptr),
		m_includeHandler(nullptr)
	{
		HRESULT result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler));
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

		ComPtr<IDxcVersionInfo> info;
		DXC_CHECK(m_compiler.As(&info));
		vath::Vector2u32 version;
		DXC_CHECK(info->GetVersion(&version.x, &version.y));
		DXRAY_INFO("Loaded DirectX shader compiler version {}.{}", version.x, version.y);
	}

	WString DxShaderCompiler::DxcHashToWideString(const u8* a_pDigets)
	{
		WOStringStream outStream;
		for (size_t i = 0; i < 16; ++i) //These hashes are 16 length hardcoded.
		{
			outStream << std::hex << static_cast<i32>(a_pDigets[i]);
		}
		return outStream.str();
	}

	bool DxShaderCompiler::CompileShaderSource(const ShaderCompilationInput& a_compilationInput, ShaderCompilationOutput& a_compilationOutput)
	{
		std::vector<const wchar*> arguments =
		{
			a_compilationInput.ShaderFile.c_str(),
			L"-E", a_compilationInput.EntryPoint.c_str(),
			L"-T", a_compilationInput.TargetProfile.c_str(),

			//#Note_dxc: It's recommended to strip everything from the shader output blob and retrieve it separately later - this saves diskspace in shipped applications.
			L"Qstrip_debug",
			L"Qstrip_reflect",
			L"Qstrip_priv",
			L"Qstrip_rootsignature"
		};

		if (a_compilationInput.WarningsAreErrors)
		{
			arguments.push_back(L"-WX");
		}

		if (a_compilationInput.SaveSymbols)
		{
			arguments.push_back(L"-Od");
			arguments.push_back(L"-Zs"); //#Note_dxc: using slim pdbs here - only supported by PIX.
			arguments.push_back(L"-Zss");
		}
		else
		{
			switch (a_compilationInput.OptimizationLevel)
			{
			case EOptimizeLevel::O0:
				arguments.push_back(L"-O0");
				break;
			case EOptimizeLevel::O1:
				arguments.push_back(L"-O1");
				break;
			case EOptimizeLevel::O2:
				arguments.push_back(L"-O2");
				break;
			case EOptimizeLevel::O3:
				arguments.push_back(L"-O3");
				break;
			default:
				DXRAY_WARN("Unknown optimization level specified, will use O0");
				arguments.push_back(L"-O0");
				break;
			}
		}

		for (u32 i = 0; i < a_compilationInput.MacroDefinitions.size(); i++)
		{
			arguments.push_back(L"-D");
			arguments.push_back(std::format(L"{}={}",
				StringEncoder::Utf8ToUnicode(a_compilationInput.MacroDefinitions[i].Macro),
				StringEncoder::Utf8ToUnicode(a_compilationInput.MacroDefinitions[i].Value)).c_str()
			);
		}

		ComPtr<IDxcResult> compileResult = nullptr;
		DXC_CHECK(m_compiler->Compile(a_compilationInput.SourceBuffer, arguments.data(), static_cast<u32>(arguments.size()), m_includeHandler.Get(), IID_PPV_ARGS(&compileResult)));

		if (compileResult->HasOutput(DXC_OUT_ERRORS))
		{
			ComPtr<IDxcBlobUtf8> errorBlob = nullptr;
			DXC_CHECK(compileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errorBlob), nullptr));
			if (errorBlob != nullptr && errorBlob->GetStringLength() > 0)
			{
				DXRAY_ERROR("Shader compilation failed: {}", static_cast<const char*>(errorBlob->GetBufferPointer()));
				DXRAY_ASSERT(true); //#Todo_dxc: This assert should be replaced if shader hot-reloading ever becomes a thing.
				return false;
			}
		}

		WString shaderHashString = L"";
		if (compileResult->HasOutput(DXC_OUT_SHADER_HASH))
		{
			DxcShaderHash* shaderHash = nullptr;
			ComPtr<IDxcBlob> shaderHashBlob = nullptr;
			DXC_CHECK(compileResult->GetOutput(DXC_OUT_SHADER_HASH, IID_PPV_ARGS(&shaderHashBlob), nullptr));
			shaderHash = static_cast<DxcShaderHash*>(shaderHashBlob->GetBufferPointer());

			if (!(shaderHash->Flags & DXC_HASHFLAG_INCLUDES_SOURCE))
			{
				DXRAY_WARN("Shader hash did not take source into account, which can result shader collisions: {}", a_compilationInput.ShaderFile.string());
			}
			shaderHashString = DxcHashToWideString(shaderHash->HashDigest);

			const usize hashSize = _countof(shaderHash->HashDigest);
			a_compilationOutput.ShaderHash.reserve(hashSize);
			for (int i = 0; i < hashSize; i++)
			{
				a_compilationOutput.ShaderHash.push_back(shaderHash->HashDigest[i]);
			}
		}
		else
		{
			DXRAY_ERROR("No shader hash found for: {}", a_compilationInput.ShaderFile.string());
			return false;
		}

		if (compileResult->HasOutput(DXC_OUT_OBJECT))
		{
			ComPtr<IDxcBlob> shaderBinary = nullptr;
			DXC_CHECK(compileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBinary), nullptr));
			a_compilationOutput.Binary.Data = shaderBinary->GetBufferPointer();
			a_compilationOutput.Binary.SizeInBytes = shaderBinary->GetBufferSize();
			dxray::WriteBinaryFile(a_compilationInput.ShaderBinDirectory / L"bin" / a_compilationInput.ShaderFile.stem().stem() += L".dxil", a_compilationOutput.Binary);
		}
		else
		{
			DXRAY_ERROR("No shader binary found for: {}", a_compilationInput.ShaderFile.string());
			return false;
		}

		if (compileResult->HasOutput(DXC_OUT_PDB) && a_compilationInput.SaveSymbols)
		{
			ComPtr<IDxcBlob> shaderSymbols = nullptr;
			ComPtr<IDxcBlobUtf16> shaderSymbolsPath = nullptr;

			DXC_CHECK(compileResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&shaderSymbols), &shaderSymbolsPath));
			a_compilationOutput.Symbols.Data = shaderSymbols->GetBufferPointer();
			a_compilationOutput.Symbols.SizeInBytes = shaderSymbols->GetBufferSize();

			dxray::WriteBinaryFile(Path(a_compilationInput.ShaderBinDirectory / L"debug" / shaderSymbolsPath->GetStringPointer()), a_compilationOutput.Symbols);
		}
		else
		{
			DXRAY_WARN("No shader symbols found for: {}", a_compilationInput.ShaderFile.string());
			return false;
		}

		if (compileResult->HasOutput(DXC_OUT_REFLECTION) && a_compilationInput.SaveReflection)
		{
			ComPtr<IDxcBlob> shaderReflection = nullptr;
			DXC_CHECK(compileResult->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&shaderReflection), nullptr));
			a_compilationOutput.Reflection.Data = shaderReflection->GetBufferPointer();
			a_compilationOutput.Reflection.SizeInBytes = shaderReflection->GetBufferSize();
			dxray::WriteBinaryFile(Path(a_compilationInput.ShaderBinDirectory / L"debug" / shaderHashString += ".ref"), a_compilationOutput.Reflection);
		}
		else
		{
			DXRAY_WARN("No shader reflection data found for: {}", a_compilationInput.ShaderFile.string());
			return false;
		}

		return true;
	}


	// --- Shader compilation API ---

	void ShaderCompiler::CompileShaderFilesInDirectory(const Path& a_shaderDirectory, const Path& a_shaderCacheDirectory, const ShaderCompilationOptions& a_options)
	{
		if (!std::filesystem::exists(a_shaderDirectory))
		{
			DXRAY_ERROR("Could not find: {}", a_shaderDirectory.string());
			return;
		}

		for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(a_shaderDirectory))
		{
			//#Note_dxc: DxShaderCompiler currently only supports compiling files with the .hlsl extension.
			if (!file.is_regular_file() || file.path().filename().extension() != ".hlsl")
			{
				continue;
			}

			const Path absolutePath = a_shaderDirectory / file;
			const String source = dxray::ReadFile(absolutePath);

			ShaderCompilationOutput result;
			DXRAY_ASSERT(CompileShaderFile(file, a_shaderCacheDirectory, a_options, result));
		}
	}

	bool ShaderCompiler::CompileShaderFile(const Path& a_filePath, const Path& a_binDirectory, const ShaderCompilationOptions& a_options, ShaderCompilationOutput& a_compileResult)
	{
		//Create the buffer to store the raw shader source.
		const String fileContents = ReadFile(a_filePath);
		DXRAY_ASSERT(!fileContents.empty());

		const DxcBuffer buffer =
		{
			.Ptr = fileContents.data(),
			.Size = fileContents.size(),
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
			: L""; //#Note_dxc: Raytrace shader files are libraries and don't have entry points.

		const ShaderCompilationInput input =
		{
			.MacroDefinitions = a_options.MacroDefinitions,
			.SourceBuffer = &buffer,
			.ShaderFile = a_filePath,
			.ShaderBinDirectory = a_binDirectory,
			.TargetProfile = targetProfile,
			.EntryPoint = shaderEntryPoint,
			.OptimizationLevel = a_options.OptimizeLevel,
			.WarningsAreErrors = a_options.WarningsAreErrors,
			.SaveSymbols = a_options.SaveSymbols,
			.SaveReflection = a_options.SaveReflection
		};

		if (DxCompiler.CompileShaderSource(input, a_compileResult))
		{
			DXRAY_INFO("Compiled shader {} successfully", a_filePath.string());
			return true;
		}

		//#Todo_dxc: Error could be vague, see todo for enhanced error handling using error code/custom result types.
		DXRAY_ERROR("Failed to compile {}!", a_filePath.string());
		return false;
	}
}