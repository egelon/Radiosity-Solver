#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include <glm/vec3.hpp>

class BoundingBox
{
	public:
	BoundingBox(glm::vec3 _min, glm::vec3 _max);
	BoundingBox(const BoundingBox& otherBB);

	BoundingBox* Clone() const;

	~BoundingBox();

	glm::vec3 getMin() const;
	glm::vec3 getMax() const;
	glm::vec3 getCenter() const;
	float maxDim() const;
	void Set(BoundingBox *bb);

	void Set(glm::vec3 _min, glm::vec3 _max);

	void Extend(const glm::vec3 v);

	void Extend(BoundingBox *bb);

	void Draw() const;

	private:
	BoundingBox() { assert(0); }

	glm::vec3 min;
	glm::vec3 max;
};

#endif