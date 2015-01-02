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
#include "UserControls.h"

int main( void )
{
	int windowWidth = 1024;
	int windowHeight = 768;

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

	//glDisable(GL_BLEND);
	//glDisable(GL_POLYGON_SMOOTH);

	//glShadeModel(GL_FLAT);

	UserControls userControls(windowWidth, windowHeight, glm::vec3( 0, 0, 5 ), 3.14f, 0.0f, 45.0f, 3.0f, 0.0005f);

	Mesh* mesh = new Mesh();

	mesh->Load("test.obj");

	//mesh->Subdivide();
	//mesh->Subdivide();
	//mesh->Subdivide();

	// Create and compile our GLSL program from the mesh's shaders
	GLuint meshShaderProgramID = mesh->LoadShaders();

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
//	glm::mat4 Projection = glm::perspective(
//								45.0f,
//								4.0f / 3.0f,
//								0.1f,
//								100.0f
//	);
	// Or, for an ortho camera :
	//glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

	// Camera matrix
//	glm::mat4 View = glm::lookAt(
//						glm::vec3(0,0,4), // Camera is at (4,3,3), in World Space
//						glm::vec3(0,0,0), // and looks at the origin
//						glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
//					);
	
	// Model matrix : an identity matrix (model will be at the origin)
//	glm::mat4 Model = glm::mat4(1.0f); // Changes for each model !

	// Our ModelViewProjection : multiplication of our 3 matrices
//	glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around
	
	mesh->PrepareToDraw();

	//now we draw
	do
	{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		userControls.computeMatricesFromInputs();
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