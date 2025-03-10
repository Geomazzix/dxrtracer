#pragma once
#include <core/fileSystem/fileIO.h>

namespace dxray
{
	/*!
	 * @brief Identifier for the shading capabilities of the selected GPU. All other graphics APIs should be able
	 * to map to these identifiers.
	 */
	enum class EShaderModel : u8
	{
		SM5_0 = 0,
		SM5_1,
		SM6_0,
		SM6_1,
		SM6_2,
		SM6_3,
		SM6_4,
		SM6_5,
		SM6_6
	};

	inline String ShaderModelVersionToUtf8(EShaderModel a_shaderModel)
	{
		switch (a_shaderModel)
		{
		case EShaderModel::SM5_0: return "5_0";
		case EShaderModel::SM5_1: return "5_1";
		case EShaderModel::SM6_0: return "6_0";
		case EShaderModel::SM6_1: return "6_1";
		case EShaderModel::SM6_2: return "6_2";
		case EShaderModel::SM6_3: return "6_3";
		case EShaderModel::SM6_4: return "6_4";
		case EShaderModel::SM6_5: return "6_5";
		case EShaderModel::SM6_6: return "6_6";
		default:
			DXRAY_CRITICAL("Unsupported version for shader model for this GPU!");
			return "";
		}
	}


	/*!
	 * @brief Currently supported shader types.
	 */
	enum class EShaderType : u8
	{
		Vertex = 0,
		Geometry,
		Fragment,
		Raytrace,
		Compute,
		Count
	};


	/*!
	 * @brief Used to identify the optimization level used by the HlslShaderCompiler.
	 * #Todo: Add aliases for specific compiler flags - e.g. optimize for speed/memory.
	 */
	enum class EOptimizeLevel : u8
	{
		O0 = 0,
		O1,
		O2,
		O3
	};


	/*!
	 * @brief Custom macro definition for within shaders.
	 */
	struct MacroDefinition
	{
		String Macro = "";
		String Value = "";
	};


	/*!
	 * @brief Contains the data used in shader reflection.
	 */
	struct ShaderReflectionBuffer
	{
		DataBlob Blob;
		u32 Encoding;
	};


	/*!
	 * @brief The options the shader compiler provides.
	 * #Note: Potentially just select the highest available shader model and compile all with that. It's currently uncertain what potential downsides this could have. Though it would mean this option can be removed.
	 */
	struct ShaderCompilationOptions
	{
		std::vector<MacroDefinition> MacroDefinitions;
		EOptimizeLevel OptimizeLevel;
		EShaderModel ShaderModel;
		String EntryPoint;
		u8 WarningsAreErrors : 1;
		u8 SaveSymbols : 1;
		u8 SaveReflection : 1;
	};


	/*!
	 * @brief The output of a compiled shader unit.
	 */
	struct ShaderCompilationOutput final
	{
		const bool HasBinary() const;
		const bool HasSymbols() const;
		const bool HasReflection() const;

		DataBlob Binary;
		DataBlob Symbols;
		DataBlob Reflection;
	};

	inline const bool ShaderCompilationOutput::HasBinary() const
	{
		return Binary.SizeInBytes > 0;
	}

	inline const bool ShaderCompilationOutput::HasSymbols() const
	{
		return Symbols.SizeInBytes > 0;
	}

	inline const bool ShaderCompilationOutput::HasReflection() const
	{
		return Reflection.SizeInBytes > 0;
	}
}