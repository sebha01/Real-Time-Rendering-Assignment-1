#include "Includes.h"

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);


// Camera settings
//							  width, heigh, near plane, far plane
Camera_settings camera_settings{ 1500, 1200, 0.1, 150.0 };

//Timer
Timer timer;

// Instantiate the camera object with basic data
Camera camera(camera_settings, glm::vec3(0.0f, 4.0f, 60.0f));

double lastX = camera_settings.screenWidth / 2.0f;
double lastY = camera_settings.screenHeight / 2.0f;

int	currentRoad;

//Boolean to capture first mouse input to prevent snapback bug
bool firstMouseInput = false;

int main()
{
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


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
	glfwSwapInterval(1);		// glfw enable swap interval to match screen v-sync
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE); //Enables face culling
	glFrontFace(GL_CCW);//Specifies which winding order if front facing
	glEnable(GL_BLEND);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	TextRenderer textRenderer(width, height);

	PrincipleAxes	*principleAxes = new PrincipleAxes();

	////	Shaders - Textures - Models	////

	static const GLuint	NUM_ROADS = 6;
	TexturedQuad	*road[NUM_ROADS];

	//
	// Load example road texture with different filtering properites

	// Point filtering
	road[0] = new TexturedQuad("Resources\\Models\\road.bmp", TextureGenProperties(GL_COMPRESSED_SRGB, GL_NEAREST, GL_NEAREST, 1.0f, GL_REPEAT, GL_REPEAT, true));

	// Bilinear filtering
	road[1] = new TexturedQuad("Resources\\Models\\road.bmp", TextureGenProperties(GL_SRGB8_ALPHA8, GL_LINEAR, GL_LINEAR, 1.0f, GL_REPEAT, GL_REPEAT, true));

	// Tri-linear filtering
	road[2] = new TexturedQuad("Resources\\Models\\road.bmp", TextureGenProperties(GL_SRGB8_ALPHA8, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, 1.0f, GL_REPEAT, GL_REPEAT, true));

	// Anisotropic x2
	road[3] = new TexturedQuad("Resources\\Models\\road.bmp", TextureGenProperties(GL_SRGB8_ALPHA8, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, 2.0f, GL_REPEAT, GL_REPEAT, true));

	// Anisotropic x8
	road[4] = new TexturedQuad("Resources\\Models\\road.bmp", TextureGenProperties(GL_SRGB8_ALPHA8, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, 8.0f, GL_REPEAT, GL_REPEAT, true));

	// Anisotropic x16
	road[5] = new TexturedQuad("Resources\\Models\\road.bmp", TextureGenProperties(GL_SRGB8_ALPHA8, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, 16.0f, GL_REPEAT, GL_REPEAT, true));

	currentRoad = 0;


	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// input
		processInput(window);
		timer.tick();
	

		// Clear the screen
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glm::mat4 model = glm::rotate(glm::mat4(1.0), glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
		model = glm::scale(model, glm::vec3(16.0f, 64.0f, 1.0f));
		model = glm::translate(model, glm::vec3(-1.0f, -1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 1.0f));

		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 projection = camera.getProjectionMatrix();
		glm::mat4 viewProjection = projection * view;

		glm::mat4 roadMVP = viewProjection * model;

		road[currentRoad]->render(roadMVP);

		static const char *filterStrings[] = {
		"Point filtering",
		"Bi-linear filtering",
		"Tri-linear filtering",
		"Anisotropic filtering 2x",
		"Anisotropic filtering 8x",
		"Anisotropic filtering 16x" };
		textRenderer.renderText(filterStrings[currentRoad], 25.0f, 25.0f, 1.0f, glm::vec3(1.0, 1.0f, 1.0f));

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

	//Switch the texture filter modes
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		currentRoad = 0;	
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		currentRoad = 1;
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		currentRoad = 2;
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		currentRoad = 3;
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
		currentRoad = 4;
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
		currentRoad = 5;
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