#ifndef RAY_H
#define RAY_H

#include<glm/vec3.hpp>
#include <glm/glm.hpp>
#include<glm/gtx/vector_angle.hpp>
#include <glm/gtx/fast_square_root.hpp>
#include <glm/gtx/normal.hpp>
#include <glm/gtx/scalar_relational.hpp>


class Ray
{
public:
	Ray()
	{
		setStart(glm::vec3(0.0f, 0.0f, 0.0f));
		setDirection(glm::vec3(1.0f, 1.0f, 1.0f));
	}

	Ray(Ray& r)
	{
		setStart(r.getStart());
		setDirection(r.getDirection());
	}

	Ray(glm::vec3 s, glm::vec3 dir)
	{
		setStart(s);
		setDirection(dir);
	}

	Ray& operator=(Ray& r)
	{
		if(this != &r)
		{
			this->setStart(r.getStart());
			this->setDirection(r.getDirection());
		}
		return *this;
	}

	void setStart(glm::vec3 s) {start = s;}
	glm::vec3 getStart() {return start;}
	void setDirection(glm::vec3 dir) {direction = dir;}
	glm::vec3 getDirection() 
	{
		return glm::normalize(direction);
		//return direction;
	}
private:
	glm::vec3 start;
	glm::vec3 direction;
};

#endif