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

#include "ArgParser.h"
#include "Mesh.h"
#include "Radiosity.h"
#include "UserControls.h"

int main( int argc, char *argv[] )
{
	ArgParser argParser(argc, argv);

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
	window = glfwCreateWindow( argParser.windowWidth, argParser.windowHeight, "Radiosity", NULL, NULL);

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

	glfwSetCursorPos(window, argParser.windowWidth/2, argParser.windowHeight/2);

	// Dark blue background
	glClearColor(argParser.bgcolor.r, argParser.bgcolor.g, argParser.bgcolor.b, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);

	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	UserControls userControls(
			argParser.cameraPosition,
			argParser.horizontalAngle,
			argParser.verticalAngle
		);
	Mesh* mesh = new Mesh();
	Radiosity* radiosity = new Radiosity();

	mesh->Load(argParser.sceneName);


	
	glShadeModel(GL_SMOOTH);
	// Create and compile our GLSL program from the mesh's shaders
	GLuint meshShaderProgramID = mesh->LoadDefaultShaders();
	mesh->cacheVerticesFacesAndColors();
	mesh->PrepareToDraw();

	printf("Loading faces...\n");
	radiosity->loadSceneFacesFromMesh(mesh);

	//now we draw
	do
	{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		userControls.handleKeyboard(mesh, radiosity);
		userControls.computeMatrices(argParser.initialFoV, argParser.nearClippingPlane, argParser.farClippingPlane, argParser.moveSpeed, argParser.mouseSpeed);
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