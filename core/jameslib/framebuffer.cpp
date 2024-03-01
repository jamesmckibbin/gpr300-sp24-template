#include "framebuffer.h"
#include <stdio.h>

jameslib::Framebuffer jameslib::createFramebuffer(unsigned int width, unsigned int height, int colorFormat)
{
	Framebuffer buffer;
	buffer.width = width;
	buffer.height = height;

	glCreateFramebuffers(1, &buffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, buffer.fbo);

	glGenTextures(1, buffer.colorBuffers);
	glBindTexture(GL_TEXTURE_2D, buffer.colorBuffers[0]);
	glTexStorage2D(GL_TEXTURE_2D, 1, colorFormat, width, height);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer.colorBuffers[0], 0);

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

jameslib::Framebuffer jameslib::createGBuffer(unsigned int width, unsigned int height)
{
	Framebuffer framebuffer;
	framebuffer.width = width;
	framebuffer.height = height;

	glCreateFramebuffers(1, &framebuffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

	int formats[3] = {
		GL_RGB32F, //0 = World Position 
		GL_RGB16F, //1 = World Normal
		GL_RGB16F  //2 = Albedo
	};

	for (size_t i = 0; i < 3; i++)
	{
		glGenTextures(1, &framebuffer.colorBuffers[i]);
		glBindTexture(GL_TEXTURE_2D, framebuffer.colorBuffers[i]);
		glTexStorage2D(GL_TEXTURE_2D, 1, formats[i], width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, framebuffer.colorBuffers[i], 0);
	}

	const GLenum drawBuffers[3] = {
			GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2
	};
	glDrawBuffers(3, drawBuffers);

	glGenTextures(1, &framebuffer.depthBuffer);
	glBindTexture(GL_TEXTURE_2D, framebuffer.depthBuffer);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, width, height);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebuffer.depthBuffer, 0);

	GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer incomplete: %d", fboStatus);
	}


	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return framebuffer;
}