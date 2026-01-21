#include "Includes.h"
#include "SceneFBO.h"
#include "SceneRenderer.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// 
// TO DO 
// SETUP SCENE RENDERER CLASS
// INCORPORATE SCENE RENDERER TO REMOVE BULK OF CODE IN SOURCE
// CREATE TEXTURED QUAD READY FOR FBO CLASS TO RENDER THE SCENE
// APPLY SCENE TO QUAD WHEN RENDERING AND HAVE QUAD FILL THE SCREEN
// YOU HAVE A SCREENSHOT ON YOUR PHONE FOR THE FULLSCREEN STUFF
// 
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

int	currentAntiAliasingFilter = 0;
static const char* filterStrings[] = {
		"No Anti-Aliasing",
		"Multi-Sample Anti-Aliasing x4",
		"Super-Sample Anti-Aliasing"
};
bool SSAA_enabled = false;

//Boolean to capture first mouse input to prevent snapback bug
bool firstMouseInput = false;

//Initialise componenets needed to render scene for non AA, MSAA and SSAA scene
//SceneFBO sceneFBO;
SceneRenderer* sceneRenderer;
TexturedQuad *superSamplingScene = nullptr;

int main()
{
	//Initialise the scene Renderer object
	sceneRenderer = new SceneRenderer();

	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//Got from https://www.youtube.com/watch?v=oHVh8htoGKw
	glfwWindowHint(GLFW_SAMPLES, 4);

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(sceneRenderer->getCameraSettings().screenWidth, sceneRenderer->getCameraSettings().screenHeight, "30077548 Anti-Aliasing assignment", NULL, NULL);
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

	sceneRenderer->Init();
	SceneFBO sceneFBO;
	superSamplingScene = new TexturedQuad(sceneFBO.getSceneTexture(), true);

	//Rendering settings
	glfwSwapInterval(0);		// glfw enable swap interval to match screen v-sync 1 for vsync 0 for off
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	TextRenderer textRenderer(width, height);
	
	currentAntiAliasingFilter = 0;
	
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
				SSAA_enabled = false;
				break;
			case 1:
				glEnable(GL_MULTISAMPLE);
				SSAA_enabled = false;
				break;
			case 2:
				glDisable(GL_MULTISAMPLE);
				SSAA_enabled = true;
				//Frame buffer automatically swapped within and reset back to main swap chain
				break;
			default:
				glDisable(GL_MULTISAMPLE);
				SSAA_enabled = false;
				break;
		}

		// input
		processInput(window);
		sceneRenderer->getTimer().tick();

		if (SSAA_enabled)
		{
			sceneFBO.BeginRender();
			sceneRenderer->Render();
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

			//Enable blending to render the text otherwise renders as a block form
			// disable after text rendered so that it doesn't affect the other models being rendered
			glEnable(GL_BLEND);
			textRenderer.renderText(filterStrings[currentAntiAliasingFilter], 15.0f, 15.0f, 1.0f, glm::vec3(1.0, 1.0f, 1.0f));
			glDisable(GL_BLEND);

			sceneFBO.EndRender(sceneRenderer->getCameraSettings().screenWidth, sceneRenderer->getCameraSettings().screenHeight);

			glDisable(GL_DEPTH_TEST);
			superSamplingScene->render(glm::mat4(1.0));
			glEnable(GL_DEPTH_TEST);

		}
		else
		{
			sceneRenderer->Render();

			//Enable blending to render the text otherwise renders as a block form
			// disable after text rendered so that it doesn't affect the other models being rendered
			glEnable(GL_BLEND);
			textRenderer.renderText(filterStrings[currentAntiAliasingFilter], 15.0f, 15.0f, 1.0f, glm::vec3(1.0, 1.0f, 1.0f));
			glDisable(GL_BLEND);
		}

		// glfw: swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window)
{
	sceneRenderer->getTimer().updateDeltaTime();

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		sceneRenderer->getCamera()->processKeyboard(FORWARD, sceneRenderer->getTimer().getDeltaTimeSeconds());
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		sceneRenderer->getCamera()->processKeyboard(BACKWARD, sceneRenderer->getTimer().getDeltaTimeSeconds());
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		sceneRenderer->getCamera()->processKeyboard(LEFT, sceneRenderer->getTimer().getDeltaTimeSeconds());
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		sceneRenderer->getCamera()->processKeyboard(RIGHT, sceneRenderer->getTimer().getDeltaTimeSeconds());
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		sceneRenderer->getCamera()->setRunSpeed(3.0);
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
		sceneRenderer->getCamera()->setRunSpeed(1.0);

	//Rotate light direction on the moon
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		sceneRenderer->changeSunThetaValue(-0.1f);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		sceneRenderer->changeSunThetaValue(0.1f);

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
	sceneRenderer->getCamera()->updateScreenSize(width, height);
}

// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	//Ensure that camera does not snap due to the initial large offset of where lastX and 
	// lastY have not yet matched the mouse position the first time the mousecallback 
	// function is called
	if (firstMouseInput)
	{
		sceneRenderer->setLastX(xpos);
		sceneRenderer->setLastY(ypos);
		firstMouseInput = false;
	}

	double xoffset = xpos - sceneRenderer->getLastX();
	double yoffset = sceneRenderer->getLastY() - ypos; // reversed since y-coordinates go from bottom to top

	sceneRenderer->setLastX(xpos);
	sceneRenderer->setLastY(ypos);

	sceneRenderer->getCamera()->processMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	sceneRenderer->getCamera()->processMouseScroll(yoffset);
}