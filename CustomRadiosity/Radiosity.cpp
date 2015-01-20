#include "Radiosity.h"
#include <algorithm>

#define INITIAL_LIGHT_EMITTER_INTENSITY		5.0f
#define INITIAL_AMBIENT_INTENSITY			glm::vec3(0.03f, 0.03f, 0.03f)

#define RADIOSITY_SOLUTION_THRESHOLD		glm::vec3(0.25f, 0.25f, 0.25f)
#define FORM_FACTOR_SAMPLES					15

void Radiosity::loadSceneFacesFromMesh(Mesh* mesh)
{
	sceneFaces.clear();
	formFactors.clear();

	for(int i=0; i<mesh->sceneModel.size(); i++)
	{
		//for every scene object
		ObjectModel* currentObject = &mesh->sceneModel[i].obj_model;

		int currentObjFaces = currentObject->faces.size();

		for(int j=0; j < currentObjFaces; j++)
		{
			//for every face of it
			ModelFace* currentFace = &mesh->sceneModel[i].obj_model.faces[j];

			RadiosityFace radiosityFace;

			radiosityFace.model = currentObject;
			radiosityFace.faceIndex = j;
			if(currentFace->material->illuminationMode != 1)
				radiosityFace.emission = currentFace->material->diffuseColor * INITIAL_LIGHT_EMITTER_INTENSITY;
			else
				radiosityFace.emission = currentFace->material->diffuseColor * INITIAL_AMBIENT_INTENSITY;

			radiosityFace.totalRadiosity = radiosityFace.emission;
			radiosityFace.unshotRadiosity = radiosityFace.emission;

			sceneFaces.push_back(radiosityFace);
		}
	}

	formFactors.resize(sceneFaces.size(), vector<double>(sceneFaces.size()));
}

int Radiosity::getMaxUnshotRadiosityFaceIndex()
{
	int index = -1;
	double maxUnshot = 0.0;

	for(int i=0; i<sceneFaces.size(); i++)
	{
		double curUnshot = glm::length(sceneFaces[i].unshotRadiosity);
		double curArea = sceneFaces[i].model->getFaceArea(sceneFaces[i].faceIndex);

		curUnshot *= curArea;

		if(curUnshot > maxUnshot)
		{
			index = i;
			maxUnshot = curUnshot;
		}
	}
	return index;
}

void Radiosity::calculateFormFactorsForFace(int i, int samplePointsCount)
{	
	glm::vec3 normal_i = sceneFaces[i].model->getFaceNormal(sceneFaces[i].faceIndex);
	//for every other face
	for(int j=0; j<sceneFaces.size(); j++)
	{
		if(i == j)
		{
			formFactors[i][j] = 0.0;
			continue;
		}

		float area_j = sceneFaces[j].model->getFaceArea(sceneFaces[j].faceIndex);
		glm::vec3 normal_j = sceneFaces[j].model->getFaceNormal(sceneFaces[j].faceIndex);

		formFactors[i][j] = 0.0;

		vector<glm::vec3> samplePoints_i = sceneFaces[i].model->monteCarloSamplePoints(sceneFaces[i].faceIndex, samplePointsCount);
		vector<glm::vec3> samplePoints_j = sceneFaces[j].model->monteCarloSamplePoints(sceneFaces[j].faceIndex, samplePointsCount);

		int numSamplePoints = samplePoints_i.size();
		for(int k=0; k<numSamplePoints; k++)
		{
			if( isVisibleFrom(samplePoints_j[k], samplePoints_i[k]) )
			{
				glm::vec3 r_ij = glm::normalize(samplePoints_j[k] - samplePoints_i[k]);
				//glm::vec3 r_ji = glm::normalize(samplePoints_i[k] - samplePoints_j[k]);

				float r_squared = glm::dot((samplePoints_i[k] - samplePoints_j[k]) , (samplePoints_i[k] - samplePoints_j[k]));
				
				double cos_angle_i = glm::dot(r_ij, normal_i);
				double cos_angle_j = glm::dot(r_ij, normal_j);

				double delta_F = ( cos_angle_i * cos_angle_j ) / ( 3.14159265359 * r_squared + (area_j / numSamplePoints ));

				if(abs(delta_F) > 0.0)
					formFactors[i][j] = formFactors[i][j] + abs(delta_F);
			}
		}
		formFactors[i][j] = formFactors[i][j] * area_j;
	}
}

