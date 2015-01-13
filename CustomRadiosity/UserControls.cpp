#include "UserControls.h"

#include <glfw/glfw3.h>
#include <ctime>

extern GLFWwindow* window;

UserControls::UserControls(glm::vec3 pos, float hAngle, float vAngle) 
{
	currentPosition = pos;
	currentHorizontalAngle = hAngle;
	currentVerticalAngle = vAngle;
}

glm::mat4 UserControls::getViewMatrix()
{
	return ViewMatrix;
}

glm::mat4 UserControls::getProjectionMatrix()
{
	return ProjectionMatrix;
}

void UserControls::handleKeyboard(Mesh* mesh, Radiosity* radiosity)
{
	//Subdivide
	if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS)
	{
		if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_RELEASE)
		{
			printf("Subdividing mesh...\n");
			mesh->Subdivide();
			mesh->cacheVerticesFacesAndColors();
			mesh->PrepareToDraw();
		}
	}

	//Reset
	if (glfwGetKey( window, GLFW_KEY_R ) == GLFW_PRESS)
	{
		if (glfwGetKey( window, GLFW_KEY_R ) == GLFW_RELEASE)
		{
			printf("Resetting mesh...\n");
			mesh->ResetMesh();
			mesh->cacheVerticesFacesAndColors();
			mesh->PrepareToDraw();
		}
	}

	//Radiosity
	if (glfwGetKey( window, GLFW_KEY_I ) == GLFW_PRESS)
	{
		if (glfwGetKey( window, GLFW_KEY_I ) == GLFW_RELEASE)
		{
			printf("Radiosity iteration. Please wait, this could take a while...\n");
			printf("Loading faces...\n");
			radiosity->loadSceneFacesFromMesh(mesh);
			printf("Calculating radiosity values...\n");
			radiosity->calculateRadiosityValues();
			printf("Preparing to re-draw scene...\n");
			radiosity->setMeshFaceColors();
			mesh->cacheVerticesFacesAndColors_Radiosity();
			mesh->PrepareToDraw();
		}
	}

	//dump to bitmap
	if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS)
	{
		if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_RELEASE)
		{
			time_t now = time(0);
			string bmpName(to_string(now).append(".bmp"));
			int windowWidth;
			int windowHeight;
			glfwGetWindowSize (window, &windowWidth, &windowHeight);
			mesh->OutputToBitmap(bmpName, windowWidth, windowHeight);
			printf("Screenshot saved: %s\n", bmpName);
		}
	}
}

void UserControls::computeMatrices(float initialFoV, float nearClippingPlane, float farClippingPlane, float speed, float mouseSpeed)
{
	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	//get current window size
	int windowWidth;
	int windowHeight;
	glfwGetWindowSize (window, &windowWidth, &windowHeight);

	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	glfwSetCursorPos(window, windowWidth/2, windowHeight/2);

	// Compute new orientation
	currentHorizontalAngle += mouseSpeed * float(windowWidth/2 - xpos );
	currentVerticalAngle   += mouseSpeed * float( windowHeight/2 - ypos );

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
			cos(currentVerticalAngle) * sin(currentHorizontalAngle), 
			sin(currentVerticalAngle),
			cos(currentVerticalAngle) * cos(currentHorizontalAngle)
		);

	// Right vector
	glm::vec3 right = glm::vec3(
			sin(currentHorizontalAngle - 3.14f/2.0f), 
			0,
			cos(currentHorizontalAngle - 3.14f/2.0f)
		);

	// Up vector
	glm::vec3 up = glm::cross( right, direction );

	//handle arrow keys here! The position vlaue is needed for the View matrix!
	// Move forward
	if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS)
	{
		currentPosition += direction * deltaTime * speed;
	}
	// Move backward
	if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS)
	{
		currentPosition -= direction * deltaTime * speed;
	}
	// Strafe right
	if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS)
	{
		currentPosition += right * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS)
	{
		currentPosition -= right * deltaTime * speed;
	}

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	if(windowHeight == 0)
		windowHeight = 1;
	float windowRatio = (float)windowWidth / (float)windowHeight;
	ProjectionMatrix = glm::perspective(initialFoV, windowRatio, nearClippingPlane, farClippingPlane);

	// Camera matrix
	ViewMatrix = glm::lookAt(
			currentPosition,			// Camera is here
			currentPosition+direction,	// and looks here : at the same position, plus "direction"
			up							// Head is up (set to 0,-1,0 to look upside-down)
		);

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}