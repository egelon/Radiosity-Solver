#include "Mesh.h"
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iterator>
#include <regex>


#include "bitmap_image.hpp"

Mesh::Mesh()
{
	
}

Mesh::~Mesh()
{
	
}

vector<string> split(const string& s, const string& delim, const bool keep_empty = true)
{
	vector<string> result;
	if (delim.empty())
	{
		result.push_back(s);
		return result;
	}
	string::const_iterator substart = s.begin(), subend;
	while (true)
	{
		subend = search(substart, s.end(), delim.begin(), delim.end());
		string temp(substart, subend);
		if (keep_empty || !temp.empty())
		{
			result.push_back(temp);
		}
		if (subend == s.end())
		{
			break;
		}
		substart = subend + delim.size();
	}
	return result;
}

Material* Mesh::getMaterialPtrByName(string matName)
{
	for(int i=0; i<materials.size(); i++)
		if(materials[i].name == matName)
			return &materials[i];
}

void parseCurrentMaterial(Material& currentMaterial, ifstream& fileStream, string line)
{
	currentMaterial.name = line;

	string prefix;
	vector<string> tokens;
	while(getline(fileStream, line))
	{
		tokens = split(line, " ", false);

		//specular exponent
		prefix = "Ns ";
		if (!line.compare(0, prefix.size(), prefix))
		{
			currentMaterial.specularColorExponent = stod(tokens[1]);
		}

		//ambient color
		prefix = "Ka ";
		if (!line.compare(0, prefix.size(), prefix))
		{
			glm::vec3 ambColor(stod(tokens[1]), stod(tokens[2]), stod(tokens[3]));
			currentMaterial.ambientColor = ambColor;
		}

		//diffuse color
		prefix = "Kd ";
		if (!line.compare(0, prefix.size(), prefix))
		{
			glm::vec3 diffColor(stod(tokens[1]), stod(tokens[2]), stod(tokens[3]));
			currentMaterial.diffuseColor = diffColor;
		}

		//specular color
		prefix = "Ks ";
		if (!line.compare(0, prefix.size(), prefix))
		{
			glm::vec3 specColor(stod(tokens[1]), stod(tokens[2]), stod(tokens[3]));
			currentMaterial.specularColor = specColor;
		}


		//optical density
		prefix = "Ni ";
		if (!line.compare(0, prefix.size(), prefix))
		{
			currentMaterial.opticalDensity = stod(tokens[1]);
		}

		//transparency
		string prefix1 = "d ";
		string prefix2 = "Tr ";
		if (!line.compare(0, prefix1.size(), prefix1) || !line.compare(0, prefix2.size(), prefix2))
		{
			currentMaterial.alpha = stod(tokens[1]);
		}

		//illumination mode
		prefix = "illum ";
		if (!line.compare(0, prefix.size(), prefix))
		{
			currentMaterial.illuminationMode = (IlluminationModes)stoi(tokens[1]);
			return;
		}
	}
}

void Mesh::parseMaterials(string materialsFileName)
{
	ifstream fileStream(materialsFileName, ios::in);

	if (!fileStream)
	{
		cout << "ERROR: cannot open file " << materialsFileName << endl;
		exit(1);
	}

	string line;
	std::string prefix;
	vector<string> tokens;
	while(getline(fileStream, line))
	{
		tokens = split(line, " ", false);
		prefix = "newmtl ";
		if (!line.compare(0, prefix.size(), prefix))
		{
			Material currentMaterial;
			parseCurrentMaterial(currentMaterial, fileStream, tokens[1]);
			materials.push_back(currentMaterial);
		}
	}
	fileStream.close();
}

