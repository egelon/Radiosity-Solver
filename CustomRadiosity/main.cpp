// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include<vector>
using namespace std;

#include "Mesh.h"
#include "Radiosity.h"
#include "UserControls.h"

int main( void )
{
	string sceneName = "test_quads.obj";

	int windowWidth = 1024;
	int windowHeight = 768;
	glm::vec3 cameraPosition( 0, 0, 5 );
	float horizontalAngle = 3.14f;
	float verticalAngle = 0.0f;
	float initialFoV = 45.0f;
	float nearClippingPlane = 0.1f;
	float farClippingPlane = 100.0f;
	float moveSpeed = 3.0f;
	float mouseSpeed = 0.0005f;

	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( windowWidth, windowHeight, "Radiosity", NULL, NULL);

	if( window == NULL )
	{
		fprintf( stderr, "Failed to open GLFW window.\n" );
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) 
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	glfwSetCursorPos(window, windowWidth/2, windowHeight/2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);

	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	UserControls userControls(
			windowWidth,
			windowHeight,
			cameraPosition,
			horizontalAngle,
			verticalAngle,
			initialFoV,
			nearClippingPlane,
			farClippingPlane,
			moveSpeed,
			mouseSpeed
		);
	Mesh* mesh = new Mesh();
	Radiosity* radiosity = new Radiosity();

	mesh->Load(sceneName);

	

	

	// Create and compile our GLSL program from the mesh's shaders
	GLuint meshShaderProgramID = mesh->LoadShaders();
	mesh->cacheVerticesFacesAndColors();
	mesh->PrepareToDraw();

	//now we draw
	do
	{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		userControls.computeMatricesFromInputs(mesh, radiosity);
		glm::mat4 ProjectionMatrix = userControls.getProjectionMatrix();
		glm::mat4 ViewMatrix = userControls.getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		mesh->SetMVP(MVP);

		//draw the mesh
		mesh->Draw();

		// Swap buffers
		glfwSwapBuffers(window);

		glfwPollEvents();

		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			printf("OpenGL error: %d\n",  err);
		}
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose(window) == 0 );

	// Cleanup mesh VBO
	mesh->Cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}