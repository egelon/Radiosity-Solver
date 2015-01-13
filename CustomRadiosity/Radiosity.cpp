#include "Radiosity.h"
#include <algorithm>



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
				radiosityFace.emission = glm::vec3(0.9f, 0.9f, 0.9f);
			else
				radiosityFace.emission = glm::vec3(0.0f, 0.0f, 0.0f);

			radiosityFace.totalRadiosity = radiosityFace.emission;
			radiosityFace.unshotRadiosity = radiosityFace.emission;

			sceneFaces.push_back(radiosityFace);
		}
	}

	formFactors.resize(sceneFaces.size(), vector<double>(sceneFaces.size()));
}

/*
void Radiosity::initEmittedEnergies()
{
	emittedEnergy.clear();

	for(int i=0; i<mesh->sceneModel.size(); i++)
	{
		//for every scene object
		ObjectModel* currentObject = &mesh->sceneModel[i].obj_model;

		int currentObjFaces = currentObject->faces.size();

		for(int j=0; j < currentObjFaces; j++)
		{
			//for every face of it
			ModelFace* currentFace = &mesh->sceneModel[i].obj_model.faces[j];
			Material* currentMaterial = currentFace->material;

			if(currentMaterial->illuminationMode == 0) //material is emitter
				emittedEnergy.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
			else
				emittedEnergy.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
		}
	}
}
*/
void Radiosity::calculateFormFactorsForFace(int i, int samplePointsCount)
{	
	glm::vec3 normal_i = sceneFaces[i].model->getFaceNormal(sceneFaces[i].faceIndex);
	//for every other face
	for(int j=0; j<sceneFaces.size(); j++)
	{
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

					double delta_F = ( cos_angle_i * cos_angle_j ) / ( 3.14 * r_squared + (area_j / numSamplePoints ));

					if(abs(delta_F) > 0.0)
						formFactors[i][j] = formFactors[i][j] + abs(delta_F);
				}
			}
			formFactors[i][j] = formFactors[i][j] * area_j;
	}
}