void Mesh::parseObject(ifstream& fileStream, SceneObject& currentObject, int totalVertexCount)
{
	string line;
	std::string prefix;
	Material* currentMaterial = NULL;
	vector<string> tokens;
	while(getline(fileStream, line))
	{
		//found another object, end parsing
		prefix = "o ";
		if (!line.compare(0, prefix.size(), prefix))
		{
			return;
		}

		//load vertex data
		prefix = "v ";
		if (!line.compare(0, prefix.size(), prefix))
		{
			tokens = split(line, " ", false);
			glm::vec3 vertex(stod(tokens[1]), stod(tokens[2]), stod(tokens[3]));
			currentObject.obj_model.vertices.push_back(vertex);
		}

		//load texture UV/W coordinates
		prefix = "vt ";
		if (!line.compare(0, prefix.size(), prefix))
		{
			tokens = split(line, " ", false);

			float u = stod(tokens[1]);
			float v = stod(tokens[2]);
			float w = 0.0f;
			if(tokens.size() > 3)
				w = stod(tokens[3]);

			glm::vec3 textureUVW_coord(u, v, w);
			currentObject.obj_model.textureUVW.push_back(textureUVW_coord);
		}

		//load vertex normals data
		prefix = "vn ";
		if (!line.compare(0, prefix.size(), prefix))
		{
			tokens = split(line, " ", false);
			glm::vec3 vertexNormal(stod(tokens[1]), stod(tokens[2]), stod(tokens[3]));
			currentObject.obj_model.vertexNormals.push_back(vertexNormal);
		}

		
		//load face materials
		prefix = "usemtl ";
		if (!line.compare(0, prefix.size(), prefix))
		{
			tokens = split(line, " ", false);
			currentMaterial = getMaterialPtrByName(tokens[1]);
		}
		//load faces
		prefix = "f ";
		if (!line.compare(0, prefix.size(), prefix))
		{
			tokens = split(line, " ", false);
			ModelFace face;
			
			int numIndexes = tokens.size() - 1;
			if(tokens[1].find("//") != string::npos) //we have vertex//normals
			{
				face.vertexIndexes.resize(numIndexes);
				face.normalIndexes.resize(numIndexes);

				for(int i=1; i<tokens.size(); i++)
				{
					vector<string> tmp = split(tokens[i], "//", false);
					face.vertexIndexes[i-1] = stoi(tmp[0]) - totalVertexCount - 1;
					face.normalIndexes[i-1] = stoi(tmp[1]) - totalVertexCount - 1;
				}
			}
			else if(regex_match(tokens[1], regex("\\d+/\\d+/\\d+"))) //we have vertex/texture/normal
			{
				face.vertexIndexes.resize(numIndexes);
				face.normalIndexes.resize(numIndexes);
				face.textureIndexes.resize(numIndexes);

				for(int i=1; i<tokens.size(); i++)
				{
					vector<string> tmp = split(tokens[i], "/", false);
					face.vertexIndexes[i-1] = stoi(tmp[0]) - totalVertexCount - 1;
					face.textureIndexes[i-1] = stoi(tmp[1]) - totalVertexCount - 1;
					face.normalIndexes[i-1] = stoi(tmp[2]) - totalVertexCount - 1;
				}
			}
			else //we have vertex/texture
			{
				face.vertexIndexes.resize(numIndexes);
				face.textureIndexes.resize(numIndexes);

				for(int i=1; i<tokens.size(); i++)
				{
					vector<string> tmp = split(tokens[i], "/", false);
					face.vertexIndexes[i-1] = stoi(tmp[0]) - totalVertexCount - 1;
					face.textureIndexes[i-1] = stoi(tmp[1]) - totalVertexCount - 1;
				}
			}
			face.material = currentMaterial;
			currentObject.obj_model.faces.push_back(face);
		}
	}
}

void Mesh::Load(string input_file)
{
	ifstream fileStream(input_file, ios::in);

	if (!fileStream)
	{
		cout << "ERROR: cannot open file " << input_file << endl;
		exit(1);
	}

	

	string line;
	std::string prefix;
	int currentObjectId = 0;

	int streamPos = 0;

	while(getline(fileStream, line))
	{
		//parse materials file
		prefix = "mtllib ";
		if (!line.compare(0, prefix.size(), prefix))
		{
			vector<string> tokens = split(line, " ", false);
			parseMaterials(tokens[1]);
			break;
		}
	}

	fileStream.seekg(ios_base::beg);

	int totalVertexCount = 0;
	int lastVertexCount = 0;

	while(!fileStream.eof())
	{
		//found another object, end parsing
		
			SceneObject currentObject;
			currentObject.obj_id = currentObjectId;
			parseObject(fileStream, currentObject, totalVertexCount);
			currentObject.obj_model.vertexIndexOffset = lastVertexCount;

			totalVertexCount += currentObject.obj_model.vertices.size();
			lastVertexCount = totalVertexCount;

			sceneModel.push_back(currentObject);
			currentObjectId ++;
	}
	fileStream.close();

	sceneModel.erase(sceneModel.begin());

	startingSceneModel = sceneModel;
}

void Mesh::ResetMesh()
{
	sceneModel = startingSceneModel;
}

