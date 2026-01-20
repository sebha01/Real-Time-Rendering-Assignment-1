#pragma once
#include "Includes.h"
#include <array>

class SceneRenderer
{
	private:
		//// Shaders ////
		GLuint phongShader;
		GLuint basicShader;

		// Models // 
		Sphere* moonModel = nullptr;
		Model* grass = nullptr;
		Model* rubik = nullptr;
	
		// Textures //
		GLuint grassTexture;
		GLuint rubikTexture;
		GLuint moonTexture;

		//Light Data///
		// Lights
		GLfloat light_ambient[4] = { 1.0, 1.0, 1.0, 1.0 };	// Dim light 
		GLfloat light_diffusers[20] = {
			1.0, 0.0, 0.0, 1.0, // Red
			0.0, 1.0, 0.0, 1.0, // Green
			1.0, 1.0, 0.0, 1.0, //Yellow
			0.0, 0.0, 1.0, 1.0, //Blue
			1.0, 1.0, 1.0, 1.0 //White light above tower
		};	// White main light 
		GLfloat light_positions[20] = {
			27.0, 5.0, 18.0, 1.0, //Red Light
			-27.0, 5.0, 18.0, 1.0, //Green
			-27.0, 5.0, -18.0, 1.0, //Yellow
			27.0, 5.0, -18.0, 1.0, // Blue
			0.0, 25.0, 0.0, 1.0 // White light above tower
		};	// Point light (w=1.0)
		GLfloat	attenuation[3] = {1.0, 0.10, 0.08};

		// Materials
		GLfloat mat_amb_diff[4] = {1.0, 1.0, 1.0, 1.0}; // Texture map will provide ambient and diffuse.
		GLfloat mat_specularCol[4] = {1.0, 1.0, 1.0, 1.0}; // White highlight
		GLfloat mat_specularExp = 32.0;					// Shiny surface

		// Uniform variables //
		GLuint textureUniformLoc;
		GLint modelMatrixLocation;
		GLint viewProjectionMatrixLocation;
		GLint invTransposeMatrixLocation;

		GLint lightDirectionLocation;
		GLint lightDiffuseLocation;
		GLint lightSpecularLocation;
		GLint lightSpecExpLocation;

		GLint cameraPosLocation;

		//Uniform Locations - Basic Shader////////////////////////////////////////////
		// Get unifom locations in shader
		GLuint uLightAmbient;
		GLuint uLightDiffusers;
		GLuint uLightAttenuation;
		GLuint uLightPositions;
		GLuint uEyePos;

		// Get material unifom locations in shader
		GLuint uMatAmbient;
		GLuint uMatDiffuse;
		GLuint uMatSpecularCol;
		GLuint uMatSpecularExp;

		// Other variables //
		float moonTheta = 0.0f;
		float sunTheta = 0.0f;

		PrincipleAxes* principleAxes = nullptr;

		//Variables to be passed in from source.cpp
		unsigned int w = 1500;
		unsigned int h = 1200;

		// Camera settings
		//							  width, heigh, near plane, far plane
		Camera_settings camera_settings{ w, h, 0.1, 1500.0 };

		//Timer
		Timer timer;

		// Instantiate the camera object with basic data
		Camera* camera;

		double lastX = camera_settings.screenWidth / 2.0f;
		double lastY = camera_settings.screenHeight / 2.0f;
	public:
		SceneRenderer();
		void Init();
		void Render();
		void changeSunThetaValue(double value);
		//Getters
		Camera_settings getCameraSettings();
		Timer& getTimer();
		Camera* getCamera();
		void setLastX(double x);
		void setLastY(double y);
		double getLastX();
		double getLastY();
};