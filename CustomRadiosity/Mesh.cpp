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



const string Mesh::vertexShaderFile = "shaders/MeshVertexShader.vert";
const string Mesh::fragmentShaderFile = "shaders/MeshFragmentShader.frag";

Mesh::Mesh()
{
	bbox = NULL;
}

Mesh::~Mesh()
{
	delete bbox;
	bbox = NULL;
}

Vertex* Mesh::addVertex(const glm::vec3 &position)
{
	int index = numVertices();
	Vertex *v = new Vertex(index, position);
	vertices.push_back(v);
	if (bbox == NULL) 
		bbox = new BoundingBox(position,position);
	else 
		bbox->Extend(position);
	return v;
}

HalfEdge* Mesh::getEdge(Vertex* a, Vertex* b) const
{
	for(int i=0; i<numHalfEdges(); i++)
	{
		//if there is such an edge, and it starts at a and ends at b, return it
		if(	
			(halfEdges[i] != NULL &&
			halfEdges[i]->getVertex() != NULL &&
			halfEdges[i]->getNext() != NULL &&
			halfEdges[i]->getNext()->getVertex() != NULL)

			&&
			(halfEdges[i]->getVertex() == a && halfEdges[i]->getNext()->getVertex() == b)
			)
			return halfEdges[i];
	}
	return NULL;
}

void Mesh::addFace(Vertex *a, Vertex *b, Vertex *c, const glm::vec3& col, const glm::vec3& emit)
{
	//make the face
	Face *newFace = new Face(col,emit);

	//make the edges
	HalfEdge* ea = new HalfEdge(a,newFace);
	HalfEdge* eb = new HalfEdge(b,newFace);
	HalfEdge* ec = new HalfEdge(c,newFace);

	//point the face to the first one
	newFace->setEdge(ea);

	// connect the edges to each other
	ea->setNext(eb);
	eb->setNext(ec);
	ec->setNext(ea);

	// add them to the halfEdge array
	halfEdges.push_back(ea);
	halfEdges.push_back(eb);
	halfEdges.push_back(ec);

	//get each half edge's oppposite edge if it exists
	HalfEdge* ea_opposite = getEdge(ea->getEndVertex(), ea->getVertex());
	HalfEdge* eb_opposite = getEdge(eb->getEndVertex(), eb->getVertex());
	HalfEdge* ec_opposite = getEdge(ec->getEndVertex(), ec->getVertex());

	//connect each half edge to it's opposite
	if (ea_opposite != NULL && ea_opposite != ea)
	{
		ea_opposite->clearOpposite();
		ea_opposite->setOpposite(ea);
	}
	if (eb_opposite != NULL && eb_opposite != eb)
	{
		eb_opposite->clearOpposite();
		eb_opposite->setOpposite(eb);
	}
	if (ec_opposite != NULL && ec_opposite != ec)
	{
		ec_opposite->clearOpposite();
		ec_opposite->setOpposite(ec);
	}

	//add the face to the face array
	faces.push_back(newFace);
}

void Mesh::removeFace(Face *f)
{
	//get the half-edges of the face
	HalfEdge *ea = f->getEdge();
	HalfEdge *eb = ea->getNext();
	HalfEdge *ec = eb->getNext();

	assert (ec->getNext() == ea);

	// remove those edges
	halfEdges.erase(remove(halfEdges.begin(), halfEdges.end(), ea), halfEdges.end());
	halfEdges.erase(remove(halfEdges.begin(), halfEdges.end(), eb), halfEdges.end());
	halfEdges.erase(remove(halfEdges.begin(), halfEdges.end(), ec), halfEdges.end());

	//remove the face
	faces.erase(remove(faces.begin(), faces.end(), f), faces.end());

	// clean up memory
	delete ea;
	delete eb;
	delete ec;
	delete f;
}

