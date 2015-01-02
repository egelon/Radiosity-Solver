#ifndef USER_CONTROLS_H
#define USER_CONTROLS_H

#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
class UserControls
{
	public:
	UserControls();
	UserControls(int wW, int wH, glm::vec3 pos, float hAngle, float vAngle, float initFoV, float speed, float mouseSpeed);
	void computeMatricesFromInputs();
	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix();

	private:
	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;

	// Initial position : on +Z
	glm::vec3 position;
	// Initial horizontal angle : toward -Z
	float horizontalAngle;
	// Initial vertical angle : none
	float verticalAngle;
	// Initial Field of View
	float initialFoV;

	float speed; // units / second
	float mouseSpeed;

	int windowWidth;
	int windowHeight;
};

#endif