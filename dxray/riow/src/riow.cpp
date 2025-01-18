#include "riow/riow.h"

#include <core/debug.h>
#include <core/fileSystem/stbIO.h>
#include <core/vath/vath.h>

using namespace dxray;

namespace RIOW
{
	
}

int main(int argc, char** argv)
{
	i32 imageWidth = 256;
	i32 imageHeight = 256;
	i32 imageChannels = 3; //RGB image, no alpha.

	const usize imagePixelCount = imageWidth * imageHeight;
	vath::Vector3* imageData = new vath::Vector3[imagePixelCount];

	if (imageData != nullptr)
	{
		for (i32 i = 0; i < imagePixelCount; i++)
		{
			imageData[i] = vath::Vector3(1.0f);
		}
	}

	StorePNG("riowOutput", imageWidth, imageHeight, imageChannels, static_cast<vath::Vector3f*>(imageData), true);
	delete[] imageData;
	return 0;
}