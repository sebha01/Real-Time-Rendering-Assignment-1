#include "Includes.h"

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);


// Camera settings
//							  width, heigh, near plane, far plane
Camera_settings camera_settings{ 1500, 1200, 0.1, 1500.0 };

//Timer
Timer timer;

// Instantiate the camera object with basic data
Camera camera(camera_settings, glm::vec3(0.0f, 4.0f, 60.0f));

double lastX = camera_settings.screenWidth / 2.0f;
double lastY = camera_settings.screenHeight / 2.0f;

int	currentAntiAliasingFilter = 0;
static const char* filterStrings[] = {
		"No Anti-Aliasing",
		"Multi-Sample Anti-Aliasing x4",
		"Super-Sample Anti-Aliasing"
};

//Boolean to capture first mouse input to prevent snapback bug
bool firstMouseInput = false;

Sphere* moonModel = nullptr;
float moonTheta = 0.0f;
float sunTheta = 0.0f;

int main()
{
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(camera_settings.screenWidth, camera_settings.screenHeight, "30077548 Anti-Aliasing assignment", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Set the callback functions
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//Set firstMouseInput to true so camera does not snap to mouse position
	firstMouseInput = true;

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//Rendering settings
	glfwSwapInterval(0);		// glfw enable swap interval to match screen v-sync 1 for vsync 0 for off
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	TextRenderer textRenderer(width, height);

	PrincipleAxes	*principleAxes = new PrincipleAxes();

	///////////////////////////////////////////////////////////////////////////
	////	Shaders - Textures - Models	////

	GLuint phongShader;
	GLuint basicShader;
	
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

	//Grass
	Model grass("Resources\\Models\\Grass\\grass.obj");
	GLuint grassTexture = TextureLoader::loadTexture(string("Resources\\Models\\Grass\\grassTexture.jpg"));
	grass.attachTexture(grassTexture);

	//Rubik's cube
	Model rubik("Resources\\Models\\Cube\\Rubik Cube.obj");
	GLuint rubikTexture = TextureLoader::loadTexture(string("Resources\\Models\\Cube Textures\\Robot-Skin.jpg"));
	rubik.attachTexture(rubikTexture);

	//Moon
	moonModel = new Sphere(32, 16, 1.0f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), CG_RIGHTHANDED);
	GLuint moonTexture = TextureLoader::loadTexture(string("Resources\\Models\\Moon_Textures\\Moon_Diffuse.jpg"));
	
	currentAntiAliasingFilter = 0;
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	//Light Data///////////////////////////////////////////////
	// Lights
	GLfloat light_ambient[] = { 1.0, 1.0, 1.0, 1.0 };	// Dim light 
	GLfloat light_diffusers[] = {
		1.0, 0.0, 0.0, 1.0, // Red
		0.0, 1.0, 0.0, 1.0, // Green
		1.0, 1.0, 0.0, 1.0, //Yellow
		0.0, 0.0, 1.0, 1.0, //Blue
		1.0, 1.0, 1.0, 1.0 //White light above tower
	};	// White main light 
	GLfloat light_positions[] = {
		27.0, 5.0, 18.0, 1.0, //Red Light
		-27.0, 5.0, 18.0, 1.0, //Green
		-27.0, 5.0, -18.0, 1.0, //Yellow
		27.0, 5.0, -18.0, 1.0, // Blue
		0.0, 25.0, 0.0, 1.0 // White light above tower
	};	// Point light (w=1.0)
	GLfloat	attenuation[] = { 1.0, 0.10, 0.08 };

	// Materials
	GLfloat mat_amb_diff[] = { 1.0, 1.0, 1.0, 1.0 };	// Texture map will provide ambient and diffuse.
	GLfloat mat_specularCol[] = { 1.0, 1.0, 1.0, 1.0 }; // White highlight
	GLfloat mat_specularExp = 32.0;					// Shiny surface

	//Setup uniform locations for shader
	GLuint textureUniformLoc = glGetUniformLocation(phongShader, "texture");

	GLint modelMatrixLocation = glGetUniformLocation(phongShader, "modelMatrix");
	GLint viewProjectionMatrixLocation = glGetUniformLocation(phongShader, "viewProjectionMatrix");
	GLint invTransposeMatrixLocation = glGetUniformLocation(phongShader, "invTransposeModelMatrix");

	GLint lightDirectionLocation = glGetUniformLocation(phongShader, "lightDirection");
	GLint lightDiffuseLocation = glGetUniformLocation(phongShader, "lightDiffuseColour");
	GLint lightSpecularLocation = glGetUniformLocation(phongShader, "lightSpecularColour");
	GLint lightSpecExpLocation = glGetUniformLocation(phongShader, "lightSpecularExponent");

	GLint cameraPosLocation = glGetUniformLocation(phongShader, "cameraPos");
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	//Uniform Locations - Basic Shader////////////////////////////////////////////
	// Get unifom locations in shader
	GLuint uLightAmbient = glGetUniformLocation(basicShader, "lightAmbient");
	GLuint uLightDiffusers = glGetUniformLocation(basicShader, "lightDiffusers");
	GLuint uLightAttenuation = glGetUniformLocation(basicShader, "lightAttenuation");
	GLuint uLightPositions = glGetUniformLocation(basicShader, "lightPositions");
	GLuint uEyePos = glGetUniformLocation(basicShader, "eyePos");

	// Get material unifom locations in shader
	GLuint uMatAmbient = glGetUniformLocation(basicShader, "matAmbient");
	GLuint uMatDiffuse = glGetUniformLocation(basicShader, "matDiffuse");
	GLuint uMatSpecularCol = glGetUniformLocation(basicShader, "matSpecularColour");
	GLuint uMatSpecularExp = glGetUniformLocation(basicShader, "matSpecularExponent");
	///////////////////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////////////////
	//FPS Counter variables
	//Video used to create FPS counter -> https://www.youtube.com/watch?v=BA6aR_5C_BM 
	double prevTime = 0.0;
	double crntTime = 0.0;
	double timeDiff = 0.0;
	unsigned int counter = 0;
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// render loop
	while (!glfwWindowShouldClose(window))
	{
		//FPS counter
		crntTime = glfwGetTime();
		timeDiff = crntTime - prevTime;
		counter++;

		if (timeDiff >= 1.0)
		{
			//initialised title variable
			std::stringstream title;

			//Calculate the fps and ms per frame
			double FPS = (1.0 / timeDiff) * counter;
			double ms = (timeDiff / counter) * 1000;

			//set the title to show the fps and ms
			title << std::fixed << std::setprecision(2) << "30077548 Anti-Aliasing assignment\tFPS:"
				<< FPS << "\tMs: " << ms;

			//Set the new window title
			glfwSetWindowTitle(window, title.str().c_str());

			//set previous time to current time and reset counter
			prevTime = crntTime;
			counter = 0;
		}

		//Change aliasing effects, learnt how to do MSAA from https://www.youtube.com/watch?v=oHVh8htoGKw
		//Used a switch statement to change between effects.
		switch (currentAntiAliasingFilter)
		{
			case 0:
				glDisable(GL_MULTISAMPLE);
				break;
			case 1:
				glEnable(GL_MULTISAMPLE);
				break;
			deafult:
				glDisable(GL_MULTISAMPLE);
				break;
		}

		// input
		processInput(window);
		timer.tick();

		// Clear the screen
		glClearColor(0.1f, 0.2f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 grassModel = glm::mat4(1.0);
		glm::mat4 rubikModel = glm::mat4(1.0);
		glm::mat4 moon(1.0f);
		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 projection = camera.getProjectionMatrix();
		glm::mat4 viewProjection = projection * view;
		
		glm::vec3 eyePos = camera.getCameraPosition();

		principleAxes->render(viewProjection);

		///////////////////////////////////////////////////////////////////////////
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
		grass.draw(basicShader); //Draw the plane
		///////////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////////
		//////////////////
		// RUBIK CUBE
		//////////////////
		rubikModel = glm::translate(rubikModel, glm::vec3(-10.0f, 0.0f, 0.0f));
		//grassModel = glm::rotate(grassModel, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		rubikModel = glm::scale(rubikModel, glm::vec3(20.0f));

		glUniformMatrix4fv(glGetUniformLocation(basicShader, "model"), 1, GL_FALSE, glm::value_ptr(rubikModel));
		rubik.draw(basicShader);
		
		glUseProgram(0);
		///////////////////////////////////////////////////////////////////////////
		

		///////////////////////////////////////////////////////////////////////////
		// MOON MODEL
		///////////////////////////////////////////////////////////////////////////
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
			glm::vec3 cameraPos = camera.getCameraPosition();
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
		///////////////////////////////////////////////////////////////////////////

		//Enable blending to render the text otherwise renders as a block form
		// disable after text rendered so that it doesn't affect the other models being rendered
		glEnable(GL_BLEND);
		textRenderer.renderText(filterStrings[currentAntiAliasingFilter], 15.0f, 15.0f, 1.0f, glm::vec3(1.0, 1.0f, 1.0f));
		glDisable(GL_BLEND);

		// glfw: swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
	timer.updateDeltaTime();

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.processKeyboard(FORWARD, timer.getDeltaTimeSeconds());
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.processKeyboard(BACKWARD, timer.getDeltaTimeSeconds());
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.processKeyboard(LEFT, timer.getDeltaTimeSeconds());
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.processKeyboard(RIGHT, timer.getDeltaTimeSeconds());
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.setRunSpeed(3.0);
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
		camera.setRunSpeed(1.0);

	//Rotate light direction on the moon
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		sunTheta = sunTheta - 0.1f;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		sunTheta = sunTheta + 0.1f;


	//Switch the texture filter modes
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		currentAntiAliasingFilter = 0;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		currentAntiAliasingFilter = 1;
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		currentAntiAliasingFilter = 2;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	glViewport(0, 0, width, height);
	camera.updateScreenSize(width, height);
}

// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	//Ensure that camera does not snap due to the initial large offset of where lastX and 
	// lastY have not yet matched the mouse position the first time the mousecallback 
	// function is called
	if (firstMouseInput) 
	{ 
		lastX = xpos; 
		lastY = ypos; 
		firstMouseInput = false; 
	}

	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.processMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.processMouseScroll(yoffset);
}