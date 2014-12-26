#ifndef VERTEX_H
#define VERTEX_H

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class Vertex
{
	public:
	Vertex(int i, const glm::vec3& pos)
	{ 
		position.x = pos.x;
		position.y = pos.y;
		position.z = pos.z;
		position.w = 1.0f;
		index = i;
	}

	virtual ~Vertex() { }


	int getIndex() const { return index; }
	double x() const { return position.x; }
	double y() const { return position.y; }
	double z() const { return position.z; }
	glm::vec4 getPositionVec4() const { return position; }
	glm::vec3 getPositionVec3() const { return glm::vec3(position.x, position.y, position.z); }

	void set(glm::vec3 v)
	{
		position.x = v.x;
		position.y = v.y;
		position.z = v.z;
		position.w = 1.0f;
	}
	
	void set(double x, double y, double z)
	{
		position.x = x;
		position.y = y;
		position.z = z;
		position.w = 1.0f;
	}

	private:
	Vertex() { assert(0); }
	Vertex& operator=(const Vertex&) { assert(0); }
	Vertex(const Vertex&) { assert(0); }

	int index;
	glm::vec4 position;
};

#endif