void Mesh::Subdivide()
{
	for(int i=0; i<sceneModel.size(); i++)
	{
		//for every scene object
		ObjectModel* currentObject = &sceneModel[i].obj_model;
		int currentObjFaces = currentObject->faces.size();
		for(int j=0; j < currentObjFaces; j++)
		{
			//for every face of it
			ModelFace* currentFace = &sceneModel[i].obj_model.faces[j];

			Material* currentMaterial = currentFace->material;

			if(currentFace->vertexIndexes.size() == 3) //our faces are triangles
			{
				//get the 3 vertices for the face
				int index_a = currentFace->vertexIndexes[0];
				int index_b = currentFace->vertexIndexes[1];
				int index_c = currentFace->vertexIndexes[2];

				glm::vec3 vertex_a = currentObject->vertices[index_a];
				glm::vec3 vertex_b = currentObject->vertices[index_b];
				glm::vec3 vertex_c = currentObject->vertices[index_c];

				//calculate the medicenter of the face
				glm::vec3 centroid = currentObject->getFaceCentroid(j);

				//now add the 3 new faces-we add the centroid to the vertex array at the back and get it's index
				currentObject->vertices.push_back(centroid);
				int centroidIndex = currentObject->vertices.size() - 1;

				//now we modify the current face's last vertex index to be the new centroid
				currentFace->vertexIndexes[2] = centroidIndex; // a, b, centroid

				//now we push the other 2 faces
				ModelFace face_a;

				face_a.material = currentMaterial;
				face_a.vertexIndexes.push_back(index_b); //b, c, centroid
				face_a.vertexIndexes.push_back(index_c);
				face_a.vertexIndexes.push_back(centroidIndex);

				currentObject->faces.push_back(face_a);

				ModelFace face_b;

				face_b.material = currentMaterial;
				face_b.vertexIndexes.push_back(index_c); //c, a, centroid
				face_b.vertexIndexes.push_back(index_a);
				face_b.vertexIndexes.push_back(centroidIndex);

				currentObject->faces.push_back(face_b);
			}
			else if(currentFace->vertexIndexes.size() == 4) //we got quads
			{
				//get the 4 vertices for the face
				int index_a = currentFace->vertexIndexes[0];
				int index_b = currentFace->vertexIndexes[1];
				int index_c = currentFace->vertexIndexes[2];
				int index_d = currentFace->vertexIndexes[3];

				glm::vec3 vertex_a = currentObject->vertices[index_a];
				glm::vec3 vertex_b = currentObject->vertices[index_b];
				glm::vec3 vertex_c = currentObject->vertices[index_c];
				glm::vec3 vertex_d = currentObject->vertices[index_d];

				//calculate the medicenter of the face
				glm::vec3 centroid = currentObject->getFaceCentroid(j);

				glm::vec3 vertex_ab( 
						(vertex_a.x + vertex_b.x) / 2.0f,
						(vertex_a.y + vertex_b.y) / 2.0f,
						(vertex_a.z + vertex_b.z) / 2.0f
					);

				glm::vec3 vertex_bc( 
						(vertex_b.x + vertex_c.x) / 2.0f,
						(vertex_b.y + vertex_c.y) / 2.0f,
						(vertex_b.z + vertex_c.z) / 2.0f
					);

				glm::vec3 vertex_cd( 
						(vertex_c.x + vertex_d.x) / 2.0f,
						(vertex_c.y + vertex_d.y) / 2.0f,
						(vertex_c.z + vertex_d.z) / 2.0f
					);

				glm::vec3 vertex_da( 
						(vertex_d.x + vertex_a.x) / 2.0f,
						(vertex_d.y + vertex_a.y) / 2.0f,
						(vertex_d.z + vertex_a.z) / 2.0f
					);
				

				currentObject->vertices.push_back(centroid);
				int centroidIndex = currentObject->vertices.size() - 1;

				currentObject->vertices.push_back(vertex_ab);
				int abIndex = currentObject->vertices.size() - 1;
				currentObject->vertices.push_back(vertex_bc);
				int bcIndex = currentObject->vertices.size() - 1;
				currentObject->vertices.push_back(vertex_cd);
				int cdIndex = currentObject->vertices.size() - 1;
				currentObject->vertices.push_back(vertex_da);
				int daIndex = currentObject->vertices.size() - 1;


				currentFace->vertexIndexes[1] = abIndex;
				currentFace->vertexIndexes[2] = centroidIndex;
				currentFace->vertexIndexes[3] = daIndex;

				ModelFace face_a;
				face_a.material = currentMaterial;
				face_a.vertexIndexes.push_back(abIndex);
				face_a.vertexIndexes.push_back(index_b);
				face_a.vertexIndexes.push_back(bcIndex);
				face_a.vertexIndexes.push_back(centroidIndex);

				currentObject->faces.push_back(face_a);

				ModelFace face_b;
				face_b.material = currentMaterial;
				face_b.vertexIndexes.push_back(centroidIndex);
				face_b.vertexIndexes.push_back(bcIndex);
				face_b.vertexIndexes.push_back(index_c);
				face_b.vertexIndexes.push_back(cdIndex);

				currentObject->faces.push_back(face_b);

				ModelFace face_c;
				face_c.material = currentMaterial;
				face_c.vertexIndexes.push_back(daIndex);
				face_c.vertexIndexes.push_back(centroidIndex);
				face_c.vertexIndexes.push_back(cdIndex);
				face_c.vertexIndexes.push_back(index_d);

				currentObject->faces.push_back(face_c);
			}
			else
			{
				printf("Can't subdivide: faces are neither triangles, nor quads.\n");
				return;
			}
		}
	}
}

