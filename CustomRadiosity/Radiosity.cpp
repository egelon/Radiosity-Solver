#include "Radiosity.h"


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
				radiosityFace.emission = currentFace->material->diffuseColor;
			else
				radiosityFace.emission = glm::vec3(0.0f, 0.0f, 0.0f);

			radiosityFace.totalRadiosity = radiosityFace.emission;
			radiosityFace.unshotRadiosity = radiosityFace.emission;

			sceneFaces.push_back(radiosityFace);
		}
	}

	formFactors.resize(sceneFaces.size(), vector<float>(sceneFaces.size()));
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
	//for every other face
	for(int j=0; j<sceneFaces.size(); j++)
	{
		float area_j = sceneFaces[j].model->getFaceArea(sceneFaces[j].faceIndex);

		glm::vec3 normal_i = sceneFaces[i].model->getFaceNormal(sceneFaces[i].faceIndex);
		glm::vec3 normal_j = sceneFaces[j].model->getFaceNormal(sceneFaces[j].faceIndex);

		formFactors[i][j] = 0.0; 
		vector<glm::vec3> samplePoints_i = sceneFaces[i].model->monteCarloSamplePoints(sceneFaces[i].faceIndex, samplePointsCount);
		vector<glm::vec3> samplePoints_j = sceneFaces[j].model->monteCarloSamplePoints(sceneFaces[j].faceIndex, samplePointsCount);

		int numSamplePoints = samplePoints_i.size();
		for(int k=0; k<numSamplePoints; k++)
		{
			//if(xj is visible from xi)
			//{
				glm::vec3 r_ij = glm::normalize(samplePoints_j[k] - samplePoints_i[k]);
				//glm::vec3 r_ji = glm::normalize(samplePoints_i[k] - samplePoints_j[k]);

				float r_squared = glm::dot((samplePoints_i[k] - samplePoints_j[k]) , (samplePoints_i[k] - samplePoints_j[k]));
				
				double cos_angle_i = glm::dot(r_ij, normal_i);
				double cos_angle_j = glm::dot(r_ij, normal_j);

				float delta_F = ( cos_angle_i * cos_angle_j ) / ( 3.14 * r_squared + (area_j / numSamplePoints ));

				if(delta_F > 0.0)
					formFactors[i][j] = formFactors[i][j] + delta_F;
					//formFactor += delta_F;
			//}
		}

		//formFactor *= area_j;
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
	glm::vec3 maxUnshot(0.0f, 0.0f, 0.0f);
	for(int i=0; i<sceneFaces.size(); i++)
	{
		glm::vec3 curUnshot = sceneFaces[i].unshotRadiosity;
		float curArea = sceneFaces[i].model->getFaceArea(sceneFaces[i].faceIndex);

		curUnshot *= curArea;
		if(
				curUnshot.r >= maxUnshot.r &&
				curUnshot.g >= maxUnshot.g &&
				curUnshot.b >= maxUnshot.b
			)
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

	glm::vec3 threshold(2.5f, 2.5f, 2.5f);
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
			glm::vec3 p_j = sceneFaces[j].model->faces[sceneFaces[j].faceIndex].material->diffuseColor;

			glm::vec3 delta_rad = sceneFaces[i].unshotRadiosity * formFactors[i][j] / p_j ;

			sceneFaces[j].unshotRadiosity = sceneFaces[j].unshotRadiosity + delta_rad;
			sceneFaces[j].totalRadiosity  = sceneFaces[j].totalRadiosity + delta_rad;
		 }

		 sceneFaces[i].unshotRadiosity = glm::vec3(0.0f, 0.0f, 0.0f);

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
		glm::vec3 radValue = glm::clamp(sceneFaces[i].totalRadiosity,0.0f,1.0f);

		glm::clamp(radValue,0.0f,1.0f);
		sceneFaces[i].model->faces[sceneFaces[i].faceIndex].intensity = radValue.r;
	}
}