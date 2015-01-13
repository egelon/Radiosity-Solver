#ifndef USER_CONTROLS_H
#define USER_CONTROLS_H

#include "Mesh.h"
#include "Radiosity.h"
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
class UserControls
{
	public:
	UserControls(glm::vec3 pos, float hAngle, float vAngle);
	void handleKeyboard(Mesh* mesh, Radiosity* radiosity);
	void computeMatrices(float initFoV, float nearClip, float farClip, float speed, float mouseSpeed);
	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix();

	private:
	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;

	// Initial position : on +Z
	glm::vec3 currentPosition;
	// Initial horizontal angle : toward -Z
	float currentHorizontalAngle;
	// Initial vertical angle : none
	float currentVerticalAngle;
};

#endif