void Radiosity::PrepareUnshotRadiosityValues()
{
	for(int i=0; i<sceneFaces.size(); i++)
	{
		sceneFaces[i].totalRadiosity = sceneFaces[i].emission;
		sceneFaces[i].unshotRadiosity = sceneFaces[i].emission;
	}
}

void Radiosity::calculateRadiosityValues()
{
	glm::vec3 threshold = RADIOSITY_SOLUTION_THRESHOLD;
	glm::vec3 error(100.0f, 100.0f, 100.0f);

	/*
	for(int i=0; i<sceneFaces.size(); i++)
	{
		sceneFaces[i].totalRadiosity = sceneFaces[i].emission;
		sceneFaces[i].unshotRadiosity = sceneFaces[i].emission;
	}
	*/
	 while (
			error.r > threshold.r &&
			error.g > threshold.g &&
			error.b > threshold.b
		)
	 {
		int i = getMaxUnshotRadiosityFaceIndex();

		calculateFormFactorsForFace(i, FORM_FACTOR_SAMPLES);

		 for (int j=0; j< sceneFaces.size(); j++)
		 {
			glm::dvec3 p_j = (glm::dvec3)sceneFaces[j].model->faces[sceneFaces[j].faceIndex].material->diffuseColor;

			glm::dvec3 delta_rad = sceneFaces[i].unshotRadiosity * formFactors[i][j] * p_j;




			sceneFaces[j].unshotRadiosity = sceneFaces[j].unshotRadiosity + delta_rad;
			sceneFaces[j].totalRadiosity  = sceneFaces[j].totalRadiosity + delta_rad;
		 }

		 sceneFaces[i].unshotRadiosity = glm::dvec3(0.0f, 0.0f, 0.0f);

		int e = getMaxUnshotRadiosityFaceIndex();
		error = sceneFaces[e].unshotRadiosity;
	 }
	



	/*
	int i = getstrongestEmitterFaceIndex();
	calculateFormFactorsForFace(i, 40);

	glm::vec3 delta_S = sceneFaces[i].radiosity;

	//gathering
	for(int j=0; j<sceneFaces.size(); j++)
	{
		glm::vec3 p_j = sceneFaces[j].model->faces[sceneFaces[j].faceIndex].material->diffuseColor;
		float A_i = sceneFaces[i].model->getFaceArea(sceneFaces[i].faceIndex);
		float A_j = sceneFaces[j].model->getFaceArea(sceneFaces[j].faceIndex);

		glm::vec3 P_ij = sceneFaces[j].radiosity * A_j * formFactors[j][i];

		sceneFaces[j].radiosity.r = p_j.r * sceneFaces[i].radiosity.r * A_i * formFactors[i][j] / A_j;
		sceneFaces[j].radiosity.g = p_j.g * sceneFaces[i].radiosity.g * A_i * formFactors[i][j] / A_j;
		sceneFaces[j].radiosity.b = p_j.b * sceneFaces[i].radiosity.b * A_i * formFactors[i][j] / A_j;
	}
	*/
}

void Radiosity::setMeshFaceColors()
{
	for (int i=0; i< sceneFaces.size(); i++)
	{
		glm::dvec3 radValue = glm::clamp(sceneFaces[i].totalRadiosity,0.0,1.0);

		glm::clamp(radValue,0.0,1.0);
		sceneFaces[i].model->faces[sceneFaces[i].faceIndex].intensity = radValue;
	}
}

