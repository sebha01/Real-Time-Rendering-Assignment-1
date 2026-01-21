#pragma once
#include "Includes.h"

class SceneFBO
{
	private:
		//FBO variables
		//Actual FBO
		GLuint demoFBO;

		//Colour texture to render into
		GLuint fboColourTexture;

		//Depth texture to render into
		GLuint fboDepthTexture;

		//Flag to indicate that the FBO is valid
		bool fboOkay;
	public:
		SceneFBO(int windowWidth, int windowHeight, int scale);
		void BeginRender(int windowWidth, int windowHeight, int scale);
		void EndRender(int windowWidth, int windowHeight);
		GLuint getSceneTexture();
};