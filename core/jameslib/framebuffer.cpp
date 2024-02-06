#include "framebuffer.h"

jameslib::Framebuffer jameslib::createFramebuffer(unsigned int width, unsigned int height, int colorFormat)
{
	Framebuffer newFrameBuffer;
	newFrameBuffer.width = width;
	newFrameBuffer.height = height;

	return newFrameBuffer;
}