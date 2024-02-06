#include "framebuffer.h"
#include <stdio.h>

jameslib::Framebuffer jameslib::createFramebuffer(unsigned int width, unsigned int height, int colorFormat)
{
	Framebuffer buffer;
	buffer.width = width;
	buffer.height = height;

	glCreateFramebuffers(1, &buffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, buffer.fbo);

	glGenTextures(1, &buffer.colorBuffer);
	glBindTexture(GL_TEXTURE_2D, buffer.colorBuffer);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, buffer.colorBuffer, 0);

	glGenTextures(1, &buffer.depthBuffer);
	glBindTexture(GL_TEXTURE_2D, buffer.depthBuffer);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, width, height);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.depthBuffer, 0);

	GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer incomplete: %d", fboStatus);
	}

	return buffer;
}