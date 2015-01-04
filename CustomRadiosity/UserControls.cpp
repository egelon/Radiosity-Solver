#include "UserControls.h"

#include <glfw3.h>
#include <ctime>

extern GLFWwindow* window;

UserControls::UserControls()
{
	position = glm::vec3( 0, 0, 5 ); 
	horizontalAngle = 3.14f;
	verticalAngle = 0.0f;
	initialFoV = 45.0f;
	speed = 3.0f;
	mouseSpeed = 0.0005f;
	windowWidth = 1024;
	windowHeight = 768;
	nearClippingPlane = 0.1f;
	farClippingPlane = 100.0f;
}

UserControls::UserControls(int wW, int wH, glm::vec3 pos, float hAngle, float vAngle, float initFoV, float nearClip, float farClip, float sp, float mSpeed) 
{
	position = pos;
	horizontalAngle = hAngle;
	verticalAngle = vAngle;
	initialFoV = initFoV;
	speed = sp;
	mouseSpeed = mSpeed;
	windowWidth = wW;
	windowHeight = wH;
	nearClippingPlane = nearClip;
	farClippingPlane = farClip;
}

glm::mat4 UserControls::getViewMatrix()
{
	return ViewMatrix;
}

glm::mat4 UserControls::getProjectionMatrix()
{
	return ProjectionMatrix;
}

void UserControls::computeMatricesFromInputs(Mesh* mesh, Radiosity* radiosity)
{
	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	glfwSetCursorPos(window, windowWidth/2, windowHeight/2);

	// Compute new orientation
	horizontalAngle += mouseSpeed * float(windowWidth/2 - xpos );
	verticalAngle   += mouseSpeed * float( windowHeight/2 - ypos );

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
			cos(verticalAngle) * sin(horizontalAngle), 
			sin(verticalAngle),
			cos(verticalAngle) * cos(horizontalAngle)
		);

	// Right vector
	glm::vec3 right = glm::vec3(
			sin(horizontalAngle - 3.14f/2.0f), 
			0,
			cos(horizontalAngle - 3.14f/2.0f)
		);

	// Up vector
	glm::vec3 up = glm::cross( right, direction );

	// Move forward
	if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS)
	{
		position += direction * deltaTime * speed;
	}
	// Move backward
	if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS)
	{
		position -= direction * deltaTime * speed;
	}
	// Strafe right
	if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS)
	{
		position += right * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS)
	{
		position -= right * deltaTime * speed;
	}

	//Subdivide
	if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS)
	{
		if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_RELEASE)
		{
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
			radiosity->loadSceneFacesFromMesh(mesh);
			radiosity->calculateRadiosityValues();
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
			mesh->OutputToBitmap(bmpName, windowWidth, windowHeight);
		}
	}
	
	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	if(windowHeight == 0)
		windowHeight = 1;
	float windowRatio = (float)windowWidth / (float)windowHeight;
	ProjectionMatrix = glm::perspective(initialFoV, windowRatio, nearClippingPlane, farClippingPlane);

	// Camera matrix
	ViewMatrix = glm::lookAt(
			position,           // Camera is here
			position+direction, // and looks here : at the same position, plus "direction"
			up                  // Head is up (set to 0,-1,0 to look upside-down)
		);

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}