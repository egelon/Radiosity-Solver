#include "ShaderLoader.h"

#include <stdio.h>
#include <vector>
#include <fstream>
#include <assert.h>

GLuint ShaderLoader::LoadShaders(string vertex_file, string fragment_file)
{
	// Create the shader IDs
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;

	std::ifstream VertexShaderStream(vertex_file, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		//Read the vertex shader line by line, and add the \n line ending
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	else
	{
		printf("Can't open Vertex Shader code file: %s !\n", vertex_file);
		assert(false);
	}


	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;

	std::ifstream FragmentShaderStream(fragment_file, std::ios::in);
	if(FragmentShaderStream.is_open())
	{
		//Read the fragment shader line by line, and add the \n line ending
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}
	else
	{
		printf("Can't open Fragment Shader code file: %s !\n", fragment_file);
		assert(false);
	}

	//Now we compile them
	GLint Result = GL_FALSE;
	int InfoLogLength = 0;

//Compile Vertex Shader
	printf("Compiling Vertex Shader : %s\n", vertex_file);

	//get the shader text as a pointer
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	//and pass it to OpenGL
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	//then compile and save in the VertexShaderID
	glCompileShader(VertexShaderID);

	//check the compiled vertex shader
	//write the compile status in Result
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	//write the log length
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);

	//now display the shader compilation output if any
	if ( InfoLogLength > 0 )
	{
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("Vertex Shader compilation output:\n");
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

//Compile Fragment Shader
	printf("Compiling Fragment Shader : %s\n", fragment_file);
	
	//get the shader text as a pointer
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	//and pass it to OpenGL
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	//then compile and save in the FragmentShaderID
	glCompileShader(FragmentShaderID);

	//check the compiled vertex shader
	//write the compile status in Result
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	//write the log length
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);

	//now display the shader compilation output if any
	if ( InfoLogLength > 0 )
	{
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("Fragment Shader compilation output:\n");
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	//Both shaders are compiled, now link the program
	printf("Linking program\n");

	GLuint ProgramID = glCreateProgram();

	//attach the compiled Vertex and Fragment shaders to the ProgramID
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	//now check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);

	if ( InfoLogLength > 0 )
	{
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("Program Linking output:\n");
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	//we are done with the shaders, it is safe to delete them
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}