int Radiosity::getMaxUnshotRadiosityFaceIndex()
{
	/*
	int currentIndex = -1;
	float total_undistributed = 0.0;
	float total_area = 0.0;
	float max = -1.0;
	for(int i=0; i<sceneFaces.size(); i++)
	{
		float faceArea = sceneFaces[i].model->getFaceArea(sceneFaces[i].faceIndex);
		float m = sceneFaces[i].unshotRadiosity.length() * faceArea;
		total_undistributed += m;
		total_area += faceArea;
		if(max < m)
		{
			max = m;
			currentIndex = i;
		}
	}
	return currentIndex;
	*/

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

void Radiosity::calculateAllFormFactors()
{
	for(int i=0; i<sceneFaces.size(); i++)
	{
		calculateFormFactorsForFace(i, 40);
	}
}

void Radiosity::calculateRadiosityValues()
{
	//calculateAllFormFactors();

	glm::vec3 threshold(0.4f, 0.4f, 0.4f);
	glm::vec3 error(0.0f, 0.0f, 0.0f);

	for(int i=0; i<sceneFaces.size(); i++)
	{
		sceneFaces[i].totalRadiosity = sceneFaces[i].emission;
		sceneFaces[i].unshotRadiosity = sceneFaces[i].emission;
	}

	 while (
			error.r < threshold.r &&
			error.g < threshold.g &&
			error.b < threshold.b
		)
	 {
		int i = getMaxUnshotRadiosityFaceIndex();

		calculateFormFactorsForFace(i, 40);

		 for (int j=0; j< sceneFaces.size(); j++)
		 {
			glm::dvec3 p_j = (glm::dvec3)sceneFaces[j].model->faces[sceneFaces[j].faceIndex].material->diffuseColor;

			glm::dvec3 delta_rad = sceneFaces[i].unshotRadiosity * formFactors[i][j] / p_j ;

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
	if(glm::dot(r->getDirection(), n) == 0.0f)
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
	if(rayHits[0].hitSceneFaceIndex == j)
		return true;
	return false;
}

bool Radiosity::isVisibleFrom(glm::vec3 point_j, glm::vec3 point_i)
{
	vector<RayHit> rayHits;

	//now make a ray
	Ray ray(point_j, point_i);

	for(int k=0; k<sceneFaces.size(); k++)
	{
		glm::vec3 hitPoint;
		if(doesRayHit(&ray, k, hitPoint))
		{
			if(hitPoint == point_i)
				return true;
/*
			RayHit currentHit;
			currentHit.distance = glm::distance(ray.getStart(), hitPoint);
			currentHit.hitSceneFaceIndex = k;
			rayHits.push_back(currentHit);*/
		}
	}

	/*std::sort(rayHits.begin(), rayHits.end(), rayHit_LessThan);

	if(rayHits.empty())
		return false;
	if(rayHits[0].distance == glm::distance(point_j, point_i))
		return true;*/
	return true;
}

bool Radiosity::doesRayHit(Ray* ray, int j, glm::vec3& hitPoint)
{
	//check if ray is parallel to face
	glm::vec3 n_j = sceneFaces[j].model->getFaceNormal(sceneFaces[j].faceIndex);

	float isParallel = glm::dot(n_j, ray->getDirection());

	if(abs(isParallel) <= 0.0001f)
		return false;

	//get all vertices of j
	int v0_j_index = sceneFaces[j].model->faces[sceneFaces[j].faceIndex].vertexIndexes[0];
	int v1_j_index = sceneFaces[j].model->faces[sceneFaces[j].faceIndex].vertexIndexes[1];
	int v2_j_index = sceneFaces[j].model->faces[sceneFaces[j].faceIndex].vertexIndexes[2];

	glm::vec3 A = sceneFaces[j].model->vertices[v0_j_index];
	glm::vec3 B = sceneFaces[j].model->vertices[v1_j_index];
	glm::vec3 C = sceneFaces[j].model->vertices[v2_j_index];
	
	//first we handle the case where patch j has only 3 vertices

	//plane equasion
	//ax + by + cz = d
	//n.x=d
	float a = n_j.x;
	float b = n_j.y;
	float c = n_j.z;
	float d = glm::dot(n_j, A);
	
	//ray equasion
	//R(t) = ray.start + t*ray.direction
	//plug into plane equasion, solve for t

	float t = (d - glm::dot(n_j, ray->getStart())) / glm::dot(n_j, ray->getDirection());

	//calculate the intersection point between the ray and the plane where j lies
	glm::vec3 Q = ray->getStart() + (t * ray->getDirection());

	//now we determine if Q is inside or outside of j
	if(sceneFaces[j].model->faces[sceneFaces[j].faceIndex].vertexIndexes.size() == 3)
	{
		float AB_EDGE = glm::dot(glm::cross((B - A), (Q - A)) , n_j);
		float BC_EDGE = glm::dot(glm::cross((C - B), (Q - B)) , n_j);
		float CA_EDGE = glm::dot(glm::cross((A - C), (Q - C)) , n_j);

		if(AB_EDGE >= 0.0f && BC_EDGE >= 0.0f && CA_EDGE >= 0.0f)
		{
			float ABC_AREA_DOUBLE = glm::dot(glm::cross((B - A), (C - A)) , n_j);

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
		int v3_j_index = sceneFaces[j].model->faces[sceneFaces[j].faceIndex].vertexIndexes[3];
		glm::vec3 D = sceneFaces[j].model->vertices[v3_j_index];

		float AB_EDGE = glm::dot(glm::cross((B - A), (Q - A)) , n_j);
		float BD_EDGE = glm::dot(glm::cross((D - B), (Q - B)) , n_j);
		float DA_EDGE = glm::dot(glm::cross((A - D), (Q - D)) , n_j);

		float BC_EDGE = glm::dot(glm::cross((C - B), (Q - B)) , n_j);
		float CD_EDGE = glm::dot(glm::cross((D - C), (Q - C)) , n_j);
		float DB_EDGE = glm::dot(glm::cross((B - D), (Q - D)) , n_j);
		
		if(AB_EDGE >= 0.0f && BD_EDGE >= 0.0f && DA_EDGE >= 0.0f)
		{
			float ABD_AREA_DOUBLE = glm::dot(glm::cross((B - A), (D - A)) , n_j);

			float alpha = BD_EDGE / ABD_AREA_DOUBLE;
			float beta  = DA_EDGE / ABD_AREA_DOUBLE;
			float gamma = AB_EDGE / ABD_AREA_DOUBLE;

			hitPoint = glm::vec3(alpha * A + beta * B + gamma * D);
			return true;
		}
		else if(BC_EDGE >= 0.0f && CD_EDGE >= 0.0f && DB_EDGE >= 0.0f)
		{
			float BCD_AREA_DOUBLE = glm::dot(glm::cross((B - C), (D - C)) , n_j);

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