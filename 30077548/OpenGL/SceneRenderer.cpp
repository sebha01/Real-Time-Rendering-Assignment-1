#include "SceneRenderer.h"

SceneRenderer::SceneRenderer()
{
	//Render the camera ready for the source file to generate the window
 	camera = new Camera(camera_settings, glm::vec3(0.0f, 4.0f, 60.0f));
}

void SceneRenderer::Init()
{
	//Set the princple axes
	principleAxes = new PrincipleAxes();

	//Define which file the models need to use
	moonModel = new Sphere(32, 16, 1.0f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), CG_RIGHTHANDED);
	grass = new Model("Resources\\Models\\Grass\\grass.obj");
	rubik = new Model("Resources\\Models\\Cube\\Rubik Cube.obj");

	//Textures
	grassTexture = TextureLoader::loadTexture(string("Resources\\Models\\Grass\\grassTexture.jpg"));
	grass->attachTexture(grassTexture);

	rubikTexture = TextureLoader::loadTexture(string("Resources\\Models\\Cube Textures\\Robot-Skin.jpg"));
	rubik->attachTexture(rubikTexture);

	moonTexture = TextureLoader::loadTexture(string("Resources\\Models\\Moon_Textures\\Moon_Diffuse.jpg"));

	//PHONG SHADER
	GLSL_ERROR glsl_err = ShaderLoader::createShaderProgram(
		string("Resources\\Shaders\\Phong-texture.vs"),
		string("Resources\\Shaders\\Phong-texture.fs"),
		&phongShader);

	//BASIC SHADER
	glsl_err = ShaderLoader::createShaderProgram(
		string("Resources\\Shaders\\Basic_shader.vert"),
		string("Resources\\Shaders\\Basic_shader.frag"),
		&basicShader);

	//Setup uniform locations for shader
	textureUniformLoc = glGetUniformLocation(phongShader, "texture");

	modelMatrixLocation = glGetUniformLocation(phongShader, "modelMatrix");
	viewProjectionMatrixLocation = glGetUniformLocation(phongShader, "viewProjectionMatrix");
	invTransposeMatrixLocation = glGetUniformLocation(phongShader, "invTransposeModelMatrix");

	lightDirectionLocation = glGetUniformLocation(phongShader, "lightDirection");
	lightDiffuseLocation = glGetUniformLocation(phongShader, "lightDiffuseColour");
	lightSpecularLocation = glGetUniformLocation(phongShader, "lightSpecularColour");
	lightSpecExpLocation = glGetUniformLocation(phongShader, "lightSpecularExponent");

	cameraPosLocation = glGetUniformLocation(phongShader, "cameraPos");

	//Uniform Locations - Basic Shader////////////////////////////////////////////
	// Get unifom locations in shader
	uLightAmbient = glGetUniformLocation(basicShader, "lightAmbient");
	uLightDiffusers = glGetUniformLocation(basicShader, "lightDiffusers");
	uLightAttenuation = glGetUniformLocation(basicShader, "lightAttenuation");
	uLightPositions = glGetUniformLocation(basicShader, "lightPositions");
	uEyePos = glGetUniformLocation(basicShader, "eyePos");

	// Get material unifom locations in shader
	uMatAmbient = glGetUniformLocation(basicShader, "matAmbient");
	uMatDiffuse = glGetUniformLocation(basicShader, "matDiffuse");
	uMatSpecularCol = glGetUniformLocation(basicShader, "matSpecularColour");
	uMatSpecularExp = glGetUniformLocation(basicShader, "matSpecularExponent");
}

