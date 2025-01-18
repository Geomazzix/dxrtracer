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
		u8* pixels = new u8[a_width * a_height * a_numChannels];
	
		for (i32 pixelIndex = 0, j = 0; j < a_height; j++)
		{
			for (i32 i = 0; i < a_width; i++)
			{
				for (i32 channelIndex = 0; channelIndex < a_numChannels; channelIndex++)
				{
					if (a_bNormalizedData)
					{
						fp32 channelValue = vath::Clamp<fp32>(a_pColorData[pixelIndex / 3][channelIndex], 0.0f, 1.0f);
						pixels[pixelIndex + channelIndex] = static_cast<u8>(channelValue * 255.0f);
					}
					else
					{
						fp32 channelValue = vath::Clamp<fp32>(a_pColorData[pixelIndex / 3][channelIndex], 0.0f, 255.0f);
						pixels[pixelIndex + channelIndex] = static_cast<u8>(channelValue);
					}
				}
	
				pixelIndex += a_numChannels;
			}
		}

		String fileName = String(a_fileName + ".png");
		stbi_write_png(fileName.c_str(), a_width, a_height, a_numChannels, pixels, a_width * a_bNormalizedData);
		delete[] pixels;
	}
}