#ifndef VERTEX_H
#define VERTEX_H

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class Vertex
{
	public:
	Vertex(int i, const glm::vec3& pos)
	{ 
		position = pos;
		index = i;
	}

	virtual ~Vertex() { }


	int getIndex() const { return index; }
	double x() const { return position.x; }
	double y() const { return position.y; }
	double z() const { return position.z; }
	glm::vec3 getPosition() const { return position; }

	void set(glm::vec3 v)
	{
		position = v;
	}
	
	void set(double x, double y, double z)
	{
		position = glm::vec3(x, y, z);
	}

	private:
	Vertex() { assert(0); }
	Vertex& operator=(const Vertex&) { assert(0); }
	Vertex(const Vertex&) { assert(0); }

	int index;
	glm::vec3 position;
};

#endif