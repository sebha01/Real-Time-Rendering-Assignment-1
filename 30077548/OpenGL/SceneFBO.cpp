#include "SceneFBO.h"

SceneFBO::SceneFBO(int windowWidth, int windowHeight, int scale)
{
	glGenFramebuffers(1, &demoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, demoFBO);

	// Setup colour buffer texture.
	// Note: The texture is stored as linear RGB values (GL_RGBA8). There is no need to
	//pass a pointer to image data - we're going to fill in the image when we render the
	//scene at render time!
	glGenTextures(1, &fboColourTexture);
	glBindTexture(GL_TEXTURE_2D, fboColourTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowWidth * scale, windowHeight * scale, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Setup depth texture

	glGenTextures(1, &fboDepthTexture);
	glBindTexture(GL_TEXTURE_2D, fboDepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, windowWidth * scale, windowHeight * scale, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	//
	// Attach textures to the FBO
	//

	// Attach the colour texture object to the framebuffer object's colour attachment point #0
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		fboColourTexture,
		0);

	// Attach the depth texture object to the framebuffer object's depth attachment point
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D,
		fboDepthTexture,
		0);


	//
	// Before proceeding make sure FBO can be used for rendering
	//

	GLenum demoFBOStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (demoFBOStatus != GL_FRAMEBUFFER_COMPLETE) {

		fboOkay = false;
		cout << "Could not successfully create framebuffer object to render texture!" << endl;

	}
	else {

		fboOkay = true;
		cout << "FBO successfully created" << endl;
	}

	// Unbind FBO for now! (Plug main framebuffer back in as rendering destination)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneFBO::BeginRender(int windowWidth, int windowHeight, int scale)
{
	if (!fboOkay)
		return; // Don't render anything if the FBO was not created successfully

	// Bind framebuffer object so all rendering redirected to attached images (i.e. our texture)
	glBindFramebuffer(GL_FRAMEBUFFER, demoFBO);

	// All rendering from this point goes to the bound textures (setup at initialisation time) and NOT the actual screen!!!!!

	// Clear the screen (i.e. the texture)
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set viewport to specified texture size (see above)
	glViewport(0, 0, windowWidth * scale, windowHeight * scale);

	//Render code to go next
}

void SceneFBO::EndRender(int windowWidth, int windowHeight)
{
	// Set OpenGL to render to the MAIN framebuffer (ie. the screen itself!!)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
}

GLuint SceneFBO::getSceneTexture() {

	return fboColourTexture;
}