bool Radiosity::isParallelToFace(Ray* r, int i)
{
	glm::vec3 n = sceneFaces[i].model->getFaceNormal(sceneFaces[i].faceIndex);
	float dotProduct = glm::dot(n, r->getDirection());
	if(abs(dotProduct) <= 0.0001f)
		return true;
	return false;





	//ObjectModel* curModel = sceneFaces[i].model;
	//int v0_index = curModel->faces[sceneFaces[i].faceIndex].vertexIndexes[0];
	//glm::vec3 v0 = curModel->vertices[v0_index];
	//
	//glm::vec3 n = curModel->getFaceNormal(sceneFaces[i].faceIndex);

	//if(n == glm::vec3(0.0f, 0.0f, 0.0f)) //triangle is degenerate
	//	return false;

	//glm::vec3 w0 = r.getStart() - v0;
	//float a = - glm::dot(n, w0);
	//float b = glm::dot(n, r.getDirection());

	//if(glm::abs(b) < 0.000001) //ray is paralel to triangle plane
	//{
	//	if(a == 0) //ray lies in triangle plane
	//		return true;
	//	else       //ray disjoint from plane
	//		return false;
	//}
	//return false;
}

struct RayHit
{
	float distance;
	int hitSceneFaceIndex;
		
};

bool rayHit_LessThan(RayHit r1, RayHit r2)
{
	if(r1.distance < r2.distance)
		return true;
	return false;
}

bool Radiosity::isVisibleFrom(int j, int i)
{
	vector<RayHit> rayHits;

	//get both centroids
	glm::vec3 centroid_i = sceneFaces[i].model->getFaceCentroid(sceneFaces[i].faceIndex);
	glm::vec3 centroid_j = sceneFaces[j].model->getFaceCentroid(sceneFaces[j].faceIndex);

	//now make a ray
	Ray ray(centroid_i, centroid_j);

	for(int k=0; k<sceneFaces.size(); k++)
	{
		if(k == i)
			continue;

		glm::vec3 hitPoint;
		if(doesRayHit(&ray, k, hitPoint))
		{
			RayHit currentHit;
			currentHit.distance = glm::distance(ray.getStart(), hitPoint);
			currentHit.hitSceneFaceIndex = k;
			rayHits.push_back(currentHit);
		}
	}

	std::sort(rayHits.begin(), rayHits.end(), rayHit_LessThan);

	if(rayHits.empty())
		return false;
	if(rayHits[1].hitSceneFaceIndex == j)
		return true;
	return false;
}

bool Radiosity::isVisibleFrom(glm::vec3 point_j, glm::vec3 point_i)
{
	vector<RayHit> rayHits;

	//now make a ray
	Ray ray(point_i, point_j);

	for(int k=0; k<sceneFaces.size(); k++)
	{
		glm::vec3 hitPoint;
		if(doesRayHit(&ray, k, hitPoint))
		{
			if(hitPoint == point_i)
				return true;

			RayHit currentHit;
			currentHit.distance = glm::distance(ray.getStart(), hitPoint);
			currentHit.hitSceneFaceIndex = k;
			rayHits.push_back(currentHit);
		}
	}

	std::sort(rayHits.begin(), rayHits.end(), rayHit_LessThan);

	if(rayHits.empty())
		return false;

	float pointsDistance = glm::distance(point_i, point_j);

	//if(pointsDistance == rayHits[1].distance)
	//	return true;
	return false;
}

