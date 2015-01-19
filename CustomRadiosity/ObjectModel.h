#ifndef SCENEMODEL_H
#define SCENEMODEL_H

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>
#include<glm/gtx/vector_angle.hpp>
#include <glm/gtx/fast_square_root.hpp>
#include <glm/gtx/normal.hpp>
#include <glm/gtx/scalar_relational.hpp>

#include <vector>
#include <string>

#include "ModelFace.h"

using namespace std;
struct ObjectModel
{
	vector<glm::vec3> vertices;
	vector<glm::vec3> textureUVW;
	vector<glm::vec3> vertexNormals;
	vector<ModelFace> faces;
	int vertexIndexOffset;

	float getFaceArea(int faceIndex)
	{
		ModelFace* face = &faces[faceIndex];
		if(faces[faceIndex].vertexIndexes.size() == 3) //we have triangles
		{
			//get the 3 vertices for the face
			int index_a = faces[faceIndex].vertexIndexes[0];
			int index_b = faces[faceIndex].vertexIndexes[1];
			int index_c = faces[faceIndex].vertexIndexes[2];

			glm::vec3 vertex_a = vertices[index_a];
			glm::vec3 vertex_b = vertices[index_b];
			glm::vec3 vertex_c = vertices[index_c];

			float a = glm::fastDistance(vertex_b, vertex_c);
			float b = glm::fastDistance(vertex_c, vertex_a);
			float c = glm::fastDistance(vertex_a, vertex_b);

			float s = (a + b + c) / 2.0f;

			return glm::fastSqrt(s * (s-a) * (s-b) * (s-c));
		}
		else if(faces[faceIndex].vertexIndexes.size() == 4) //we have quads
		{
			//get the 4 vertices for the face
			int index_a = faces[faceIndex].vertexIndexes[0];
			int index_b = faces[faceIndex].vertexIndexes[1];
			int index_c = faces[faceIndex].vertexIndexes[2];
			int index_d = faces[faceIndex].vertexIndexes[3];

			glm::vec3 vertex_a = vertices[index_a];
			glm::vec3 vertex_b = vertices[index_b];
			glm::vec3 vertex_c = vertices[index_c];
			glm::vec3 vertex_d = vertices[index_d];

			//divide quad into 2 triangles, quad area = S1 + S2
			float a = glm::fastDistance(vertex_a, vertex_b);
			float b = glm::fastDistance(vertex_b, vertex_c);
			float c = glm::fastDistance(vertex_c, vertex_d);
			float d = glm::fastDistance(vertex_d, vertex_a);

			float f = glm::fastDistance(vertex_d, vertex_b);

			float s1 = (a + b + f) / 2.0f;
			float s2 = (c + d + f) / 2.0f;

			float S1 = glm::fastSqrt(s1 * (s1-a) * (s1-b) * (s1-f));
			float S2 = glm::fastSqrt(s2 * (s2-c) * (s2-d) * (s2-f));

			return S1 + S2;
		}
		else return -1.0f;
	}

	glm::vec3 getFaceNormal(int faceIndex)
	{
		int index_a = faces[faceIndex].vertexIndexes[0];
		int index_b = faces[faceIndex].vertexIndexes[1];
		int index_c = faces[faceIndex].vertexIndexes[2];

		glm::vec3 vertex_a = vertices[index_a];
		glm::vec3 vertex_b = vertices[index_b];
		glm::vec3 vertex_c = vertices[index_c];

		//return glm::triangleNormal(vertex_a, vertex_b, vertex_c);
		return glm::normalize(glm::cross( (vertex_b - vertex_a), (vertex_c - vertex_a) ));
	}