vector<ModelFace*> Mesh::GetFaceIndexesFromVertexIndex(int modelIndex, int vertIndex)
{
	ObjectModel* currentModel = &sceneModel[modelIndex].obj_model;

	vector<ModelFace*> result;

	for(int i=0; i<currentModel->faces.size(); i++)
	{
		ModelFace* currentFace = &currentModel->faces[i];
		for(int j=0; j<currentFace->vertexIndexes.size(); j++)
		{
			if(currentFace->vertexIndexes[j] == vertIndex)
				result.push_back(currentFace);
		}
	}
	return result;
}

void Mesh::cacheVerticesFacesAndColors_Radiosity()
{
	vertex_positions.clear();
	face_indexes.clear();
	vertex_colors.clear();

	//for every scene object
	for(int i=0; i<sceneModel.size(); i++)
	{
		for(int j=0; j<sceneModel[i].obj_model.faces.size(); j++)
		{
			//for every face of it
			ModelFace currentFace = sceneModel[i].obj_model.faces[j];

			glm::vec3 currentColor(0.0f, 0.0f, 0.0f);
			float currentIntensity = 0.0f;
			vector<ModelFace*> incidentFaces;
			int numIncidentFaces;
			
			if(currentFace.vertexIndexes.size() == 3) //we have triangles
			{
				int index_a = currentFace.vertexIndexes[0];
				int index_b = currentFace.vertexIndexes[1];
				int index_c = currentFace.vertexIndexes[2];

				glm::vec3 vertex_a = sceneModel[i].obj_model.vertices[index_a];
				glm::vec3 vertex_b = sceneModel[i].obj_model.vertices[index_b];
				glm::vec3 vertex_c = sceneModel[i].obj_model.vertices[index_c];

				//push a
				vertex_positions.push_back(vertex_a.x);
				vertex_positions.push_back(vertex_a.y);
				vertex_positions.push_back(vertex_a.z);

				face_indexes.push_back((vertex_positions.size() / 3) - 1);

				currentColor = glm::vec3(0.0f, 0.0f, 0.0f);
				currentIntensity = 0.0f;
				incidentFaces = GetFaceIndexesFromVertexIndex(i, index_a);

				numIncidentFaces = incidentFaces.size();
				for(int j=0; j< numIncidentFaces; j++)
				{
					currentIntensity += incidentFaces[j]->intensity;
					currentColor += incidentFaces[j]->material->diffuseColor;
				}

				currentIntensity /= (float)numIncidentFaces;
				currentColor /= (float)numIncidentFaces;
				currentColor *= currentIntensity;



				vertex_colors.push_back(currentColor.r);
				vertex_colors.push_back(currentColor.g);
				vertex_colors.push_back(currentColor.b);

				//push b
				vertex_positions.push_back(vertex_b.x);
				vertex_positions.push_back(vertex_b.y);
				vertex_positions.push_back(vertex_b.z);

				face_indexes.push_back((vertex_positions.size() / 3) - 1);

				currentColor = glm::vec3(0.0f, 0.0f, 0.0f);
				currentIntensity = 0.0f;
				incidentFaces = GetFaceIndexesFromVertexIndex(i, index_b);

				numIncidentFaces = incidentFaces.size();
				for(int j=0; j< numIncidentFaces; j++)
				{
					currentIntensity += incidentFaces[j]->intensity;
					currentColor += incidentFaces[j]->material->diffuseColor;
				}

				currentIntensity /= (float)numIncidentFaces;
				currentColor /= (float)numIncidentFaces;
				currentColor *= currentIntensity;

				vertex_colors.push_back(currentColor.r);
				vertex_colors.push_back(currentColor.g);
				vertex_colors.push_back(currentColor.b);

				//push c
				vertex_positions.push_back(vertex_c.x);
				vertex_positions.push_back(vertex_c.y);
				vertex_positions.push_back(vertex_c.z);

				face_indexes.push_back((vertex_positions.size() / 3) - 1);

				currentColor = glm::vec3(0.0f, 0.0f, 0.0f);
				currentIntensity = 0.0f;
				incidentFaces = GetFaceIndexesFromVertexIndex(i, index_c);

				numIncidentFaces = incidentFaces.size();
				for(int j=0; j< numIncidentFaces; j++)
				{
					currentIntensity += incidentFaces[j]->intensity;
					currentColor += incidentFaces[j]->material->diffuseColor;
				}

				currentIntensity /= (float)numIncidentFaces;
				currentColor /= (float)numIncidentFaces;
				currentColor *= currentIntensity;



				vertex_colors.push_back(currentColor.r);
				vertex_colors.push_back(currentColor.g);
				vertex_colors.push_back(currentColor.b);
			}
			else if(currentFace.vertexIndexes.size() == 4) //we have quads
			{
				int index_a = currentFace.vertexIndexes[0];
				int index_b = currentFace.vertexIndexes[1];
				int index_c = currentFace.vertexIndexes[2];
				int index_d = currentFace.vertexIndexes[3];

				glm::vec3 vertex_a = sceneModel[i].obj_model.vertices[index_a];
				glm::vec3 vertex_b = sceneModel[i].obj_model.vertices[index_b];
				glm::vec3 vertex_c = sceneModel[i].obj_model.vertices[index_c];
				glm::vec3 vertex_d = sceneModel[i].obj_model.vertices[index_d];

				//push a
				vertex_positions.push_back(vertex_a.x);
				vertex_positions.push_back(vertex_a.y);
				vertex_positions.push_back(vertex_a.z);

				face_indexes.push_back((vertex_positions.size() / 3) - 1);


				currentColor = glm::vec3(0.0f, 0.0f, 0.0f);
				currentIntensity = 0.0f;
				incidentFaces = GetFaceIndexesFromVertexIndex(i, index_a);

				numIncidentFaces = incidentFaces.size();
				for(int j=0; j< numIncidentFaces; j++)
				{
					currentIntensity += incidentFaces[j]->intensity;
					currentColor += incidentFaces[j]->material->diffuseColor;
				}

				currentIntensity /= (float)numIncidentFaces;
				currentColor /= (float)numIncidentFaces;
				currentColor *= currentIntensity;




				vertex_colors.push_back(currentColor.r);
				vertex_colors.push_back(currentColor.g);
				vertex_colors.push_back(currentColor.b);

				//push b
				vertex_positions.push_back(vertex_b.x);
				vertex_positions.push_back(vertex_b.y);
				vertex_positions.push_back(vertex_b.z);

				face_indexes.push_back((vertex_positions.size() / 3) - 1);


				currentColor = glm::vec3(0.0f, 0.0f, 0.0f);
				currentIntensity = 0.0f;
				incidentFaces = GetFaceIndexesFromVertexIndex(i, index_b);

				numIncidentFaces = incidentFaces.size();
				for(int j=0; j< numIncidentFaces; j++)
				{
					currentIntensity += incidentFaces[j]->intensity;
					currentColor += incidentFaces[j]->material->diffuseColor;
				}

				currentIntensity /= (float)numIncidentFaces;
				currentColor /= (float)numIncidentFaces;
				currentColor *= currentIntensity;




				vertex_colors.push_back(currentColor.r);
				vertex_colors.push_back(currentColor.g);
				vertex_colors.push_back(currentColor.b);

				//push d
				vertex_positions.push_back(vertex_d.x);
				vertex_positions.push_back(vertex_d.y);
				vertex_positions.push_back(vertex_d.z);

				face_indexes.push_back((vertex_positions.size() / 3) - 1);


				currentColor = glm::vec3(0.0f, 0.0f, 0.0f);
				currentIntensity = 0.0f;
				incidentFaces = GetFaceIndexesFromVertexIndex(i, index_d);

				numIncidentFaces = incidentFaces.size();
				for(int j=0; j< numIncidentFaces; j++)
				{
					currentIntensity += incidentFaces[j]->intensity;
					currentColor += incidentFaces[j]->material->diffuseColor;
				}

				currentIntensity /= (float)numIncidentFaces;
				currentColor /= (float)numIncidentFaces;
				currentColor *= currentIntensity;


				vertex_colors.push_back(currentColor.r);
				vertex_colors.push_back(currentColor.g);
				vertex_colors.push_back(currentColor.b);

				//==========================================================

				//push b
				vertex_positions.push_back(vertex_b.x);
				vertex_positions.push_back(vertex_b.y);
				vertex_positions.push_back(vertex_b.z);

				face_indexes.push_back((vertex_positions.size() / 3) - 1);

				currentColor = glm::vec3(0.0f, 0.0f, 0.0f);
				currentIntensity = 0.0f;
				incidentFaces = GetFaceIndexesFromVertexIndex(i, index_b);

				numIncidentFaces = incidentFaces.size();
				for(int j=0; j< numIncidentFaces; j++)
				{
					currentIntensity += incidentFaces[j]->intensity;
					currentColor += incidentFaces[j]->material->diffuseColor;
				}

				currentIntensity /= (float)numIncidentFaces;
				currentColor /= (float)numIncidentFaces;
				currentColor *= currentIntensity;
				


				vertex_colors.push_back(currentColor.r);
				vertex_colors.push_back(currentColor.g);
				vertex_colors.push_back(currentColor.b);


				//push c
				vertex_positions.push_back(vertex_c.x);
				vertex_positions.push_back(vertex_c.y);
				vertex_positions.push_back(vertex_c.z);

				face_indexes.push_back((vertex_positions.size() / 3) - 1);

				currentColor = glm::vec3(0.0f, 0.0f, 0.0f);
				currentIntensity = 0.0f;
				incidentFaces = GetFaceIndexesFromVertexIndex(i, index_c);

				numIncidentFaces = incidentFaces.size();
				for(int j=0; j< numIncidentFaces; j++)
				{
					currentIntensity += incidentFaces[j]->intensity;
					currentColor += incidentFaces[j]->material->diffuseColor;
				}

				currentIntensity /= (float)numIncidentFaces;
				currentColor /= (float)numIncidentFaces;
				currentColor *= currentIntensity;
				


				vertex_colors.push_back(currentColor.r);
				vertex_colors.push_back(currentColor.g);
				vertex_colors.push_back(currentColor.b);


				//push d
				vertex_positions.push_back(vertex_d.x);
				vertex_positions.push_back(vertex_d.y);
				vertex_positions.push_back(vertex_d.z);

				face_indexes.push_back((vertex_positions.size() / 3) - 1);

				currentColor = glm::vec3(0.0f, 0.0f, 0.0f);
				currentIntensity = 0.0f;
				incidentFaces = GetFaceIndexesFromVertexIndex(i, index_d);

				numIncidentFaces = incidentFaces.size();
				for(int j=0; j< numIncidentFaces; j++)
				{
					currentIntensity += incidentFaces[j]->intensity;
					currentColor += incidentFaces[j]->material->diffuseColor;
				}

				currentIntensity /= (float)numIncidentFaces;
				currentColor /= (float)numIncidentFaces;
				currentColor *= currentIntensity;
				

				vertex_colors.push_back(currentColor.r);
				vertex_colors.push_back(currentColor.g);
				vertex_colors.push_back(currentColor.b);
			}
			else
			{
				printf("Model doesn't have triangles or quads. Can't process\n");
				return;
			}
		}
	}
}

