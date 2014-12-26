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

int main( void )
{

	Mesh* mesh = new Mesh();
	
	vector<vec4> file_vertices;
	vector<vec3> file_normals;
	vector<GLushort> file_elements;
	
	mesh->LoadToArrays("cube.obj", file_vertices, file_normals, file_elements);
	mesh->LoadFromArrays(file_vertices, file_normals, file_elements);



	//mesh->Subdivide();
	//mesh->Subdivide();
	

	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);


	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Radiosity", NULL, NULL);
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

	// Dark blue background
	glClearColor(1.0f, 1.0f, 0.5f, 1.0f);

	glm::mat4 view = glm::lookAt(
				glm::vec3(1.0,  3.0, 4.0),   // eye
				glm::vec3(0.0,  0.0, 0.0),   // direction
				glm::vec3(0.0,  1.0, 0.0));  // up

		glm::mat4 projection = glm::perspective(45.0f, 1.0f*500/500, 0.1f, 100.0f);

	do
	{
		// Draw nothing, see you in tutorial 2 !

		mesh->Draw();

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose(window) == 0 );

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	
	return 0;
}