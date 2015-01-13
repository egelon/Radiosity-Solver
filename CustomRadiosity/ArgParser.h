#ifndef ARGPARSER_H
#define ARGPARSER_H

#include <stdio.h>
#include <string>
#include <glm\vec3.hpp>
using namespace std;

class ArgParser
{
public:
	ArgParser() 
	{
		DefaultValues();
	}

	ArgParser(int argc, char *argv[]) 
	{
		DefaultValues();

		for (int i = 1; i < argc; i++) 
		{
			if (!strcmp(argv[i],"-file")) 
			{
				i++;
				assert (i < argc); 
				sceneName = argv[i];
			}
			else if (!strcmp(argv[i],"-w")) 
			{
				i++;
				assert (i < argc);
				windowWidth = atoi(argv[i]);
			}
			else if (!strcmp(argv[i],"-h")) 
			{
				i++;
				assert (i < argc);
				windowHeight = atoi(argv[i]);
			}
			else if (!strcmp(argv[i],"-bgcolor")) 
			{
				i++;
				assert (i < argc);
				float r = glm::clamp((float)atof(argv[i]), 0.0f, 1.0f);

				i++;
				assert (i < argc);
				float g = glm::clamp((float)atof(argv[i]), 0.0f, 1.0f);

				i++;
				assert (i < argc);
				float b = glm::clamp((float)atof(argv[i]), 0.0f, 1.0f);

				bgcolor = glm::vec3(r, g, b);
			}
			else if (!strcmp(argv[i],"-camera")) 
			{
				i++;
				assert (i < argc);
				float x = atof(argv[i]);

				i++;
				assert (i < argc);
				float y = atof(argv[i]);

				i++;
				assert (i < argc);
				float z = atof(argv[i]);

				cameraPosition = glm::vec3(x, y, z);
			}
			else if (!strcmp(argv[i],"-ha")) 
			{
				i++;
				assert (i < argc);
				horizontalAngle = atof(argv[i]);
			}
			else if (!strcmp(argv[i],"-va")) 
			{
				i++;
				assert (i < argc);
				verticalAngle = atof(argv[i]);
			}
			else if (!strcmp(argv[i],"-fov")) 
			{
				i++;
				assert (i < argc);
				initialFoV = atof(argv[i]);
			}
			else if (!strcmp(argv[i],"-nclip")) 
			{
				i++;
				assert (i < argc);
				nearClippingPlane = atof(argv[i]);
			}
			else if (!strcmp(argv[i],"-fclip")) 
			{
				i++;
				assert (i < argc);
				farClippingPlane = atof(argv[i]);
			}
			else if (!strcmp(argv[i],"-mvs")) 
			{
				i++;
				assert (i < argc);
				moveSpeed = atof(argv[i]);
			}
			else if (!strcmp(argv[i],"-mmvs")) 
			{
				i++;
				assert (i < argc);
				mouseSpeed = atof(argv[i]);
			}
			else if (!strcmp(argv[i],"-i")) 
			{
				i++;
				assert (i < argc);
				numIterations = atoi(argv[i]);
			}
			else if (!strcmp(argv[i],"-interpolate")) 
			{
				interpolate = true;
			}
			else
			{
				printf("Error on command line argument %d: '%s'\n", i, argv[i]);
				assert(0);
			}
		}
	}
public:
	string sceneName;

	int windowWidth;
	int windowHeight;
	glm::vec3 cameraPosition;
	float horizontalAngle;
	float verticalAngle;
	float initialFoV;
	float nearClippingPlane;
	float farClippingPlane;
	float moveSpeed;
	float mouseSpeed;
	glm::vec3 bgcolor;
	bool interpolate;
	int numIterations;
private:
	void DefaultValues()
	{
		sceneName = "test.obj";
		windowWidth = 1024;
		windowHeight = 768;
		cameraPosition = glm::vec3( 0, 0, 5 );
		horizontalAngle = 3.14f;
		verticalAngle = 0.0f;
		initialFoV = 45.0f;
		nearClippingPlane = 0.1f;
		farClippingPlane = 100.0f;
		moveSpeed = 3.0f;
		mouseSpeed = 0.0005f;
		bgcolor = glm::vec3(0.0f, 0.0f, 0.4f);
		interpolate = false;
		numIterations = 1;
	}
};

#endif