void Mesh::cacheVerticesFacesAndColors()
{
	vertex_positions.clear();
	face_indexes.clear();
	vertex_colors.clear();
	
	for(int i=0; i<sceneModel.size(); i++)
	{
		//for every scene object
		for(int j=0; j<sceneModel[i].obj_model.faces.size(); j++)
		{
			//for every face of it
			ModelFace currentFace = sceneModel[i].obj_model.faces[j];

			//FOR DEBUG
			
			float color_r = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/1.0));
			float color_g = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/1.0));
			float color_b = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/1.0));
			glm::vec3 currentColor(color_r, color_g, color_b);
			
			//glm::vec3 currentColor = currentFace.material->diffuseColor;

			if(currentFace.vertexIndexes.size() == 3) //we have triangles
			{
				int index_a = currentFace.vertexIndexes[0];
				int index_b = currentFace.vertexIndexes[1];
				int index_c = currentFace.vertexIndexes[2];

				glm::vec3 vertex_a = sceneModel[i].obj_model.vertices[index_a];
				glm::vec3 vertex_b = sceneModel[i].obj_model.vertices[index_b];
				glm::vec3 vertex_c = sceneModel[i].obj_model.vertices[index_c];

				//push a
				vertex_positions.push_back(vertex_a.x);
				vertex_positions.push_back(vertex_a.y);
				vertex_positions.push_back(vertex_a.z);

				face_indexes.push_back((vertex_positions.size() / 3) - 1);

				vertex_colors.push_back(currentColor.r);
				vertex_colors.push_back(currentColor.g);
				vertex_colors.push_back(currentColor.b);

				//push b
				vertex_positions.push_back(vertex_b.x);
				vertex_positions.push_back(vertex_b.y);
				vertex_positions.push_back(vertex_b.z);

				face_indexes.push_back((vertex_positions.size() / 3) - 1);

				vertex_colors.push_back(currentColor.r);
				vertex_colors.push_back(currentColor.g);
				vertex_colors.push_back(currentColor.b);

				//push c
				vertex_positions.push_back(vertex_c.x);
				vertex_positions.push_back(vertex_c.y);
				vertex_positions.push_back(vertex_c.z);

				face_indexes.push_back((vertex_positions.size() / 3) - 1);

				vertex_colors.push_back(currentColor.r);
				vertex_colors.push_back(currentColor.g);
				vertex_colors.push_back(currentColor.b);
			}
			else if(currentFace.vertexIndexes.size() == 4) //we have quads
			{
				int index_a = currentFace.vertexIndexes[0];
				int index_b = currentFace.vertexIndexes[1];
				int index_c = currentFace.vertexIndexes[2];
				int index_d = currentFace.vertexIndexes[3];

				glm::vec3 vertex_a = sceneModel[i].obj_model.vertices[index_a];
				glm::vec3 vertex_b = sceneModel[i].obj_model.vertices[index_b];
				glm::vec3 vertex_c = sceneModel[i].obj_model.vertices[index_c];
				glm::vec3 vertex_d = sceneModel[i].obj_model.vertices[index_d];

				//push a
				vertex_positions.push_back(vertex_a.x);
				vertex_positions.push_back(vertex_a.y);
				vertex_positions.push_back(vertex_a.z);

				face_indexes.push_back((vertex_positions.size() / 3) - 1);

				vertex_colors.push_back(currentColor.r);
				vertex_colors.push_back(currentColor.g);
				vertex_colors.push_back(currentColor.b);

				//push b
				vertex_positions.push_back(vertex_b.x);
				vertex_positions.push_back(vertex_b.y);
				vertex_positions.push_back(vertex_b.z);

				face_indexes.push_back((vertex_positions.size() / 3) - 1);

				vertex_colors.push_back(currentColor.r);
				vertex_colors.push_back(currentColor.g);
				vertex_colors.push_back(currentColor.b);

				//push d
				vertex_positions.push_back(vertex_d.x);
				vertex_positions.push_back(vertex_d.y);
				vertex_positions.push_back(vertex_d.z);

				face_indexes.push_back((vertex_positions.size() / 3) - 1);

				vertex_colors.push_back(currentColor.r);
				vertex_colors.push_back(currentColor.g);
				vertex_colors.push_back(currentColor.b);

				//==========================================================

				//push b
				vertex_positions.push_back(vertex_b.x);
				vertex_positions.push_back(vertex_b.y);
				vertex_positions.push_back(vertex_b.z);

				face_indexes.push_back((vertex_positions.size() / 3) - 1);

				vertex_colors.push_back(currentColor.r);
				vertex_colors.push_back(currentColor.g);
				vertex_colors.push_back(currentColor.b);

				//push c
				vertex_positions.push_back(vertex_c.x);
				vertex_positions.push_back(vertex_c.y);
				vertex_positions.push_back(vertex_c.z);

				face_indexes.push_back((vertex_positions.size() / 3) - 1);

				vertex_colors.push_back(currentColor.r);
				vertex_colors.push_back(currentColor.g);
				vertex_colors.push_back(currentColor.b);

				//push d
				vertex_positions.push_back(vertex_d.x);
				vertex_positions.push_back(vertex_d.y);
				vertex_positions.push_back(vertex_d.z);

				face_indexes.push_back((vertex_positions.size() / 3) - 1);

				vertex_colors.push_back(currentColor.r);
				vertex_colors.push_back(currentColor.g);
				vertex_colors.push_back(currentColor.b);
			}
			else
			{
				printf("Model doesn't have triangles or quads. Can't process\n");
				return;
			}
		}
	}
}

