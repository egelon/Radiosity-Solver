#include "Mesh.h"



#include <glm/gtc/matrix_transform.hpp>


#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>



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

void Mesh::LoadToArrays(string input_file)
{
	ifstream fileStream(input_file, ios::in);

	if (!fileStream)
	{
		cout << "ERROR: cannot open file " << input_file << endl;
		exit(1);
	}

	string line;
	while(getline(fileStream, line))
	{

		//if we read a vertex
		if(line.substr(0, 2) == "v ")
		{
			istringstream str(line.substr(2));
			float x,y,z;

			str >> x >> y >> z;
			glm::vec3 vertex(x, y, z);

			file_vertices.push_back(vertex);
		}
		//if we read a face
		else if(line.substr(0, 2) == "f ")
		{
			istringstream str(line.substr(2));
			GLshort a,b,c;
			str >> a >> b >> c;
			(a<0)? a*= -1.0 : a=a;
			(b<0)? b*= -1.0 : b=b;
			(c<0)? c*= -1.0 : c=c;
			file_elements.push_back(a - 1);
			file_elements.push_back(b - 1);
			file_elements.push_back(c - 1);
		}
		else if (line[0] == '#') { /* ignoring this line */ }
		else { /* ignoring this line */ }
	}

	file_normals.resize(file_vertices.size(), glm::vec3(0.0, 0.0, 0.0));

	for(int i=0; i<file_elements.size(); i+=3)
	{
		GLushort ia = file_elements[i];
		GLushort ib = file_elements[i+1];
		GLushort ic = file_elements[i+2];

		glm::vec3 normal = glm::normalize(glm::cross(
			glm::vec3(file_vertices[ib]) - glm::vec3(file_vertices[ia]),
			glm::vec3(file_vertices[ic]) - glm::vec3(file_vertices[ia])
			));
		file_normals[ia] = normal;
		file_normals[ib] = normal;
		file_normals[ic] = normal;
	}

	for(int i=0; i<file_vertices.size(); i++)
	{
		float r = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/1.0f));
		float g = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/1.0f));
		float b = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/1.0f));
		file_colors.push_back(glm::vec3(r, g, b));
	}
}

void Mesh::LoadFromArrays()
{
	//to create a mesh:
	//first add the 3 vertices for a face
	//then add a face from them

	//load all vertices
	for(int i=0; i<file_vertices.size(); i++)
	{
		addVertex(file_vertices[i]);
	}

	//for each face, get it's 3 vertices and add the face to the mesh
	for(int i=0; i<file_elements.size(); i+=3)
	{
		GLuint index_a = file_elements[i];
		GLuint index_b = file_elements[i+1];
		GLuint index_c = file_elements[i+2];

		Vertex* a = getVertexByIndex(index_a);
		Vertex* b = getVertexByIndex(index_b);
		Vertex* c = getVertexByIndex(index_c);

		glm::vec3 faceColor(1.0, 1.0, 1.0);
		glm::vec3 faceEmission(0.0, 0.0, 0.0);

		addFace(a, b, c, file_colors[index_a], faceEmission);
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