bool Radiosity::doesRayHit(Ray* ray, int k, glm::vec3& hitPoint)
{
	//check if ray is parallel to face
	glm::vec3 n_k = sceneFaces[k].model->getFaceNormal(sceneFaces[k].faceIndex);

	if(isParallelToFace(ray, k))
		return false;

	//get all vertices of k
	int v0_k_index = sceneFaces[k].model->faces[sceneFaces[k].faceIndex].vertexIndexes[0];
	int v1_k_index = sceneFaces[k].model->faces[sceneFaces[k].faceIndex].vertexIndexes[1];
	int v2_k_index = sceneFaces[k].model->faces[sceneFaces[k].faceIndex].vertexIndexes[2];

	glm::vec3 A = sceneFaces[k].model->vertices[v0_k_index];
	glm::vec3 B = sceneFaces[k].model->vertices[v1_k_index];
	glm::vec3 C = sceneFaces[k].model->vertices[v2_k_index];
	
	//first we handle the case where patch k has only 3 vertices

	//plane equasion
	//ax + by + cz = d
	//n.x=d
	float a = n_k.x;
	float b = n_k.y;
	float c = n_k.z;
	float d = glm::dot(n_k, A);
	
	//ray equasion
	//R(t) = ray.start + t*ray.direction
	//plug into plane equasion, solve for t

	float t = (d - glm::dot(n_k, ray->getStart())) / glm::dot(n_k, ray->getDirection());

	//calculate the intersection point between the ray and the plane where k lies
	glm::vec3 Q = ray->getStart() + (t * ray->getDirection());

	//now we determine if Q is inside or outside of k
	if(sceneFaces[k].model->faces[sceneFaces[k].faceIndex].vertexIndexes.size() == 3)
	{
		float AB_EDGE = glm::dot(glm::cross((B - A), (Q - A)) , n_k);
		float BC_EDGE = glm::dot(glm::cross((C - B), (Q - B)) , n_k);
		float CA_EDGE = glm::dot(glm::cross((A - C), (Q - C)) , n_k);

		if(AB_EDGE >= 0.0f && BC_EDGE >= 0.0f && CA_EDGE >= 0.0f)
		{
			float ABC_AREA_DOUBLE = glm::dot(glm::cross((B - A), (C - A)) , n_k);

			float alpha = BC_EDGE / ABC_AREA_DOUBLE;
			float beta  = CA_EDGE / ABC_AREA_DOUBLE;
			float gamma = AB_EDGE / ABC_AREA_DOUBLE;

			hitPoint = glm::vec3(alpha * A + beta * B + gamma * C);
			return true;
		}
		else
			return false;		
	}
	else
	{
		int v3_k_index = sceneFaces[k].model->faces[sceneFaces[k].faceIndex].vertexIndexes[3];
		glm::vec3 D = sceneFaces[k].model->vertices[v3_k_index];

		float AB_EDGE = glm::dot(glm::cross((B - A), (Q - A)) , n_k);
		float BD_EDGE = glm::dot(glm::cross((D - B), (Q - B)) , n_k);
		float DA_EDGE = glm::dot(glm::cross((A - D), (Q - D)) , n_k);

		float BC_EDGE = glm::dot(glm::cross((C - B), (Q - B)) , n_k);
		float CD_EDGE = glm::dot(glm::cross((D - C), (Q - C)) , n_k);
		float DB_EDGE = glm::dot(glm::cross((B - D), (Q - D)) , n_k);
		
		if(AB_EDGE >= 0.0f && BD_EDGE >= 0.0f && DA_EDGE >= 0.0f)
		{
			float ABD_AREA_DOUBLE = glm::dot(glm::cross((B - A), (D - A)) , n_k);

			float alpha = BD_EDGE / ABD_AREA_DOUBLE;
			float beta  = DA_EDGE / ABD_AREA_DOUBLE;
			float gamma = AB_EDGE / ABD_AREA_DOUBLE;

			hitPoint = glm::vec3(alpha * A + beta * B + gamma * D);
			return true;
		}
		else if(BC_EDGE >= 0.0f && CD_EDGE >= 0.0f && DB_EDGE >= 0.0f)
		{
			float BCD_AREA_DOUBLE = glm::dot(glm::cross((B - C), (D - C)) , n_k);

			float alpha = CD_EDGE / BCD_AREA_DOUBLE;
			float beta  = DB_EDGE / BCD_AREA_DOUBLE;
			float gamma = BC_EDGE / BCD_AREA_DOUBLE;

			hitPoint = glm::vec3(alpha * B + beta * C + gamma * D);
			return true;
		}
		else
			return false;
	}
	return false;
}