void Mesh::DrawWireframe()
{
	
}

GLuint Mesh::LoadDefaultShaders()
{
	shaderProgramID = shaderLoader.LoadShaders("shaders/MeshVertexShader.vert", "shaders/MeshFragmentShader.frag");
	return shaderProgramID;
}

GLuint Mesh::LoadGouraudShaders()
{
	shaderProgramID = shaderLoader.LoadShaders("shaders/MeshGouraudShader.vert", "shaders/MeshGouraudShader.frag");
	return shaderProgramID;
}

void Mesh::PrepareToDraw()
{
	//cacheVerticesFacesAndColors();

	//create a vertex buffer
	glGenBuffers(1, &vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);

	//fill the vertex buffer with our mesh's data
	glBufferData(GL_ARRAY_BUFFER, vertex_positions.size() * sizeof(GLfloat), &vertex_positions[0], GL_STATIC_DRAW);

	//create a color buffer
	glGenBuffers(1, &colorBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
	//fill the color buffer
	glBufferData(GL_ARRAY_BUFFER, vertex_colors.size() * sizeof(GLfloat), &vertex_colors[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices
	glGenBuffers(1, &elementBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_indexes.size() * sizeof(GLuint), &face_indexes[0], GL_STATIC_DRAW);

}

void Mesh::Draw()
{
	// Use our shader
	glUseProgram(shaderProgramID);
//=============================== CONFIGURATION
	// Get a handle for our buffers
	//this will be our "vertex location" attribute

	// Get a handle to the vertexPosition_modelspace parameter from the shader
	GLuint vertexPosition_modelspaceID = glGetAttribLocation(shaderProgramID, "vertexPosition_modelspace");

	//Get a handle to the vertexColor parameter from the shader
	GLuint vertexColorID = glGetAttribLocation(shaderProgramID, "vertexColor");

	// Get a handle for our "MVP" uniform from the shader
	GLuint MVP_MatrixID = glGetUniformLocation(shaderProgramID, "MVP");

	// Send our ModelViewProjectionMatrix to the currently bound shader, in the "MVP" parameter
	glUniformMatrix4fv(MVP_MatrixID, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

// 1rst attribute buffer will be the "vertex location" we got from the shader
	glEnableVertexAttribArray(vertexPosition_modelspaceID);

	//use the vertex buffer
	//this is the data we'll give as input to OpenGL
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);

	//describe the data's attributes to OpenGL
	//(all it sees is a bunch of numbers - that could be anything)
	glVertexAttribPointer(
			vertexPosition_modelspaceID, // The attribute we want to configure
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

// 2nd attribute buffer : colors
	glEnableVertexAttribArray(vertexColorID);
	glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
	glVertexAttribPointer(
		vertexColorID,                    // The attribute we want to configure
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

// 3rd attribute buffer: indexes
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferID);

//============================================================================

	//draw the data we gave as a triangle
	glDrawElements(
		GL_TRIANGLES,      // mode
		face_indexes.size(),    // count
		GL_UNSIGNED_INT,   // type
		(void*)0           // element array buffer offset
		);

	//remove the attribute
	glDisableVertexAttribArray(vertexPosition_modelspaceID);
	glDisableVertexAttribArray(vertexColorID);

}

void Mesh::Cleanup()
{
	glDeleteBuffers(1, &vertexBufferID);
	glDeleteBuffers(1, &colorBufferID);
	glDeleteBuffers(1, &elementBufferID);
	glDeleteProgram(shaderProgramID);
}

void Mesh::OutputToBitmap(string bmpName, int width, int height)
{
	unsigned char* red_channel = new unsigned char[width*height];
	unsigned char* green_channel = new unsigned char[width*height];
	unsigned char* blue_channel = new unsigned char[width*height];
	if(red_channel == NULL || green_channel == NULL || blue_channel == NULL)
	{
		printf("Could not allocate memory for image %s\n", bmpName);
		return;
	}

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadBuffer(GL_FRONT);

	glReadPixels(0, 0, (GLsizei)width, (GLsizei)height, GL_RED, GL_UNSIGNED_BYTE, red_channel);
	glReadPixels(0, 0, (GLsizei)width, (GLsizei)height, GL_GREEN, GL_UNSIGNED_BYTE, green_channel);
	glReadPixels(0, 0, (GLsizei)width, (GLsizei)height, GL_BLUE, GL_UNSIGNED_BYTE, blue_channel);


	bitmap_image image(width, height);

	//image.bgr_to_rgb();

	image.import_rgb(red_channel, green_channel, blue_channel);

	image.save_image(bmpName);
}