#pragma once
#include "Includes.h"
#include "SceneRenderer.h"

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

		SceneRenderer* sceneRenderer;
	public:
		SceneFBO();
		void BeginRender();
		void EndRender();
		GLuint getSceneTexture();
};