	glm::vec3 getFaceCentroid(int faceIndex)
	{
		glm::vec3 centroid;
		if(faces[faceIndex].vertexIndexes.size() == 3) //we have triangles
		{
			//get the 3 vertices for the face
			int index_a = faces[faceIndex].vertexIndexes[0];
			int index_b = faces[faceIndex].vertexIndexes[1];
			int index_c = faces[faceIndex].vertexIndexes[2];

			glm::vec3 vertex_a = vertices[index_a];
			glm::vec3 vertex_b = vertices[index_b];
			glm::vec3 vertex_c = vertices[index_c];

			//calculate the medicenter of the face
			centroid = glm::vec3(
				(vertex_a.x + vertex_b.x + vertex_c.x) / 3.0f,
				(vertex_a.y + vertex_b.y + vertex_c.y) / 3.0f,
				(vertex_a.z + vertex_b.z + vertex_c.z) / 3.0f
				);
		}
		else if(faces[faceIndex].vertexIndexes.size() == 4) //we have quads
		{
			//get the 4 vertices for the face
			int index_a = faces[faceIndex].vertexIndexes[0];
			int index_b = faces[faceIndex].vertexIndexes[1];
			int index_c = faces[faceIndex].vertexIndexes[2];
			int index_d = faces[faceIndex].vertexIndexes[3];

			glm::vec3 vertex_a = vertices[index_a];
			glm::vec3 vertex_b = vertices[index_b];
			glm::vec3 vertex_c = vertices[index_c];
			glm::vec3 vertex_d = vertices[index_d];

			//calculate the medicenter of the face
			centroid = glm::vec3(
				(vertex_a.x + vertex_b.x + vertex_c.x + vertex_d.x) / 4.0f,
				(vertex_a.y + vertex_b.y + vertex_c.y + vertex_d.y) / 4.0f,
				(vertex_a.z + vertex_b.z + vertex_c.z + vertex_d.z) / 4.0f
				);
		}

		return centroid;
	}

	vector<glm::vec3> monteCarloSamplePoints(int faceIndex, int count)
	{
		//source: http://www.cs.princeton.edu/~funk/tog02.pdf
		//section 4.2
		vector<glm::vec3> result;
		
		if(faces[faceIndex].vertexIndexes.size() == 3) //we have triangles
		{
			//get the 3 vertices for the face
			int index_a = faces[faceIndex].vertexIndexes[0];
			int index_b = faces[faceIndex].vertexIndexes[1];
			int index_c = faces[faceIndex].vertexIndexes[2];

			glm::vec3 vertex_a = vertices[index_a];
			glm::vec3 vertex_b = vertices[index_b];
			glm::vec3 vertex_c = vertices[index_c];

			double r1;
			double r2;

			for(int i=0; i<count; i++)
			{
				r1 = static_cast <double> (rand()) / (static_cast <double> (RAND_MAX/1.0));
				r2 = static_cast <double> (rand()) / (static_cast <double> (RAND_MAX/1.0));
				glm::vec3 point(
						(float)(1.0 - glm::sqrt(r1)) * vertex_a +
						(float)(glm::sqrt(r1) * (1.0 - r2)) * vertex_b +
						(float)(r2 * glm::sqrt(r1)) * vertex_c
					);
				result.push_back(point);
			}
		}
		else if(faces[faceIndex].vertexIndexes.size() == 4) //we have quads
		{
			//get the 4 vertices for the face
			int index_a = faces[faceIndex].vertexIndexes[0];
			int index_b = faces[faceIndex].vertexIndexes[1];
			int index_c = faces[faceIndex].vertexIndexes[2];
			int index_d = faces[faceIndex].vertexIndexes[3];

			glm::vec3 vertex_a = vertices[index_a];
			glm::vec3 vertex_b = vertices[index_b];
			glm::vec3 vertex_c = vertices[index_c];
			glm::vec3 vertex_d = vertices[index_d];

			double r1;
			double r2;

			for(int i=0; i<count/2; i++)
			{
				r1 = static_cast <double> (rand()) / (static_cast <double> (RAND_MAX/1.0));
				r2 = static_cast <double> (rand()) / (static_cast <double> (RAND_MAX/1.0));
				glm::vec3 point1(
					(float)(1.0 - glm::sqrt(r1)) * vertex_a +
					(float)(glm::sqrt(r1) * (1.0 - r2)) * vertex_b +
					(float)(r2 * glm::sqrt(r1)) * vertex_d
					);
				result.push_back(point1);

				r1 = static_cast <double> (rand()) / (static_cast <double> (RAND_MAX/1.0));
				r2 = static_cast <double> (rand()) / (static_cast <double> (RAND_MAX/1.0));

				glm::vec3 point2(
					(float)(1.0 - glm::sqrt(r1)) * vertex_b +
					(float)(glm::sqrt(r1) * (1.0 - r2)) * vertex_c +
					(float)(r2 * glm::sqrt(r1)) * vertex_d
					);
				result.push_back(point2);
			}
		}

		return result;
	}
};

#endif