int Mesh::numVertices() const { return vertices.size(); }
int Mesh::numHalfEdges() const { return halfEdges.size(); }
int Mesh::numFaces() const { return faces.size(); }

Vertex* Mesh::getVertexByIndex(int index) const
{
	assert (index >= 0 && index < numVertices());
	for(int i=0; i<numVertices(); i++)
	{
		assert (vertices[i] != NULL);
		if (vertices[i]->getIndex() == index)
			return vertices[i];
	}
}

vector<Vertex*> Mesh::getVertices() const { return vertices; }
vector<HalfEdge*> Mesh::getHalfEdges() const { return halfEdges; }
vector<Face*> Mesh::getFaces() const { return faces; }

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

void parseObject(ifstream& fileStream, SceneObject& currentObject)
{
	string line;
	std::string prefix;
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
			const vector<string> tokens = split(line, " ", false);
			glm::vec3 vertex(stod(tokens[1]), stod(tokens[2]), stod(tokens[3]));
			currentObject.obj_model.vertices.push_back(vertex);
		}

		//load texture UV/W coordinates
		prefix = "vt ";
		if (!line.compare(0, prefix.size(), prefix))
		{
			const vector<string> tokens = split(line, " ", false);

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
			const vector<string> tokens = split(line, " ", false);
			glm::vec3 vertexNormal(stod(tokens[1]), stod(tokens[2]), stod(tokens[3]));
			currentObject.obj_model.vertexNormals.push_back(vertexNormal);
		}

		//load faces
		prefix = "f ";
		if (!line.compare(0, prefix.size(), prefix))
		{
			const vector<string> tokens = split(line, " ", false);
			ModelFace face;
			int numIndexes = tokens.size() - 1;
			if(tokens[1].find("//") != string::npos) //we have vertex//normals
			{
				face.vertexIndexes.resize(numIndexes);
				face.normalIndexes.resize(numIndexes);

				for(int i=1; i<tokens.size(); i++)
				{
					vector<string> tmp = split(tokens[i], "//", false);
					face.vertexIndexes[i-1] = stoi(tmp[0]) - 1;
					face.normalIndexes[i-1] = stoi(tmp[1]) - 1;
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
					face.vertexIndexes[i-1] = stoi(tmp[0]) - 1;
					face.textureIndexes[i-1] = stoi(tmp[1]) - 1;
					face.normalIndexes[i-1] = stoi(tmp[2]) - 1;
				}
			}
			else //we have vertex/texture
			{
				face.vertexIndexes.resize(numIndexes);
				face.textureIndexes.resize(numIndexes);

				for(int i=1; i<tokens.size(); i++)
				{
					vector<string> tmp = split(tokens[i], "/", false);
					face.vertexIndexes[i-1] = stoi(tmp[0]) - 1;
					face.textureIndexes[i-1] = stoi(tmp[1]) - 1;
				}
			}

			currentObject.obj_model.faces.push_back(face);
		}
	}
}

void Mesh::LoadToArrays(string input_file)
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

	while(!fileStream.eof())
	{
		//found another object, end parsing
		
			SceneObject currentObject;
			currentObject.obj_id = currentObjectId;
			parseObject(fileStream, currentObject);
			sceneModel.push_back(currentObject);
			currentObjectId ++;
		
	}

	sceneModel.erase(sceneModel.begin());

	
}

