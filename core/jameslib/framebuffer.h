#pragma once

#include "../ew/external/glad.h"

namespace jameslib
{
	struct Framebuffer
	{
		unsigned int fbo;
		unsigned int colorBuffers[8];
		unsigned int depthBuffer;
		unsigned int width;
		unsigned int height;
	};

	Framebuffer createFramebuffer(unsigned int width, unsigned int height, int colorFormat);
	Framebuffer createGBuffer(unsigned int width, unsigned int height);
}