void SceneRenderer::Render()
{
	// Clear the screen
	glClearColor(0.1f, 0.2f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 grassModel = glm::mat4(1.0);
	glm::mat4 rubikModel = glm::mat4(1.0);
	glm::mat4 moon(1.0f);
	glm::mat4 view = camera->getViewMatrix();
	glm::mat4 projection = camera->getProjectionMatrix();
	glm::mat4 viewProjection = projection * view;

	glm::vec3 eyePos = camera->getCameraPosition();

	principleAxes->render(viewProjection);

	/////////////////////
	// GRASS MODEL
	/////////////////////
	grassModel = glm::translate(grassModel, glm::vec3(-10.0f, -10.0f, 0.0f));
	grassModel = glm::rotate(grassModel, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	grassModel = glm::scale(grassModel, glm::vec3(0.1f));

	glUseProgram(basicShader);

	glUniform4fv(uLightDiffusers, 5, (GLfloat*)&light_diffusers);
	glUniform4fv(uLightAmbient, 1, (GLfloat*)&light_ambient);
	glUniform4fv(uLightPositions, 5, (GLfloat*)&light_positions);
	glUniform3fv(uLightAttenuation, 1, (GLfloat*)&attenuation);
	glUniform3fv(uEyePos, 1, (GLfloat*)&eyePos);


	//Pass material data
	glUniform4fv(uMatAmbient, 1, (GLfloat*)&mat_amb_diff);
	glUniform4fv(uMatDiffuse, 1, (GLfloat*)&mat_amb_diff);
	glUniform4fv(uMatSpecularCol, 1, (GLfloat*)&mat_specularCol);
	glUniform1f(uMatSpecularExp, mat_specularExp);

	glUniformMatrix4fv(glGetUniformLocation(basicShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(basicShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	//Drawing the objects
	glUniformMatrix4fv(glGetUniformLocation(basicShader, "model"), 1, GL_FALSE, glm::value_ptr(grassModel));
	grass->draw(basicShader); //Draw the plane

	//////////////////
	// RUBIK CUBE
	//////////////////
	rubikModel = glm::translate(rubikModel, glm::vec3(-10.0f, 0.0f, 0.0f));
	//grassModel = glm::rotate(grassModel, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	rubikModel = glm::scale(rubikModel, glm::vec3(20.0f));

	glUniformMatrix4fv(glGetUniformLocation(basicShader, "model"), 1, GL_FALSE, glm::value_ptr(rubikModel));
	rubik->draw(basicShader);

	glUseProgram(0);


	/////////////
	// MOON MODEL
	/////////////
	moon = glm::translate(moon, glm::vec3(20.0f, 0.0f, -15.0f));
	moon = glm::scale(moon, glm::vec3(10.0f));

	moonTheta += 15.0f * float(timer.getDeltaTimeSeconds());

	if (moonModel)
	{
		// Modelling transform
		moon = glm::rotate(moon, glm::radians(23.44f), glm::vec3(0.0, 0.0, 1.0));//Moon tilt
		moon = glm::rotate(moon, glm::radians(moonTheta), glm::vec3(0.0, 1.0, 0.0));//Earth rotation

		// Calculate inverse transpose of the modelling transform for correct transformation of normal vectors
		glm::mat4 invT = glm::transpose(glm::inverse(moon));;

		glUseProgram(phongShader);

		//// Get the location of the camera in world coords and set the corresponding uniform in the shader
		glm::vec3 cameraPos = camera->getCameraPosition();
		glUniform3fv(cameraPosLocation, 1, (GLfloat*)&cameraPos);

		// Set the model, view and projection matrix uniforms (from the camera data obtained above)
		glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(moon));
		glUniformMatrix4fv(invTransposeMatrixLocation, 1, GL_FALSE, glm::value_ptr(invT));
		glUniformMatrix4fv(viewProjectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewProjection));

		//// Set the light direction uniform vector in world coordinates based on the Sun's position
		glUniform4f(lightDirectionLocation, cosf(glm::radians(sunTheta)), 0.0f, sinf(glm::radians(sunTheta)), 0.0f);
		glUniform4f(lightDiffuseLocation, 1.0f, 1.0f, 1.0f, 1.0f); // white diffuse light
		glUniform4f(lightSpecularLocation, 0.5f, 0.5f, 0.5f, 1.0f); // white specular light
		glUniform1f(lightSpecExpLocation, 10.0f); // specular exponent / falloff

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, moonTexture);

		moonModel->render();

		glUseProgram(0);
	}
}

void SceneRenderer::changeSunThetaValue(double value)
{
	sunTheta += value;
}

Camera_settings SceneRenderer::getCameraSettings()
{
	return camera_settings;
}

Timer& SceneRenderer::getTimer()
{
	return timer;
}

Camera* SceneRenderer::getCamera()
{
	return camera;
}

void SceneRenderer::setLastX(double x)
{
	this->lastX = x;
}

void SceneRenderer::setLastY(double y)
{
	this->lastY = y;
}

double SceneRenderer::getLastX()
{
	return lastX;
}

double SceneRenderer::getLastY()
{
	return lastY;
}