#include "core/fileSystem/stbIO.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stbImageLoad.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stbImageWrite.h>

namespace dxray
{
	void StorePNG(String a_fileName, i32 a_width, i32 a_height, i32 a_numChannels, vath::Vector3f* a_pColorData, bool a_bNormalizedData)
	{
		const i32 pixelCount = a_width * a_height;
		u8* pixels = new u8[pixelCount * a_numChannels];
	
		for (i32 pixelIndex = 0; pixelIndex < pixelCount * a_numChannels; pixelIndex)
		{
			for (i32 channelIndex = 0; channelIndex < a_numChannels; channelIndex++)
			{
				if (a_bNormalizedData)
				{
					fp32 channelValue = vath::Clamp<fp32>(a_pColorData[pixelIndex / a_numChannels][channelIndex], 0.0f, 1.0f);
					pixels[pixelIndex + channelIndex] = static_cast<u8>(channelValue * 255.0f);
				}
				else
				{
					fp32 channelValue = vath::Clamp<fp32>(a_pColorData[pixelIndex / a_numChannels][channelIndex], 0.0f, 255.0f);
					pixels[pixelIndex + channelIndex] = static_cast<u8>(channelValue);
				}
			}

			pixelIndex += a_numChannels;
		}

		String fileName = String(a_fileName + ".png");
		stbi_write_png(fileName.c_str(), a_width, a_height, a_numChannels, pixels, a_width * a_numChannels);
		delete[] pixels;
	}
}