void Mesh::LoadFromArrays()
{
	//load all objects from the model
	for(int i=0; i< sceneModel.size(); i++)
	{
		//add all vertices of the object
		for(int j=0; j<sceneModel[i].obj_model.vertices.size(); j++)
		{
			addVertex(sceneModel[i].obj_model.vertices[j]);
		}
		//for each face, get all it's vertices and add the face to the mesh
		for(int j=0; j<sceneModel[i].obj_model.faces.size(); j++)
		{
			ModelFace currentFace = sceneModel[i].obj_model.faces[j];
			int numIndexes = currentFace.vertexIndexes.size();

			Vertex* a = getVertexByIndex(currentFace.vertexIndexes[0]);
			Vertex* b = getVertexByIndex(currentFace.vertexIndexes[1]);
			Vertex* c = getVertexByIndex(currentFace.vertexIndexes[2]);
			Vertex* d = NULL;

			//FOR DEBUG
			float color_r = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/1.0));
			float color_g = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/1.0));
			float color_b = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/1.0));

			glm::vec3 faceColor(color_r, color_g, color_b);
			glm::vec3 faceEmission(0.0, 0.0, 0.0);

			addFace(a, b, c, faceColor, faceEmission);
			if(numIndexes > 3)
			{
				d = getVertexByIndex(currentFace.vertexIndexes[3]);
				addFace(a, c, d, faceColor, faceEmission);
			}
		}
	}
}

void Mesh::Load(string input_file)
{
	LoadToArrays(input_file);
	LoadFromArrays();
}
/*
void Mesh::Subdivide()
{
	vector<Face*> startingFaces = faces;
	int numStartingFaces = startingFaces.size();

	for(int i=0; i<numStartingFaces; i++)
	{
		Face* currentFace = startingFaces[i];
		//for every face, get the 3 vertices, add a new vertex at their centroid, then delete the old face and add the 3 new ones

		//first get the centroid
		glm::vec3 centroid = faces[i]->getCentroid();

		//make a new vertex with that position
		Vertex* center = new Vertex(numVertices(), centroid);

		//add it to the back of the vertex array
		vertices.push_back(center);

		//then get the current face's 3 other vertices
		Vertex* a = (*faces[i])[0];
		Vertex* b = (*faces[i])[1];
		Vertex* c = (*faces[i])[2];

		//and the color and emission
		glm::vec3 color = faces[i]->getColor();
		glm::vec3 emission = faces[i]->getEmission();


		removeFace(faces[i]);

		//add the 3 new faces
		addFace(a, b, center, color, emission);
		addFace(b, c, center, color, emission);
		addFace(c, a, center, color, emission);
	}
}
*/

//WORKING!!!
void Mesh::Subdivide()
{
	vector<Face*> startingFaces;
	startingFaces.resize(faces.size());
	for(int i=0; i<faces.size(); i++)
	{
		startingFaces[i] = faces[i]->Clone();
	}
	int numStartingFaces = startingFaces.size();

	for(int i=0; i<numStartingFaces; i++)
	{
		//for every face, get the 3 vertices, add a new vertex at their centroid, then delete the old face and add the 3 new ones

		//first get the centroid
		glm::vec3 centroid = startingFaces[i]->getCentroid();

		//make a new vertex with that position
		Vertex* center = new Vertex(numVertices(), centroid);

		//add it to the back of the vertex array
		vertices.push_back(center);

		//then get the current face's 3 other vertices
		Vertex* a = getVertexByIndex((*startingFaces[i])[0]->getIndex());
		Vertex* b = getVertexByIndex((*startingFaces[i])[1]->getIndex());
		Vertex* c = getVertexByIndex((*startingFaces[i])[2]->getIndex());

		//and the color and emission
		glm::vec3 color = startingFaces[i]->getColor();
		glm::vec3 emission = startingFaces[i]->getEmission();
		
		removeFace(faces[i]);

		//add the 3 new faces
		addFace(a, b, center, color, emission);
		addFace(b, c, center, color, emission);
		addFace(c, a, center, color, emission);

		//delete the old one
		//removeFace(faces[i]);
	}
}


void Mesh::cacheVertexPositions()
{
	vertex_positions.clear();
	for(int i=0; i<vertices.size(); i++)
	{
		vertex_positions.push_back(vertices[i]->x());
		vertex_positions.push_back(vertices[i]->y());
		vertex_positions.push_back(vertices[i]->z());
	}
}

//WARNING! Call this last!!!
void Mesh::cacheVertexColors()
{
	vertex_colors.clear();
	vertex_colors.resize(face_indexes.size());
	for(int i=0; i<face_indexes.size(); i+= 3)
	{
		glm::vec3 faceColor = faces[i/3]->getColor();
		
		int index_a = face_indexes[i];
		int index_b = face_indexes[i+1];
		int index_c = face_indexes[i+2];

		vertex_colors[index_a] = faceColor.r;
		vertex_colors[index_a+1] = faceColor.g;
		vertex_colors[index_a+2] = faceColor.b;

		vertex_colors[index_b] = faceColor.r;
		vertex_colors[index_b+1] = faceColor.g;
		vertex_colors[index_b+2] = faceColor.b;

		vertex_colors[index_c] = faceColor.r;
		vertex_colors[index_c+1] = faceColor.g;
		vertex_colors[index_c+2] = faceColor.b;
	}
}

void Mesh::cacheFaceIndexes()
{
	face_indexes.clear();
	for(int i=0; i< faces.size(); i++)
	{
		HalfEdge* startEdge = faces[i]->getEdge();
		Vertex* start = startEdge->getVertex();

		face_indexes.push_back(start->getIndex());

		HalfEdge* currentEdge = startEdge->getNext();
		while(currentEdge->getVertex() != start)
		{
			face_indexes.push_back(currentEdge->getVertex()->getIndex());
			currentEdge = currentEdge->getNext();
		}
	}
}

void Mesh::DrawWireframe()
{
	glm::vec3 center = bbox->getCenter();
	float s = 1.0/bbox->maxDim();

	glScalef(s,s,s);
	glTranslatef(-center.x,-center.y,-center.z);

	// this offset prevents "z-fighting" bewteen the edges and faces
	// the edges will always win.
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.1,4.0);

	
		// for all other visualizations, just render the patch in a uniform color
		glEnable(GL_LIGHTING);
		glBegin (GL_LINES);
		for (int i = 0; i < numFaces(); i++)
		{
			Face *f = faces[i];
			glm::vec3 color = f->getColor();
			
			glColor3f(color.x, color.y, color.z);
			glm::vec3 a = (*f)[0]->getPosition();
			glm::vec3 b = (*f)[1]->getPosition();
			glm::vec3 c = (*f)[2]->getPosition();
			
			glVertex3f(a.x,a.y,a.z);
			glVertex3f(b.x,b.y,b.z);
			glVertex3f(c.x,c.y,c.z);
		}
		glEnd();
		
	
		

	glDisable(GL_POLYGON_OFFSET_FILL); 
	 

}

GLuint Mesh::LoadShaders()
{
	shaderProgramID = shaderLoader.LoadShaders(vertexShaderFile, fragmentShaderFile);
	return shaderProgramID;
}

void Mesh::PrepareToDraw()
{
	cacheVertexPositions();
	cacheFaceIndexes();
	cacheVertexColors();

	//create a vertex buffer
	glGenBuffers(1, &vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);

	//fill the vertex buffer with our mesh's data
	glBufferData(GL_ARRAY_BUFFER, vertex_positions.size() * sizeof(GLfloat), &vertex_positions[0], GL_STATIC_DRAW);

	//create a color buffer
	glGenBuffers(1, &colorBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
	//fill the color buffer with random values
	glBufferData(GL_ARRAY_BUFFER, vertex_colors.size() * sizeof(GLfloat), &vertex_colors[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices
	glGenBuffers(1, &elementBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_indexes.size() * sizeof(GLuint), &face_indexes[0], GL_STATIC_DRAW);

}

void Mesh::Cleanup()
{
	glDeleteBuffers(1, &vertexBufferID);
	glDeleteBuffers(1, &colorBufferID);
	glDeleteBuffers(1, &elementBufferID);
	glDeleteProgram(shaderProgramID);
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
	//this is the data we'll give as input to OpanGL
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