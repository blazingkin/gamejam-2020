#include "Shape.h"
#include <iostream>
#include <assert.h>

#include "GLSL.h"
#include "Program.h"

using namespace std;

Shape::Shape() :
	eleBufID(0),
	posBufID(0),
	norBufID(0),
	texBufID(0), 
   vaoID(0)
{
	min = glm::vec3(0);
	max = glm::vec3(0);
	initialized = false;
}

Shape::~Shape()
{

}

/* copy the data from the shape to this object */
void Shape::createShape(tinyobj::shape_t & shape)
{
		posBuf = shape.mesh.positions;
		norBuf = shape.mesh.normals;
		texBuf = shape.mesh.texcoords;
		eleBuf = shape.mesh.indices;

		if (norBuf.size() == 0) {
			/* Calculate normals based on the mesh */
			/* Initialize to 0 */
			for (size_t i = 0; i < posBuf.size(); i++) {
				norBuf.push_back(0);
			}

			/* For each face, add the cross product of the vertices to each vertex */
			glm::vec3 a, b, c;
			for (size_t f = 0; f < eleBuf.size(); f += 3) {
				int aInd = eleBuf[f] * 3; int bInd = eleBuf[f + 1] * 3; int cInd = eleBuf[f + 2] * 3;
				a.x = posBuf[aInd]; a.y = posBuf[aInd + 1]; a.z = posBuf[aInd + 2];
				b.x = posBuf[bInd]; b.y = posBuf[bInd + 1]; b.z = posBuf[bInd + 2];
				c.x = posBuf[cInd]; c.y = posBuf[cInd + 1]; c.z = posBuf[cInd + 2];

				glm::vec3 u = b - a;
				glm::vec3 v = c - a;
				glm::vec3 faceNormal = glm::cross(u, v);
				norBuf[aInd] += faceNormal.x; norBuf[bInd] += faceNormal.x; norBuf[cInd] += faceNormal.x;
				norBuf[aInd + 1] += faceNormal.y; norBuf[bInd + 1] += faceNormal.y; norBuf[cInd + 1] += faceNormal.y;
				norBuf[aInd + 2] += faceNormal.z; norBuf[bInd + 2] += faceNormal.z; norBuf[cInd + 2] += faceNormal.z;
			}

			/* Normalize the norms */
			for (size_t i = 0; i < norBuf.size(); i+= 3) {
				glm::vec3 n;
				n.x = norBuf[i]; n.y = norBuf[i + 1]; n.z = norBuf[i + 2];
				glm::normalize(n);
				norBuf[i] = n.x; norBuf[i + 1] = n.y; norBuf[i + 2] = n.z;
			}

		}
}

void Shape::measure() {
  float minX, minY, minZ;
   float maxX, maxY, maxZ;

   minX = minY = minZ = 1.1754E+38F;
   maxX = maxY = maxZ = -1.1754E+38F;

   //Go through all vertices to determine min and max of each dimension
   for (size_t v = 0; v < posBuf.size() / 3; v++) {
		if(posBuf[3*v+0] < minX) minX = posBuf[3*v+0];
		if(posBuf[3*v+0] > maxX) maxX = posBuf[3*v+0];

		if(posBuf[3*v+1] < minY) minY = posBuf[3*v+1];
		if(posBuf[3*v+1] > maxY) maxY = posBuf[3*v+1];

		if(posBuf[3*v+2] < minZ) minZ = posBuf[3*v+2];
		if(posBuf[3*v+2] > maxZ) maxZ = posBuf[3*v+2];
	}

	min.x = minX;
	min.y = minY;
	min.z = minZ;
   max.x = maxX;
   max.y = maxY;
   max.z = maxZ;
}

void Shape::init()
{
	// Initialize the vertex array object
	CHECKED_GL_CALL(glGenVertexArrays(1, &vaoID));
	CHECKED_GL_CALL(glBindVertexArray(vaoID));

	// Send the position array to the GPU
	CHECKED_GL_CALL(glGenBuffers(1, &posBufID));
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW));
	
	// Send the normal array to the GPU
	if(norBuf.empty()) {
		norBufID = 0;
	} else {
		CHECKED_GL_CALL(glGenBuffers(1, &norBufID));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW));
	}
	
	// Send the texture array to the GPU
	if(texBuf.empty()) {
		texBufID = 0;
	} else {
		CHECKED_GL_CALL(glGenBuffers(1, &texBufID));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufID));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW));
	}
	
	// Send the element array to the GPU
	CHECKED_GL_CALL(glGenBuffers(1, &eleBufID));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));
	CHECKED_GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW));
	
	// Unbind the arrays
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	
	assert(glGetError() == GL_NO_ERROR);
	initialized = true;
}

void Shape::free() {
	if (initialized) {
		glDeleteVertexArrays(1, &vaoID);
		glDeleteBuffers(1, &posBufID);
		if (!norBuf.empty()) {
			glDeleteBuffers(1, &norBufID);
		}
		if (!texBuf.empty()) {
			glDeleteBuffers(1, &texBufID);
		}
		glDeleteBuffers(1, &eleBufID);
		vaoID = posBufID = norBufID = texBufID = eleBufID = 0;
		initialized = false;
	}
}

void Shape::shift(glm::vec3 offset) {
	for (auto i = 0; i < posBuf.size(); i += 3) {
		posBuf[i] += offset.x;
		posBuf[i + 1] += offset.y;
		posBuf[i + 2] += offset.z;
	}

	// Make sure to update all of the data in the gpu
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*posBuf.size(), &posBuf[0]));
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void Shape::scale(glm::vec3 scale) {
	for (size_t i = 0; i < posBuf.size(); i += 3) {
		posBuf[i] *= scale.x;
		posBuf[i + 1] *= scale.y;
		posBuf[i + 2] *= scale.z;
	}
	// Make sure to update all of the data in the gpu
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*posBuf.size(), &posBuf[0]));
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void Shape::update(tinyobj::shape_t & shape) {
	if (shape.mesh.positions.size() != posBuf.size()) {
		cout << "Cannot add or remove vertices in update!" << endl;
		return;
	}
	posBuf = shape.mesh.positions;
	norBuf = shape.mesh.normals;
	texBuf = shape.mesh.texcoords;
	eleBuf = shape.mesh.indices;

	if (norBuf.size() == 0) {
		/* Calculate normals based on the mesh */
		/* Initialize to 0 */
		for (size_t i = 0; i < posBuf.size(); i++) {
			norBuf.push_back(0);
		}

		/* For each face, add the cross product of the vertices to each vertex */
		glm::vec3 a, b, c, u, v;
		for (size_t f = 0; f < eleBuf.size(); f += 3) {
			int aInd = eleBuf[f] * 3; int bInd = eleBuf[f + 1] * 3; int cInd = eleBuf[f + 2] * 3;
			a.x = posBuf[aInd]; a.y = posBuf[aInd + 1]; a.z = posBuf[aInd + 2];
			b.x = posBuf[bInd]; b.y = posBuf[bInd + 1]; b.z = posBuf[bInd + 2];
			c.x = posBuf[cInd]; c.y = posBuf[cInd + 1]; c.z = posBuf[cInd + 2];

			glm::vec3 u = b - a;
			glm::vec3 v = c - a;
			glm::vec3 faceNormal = glm::cross(u, v);
			norBuf[aInd] += faceNormal.x; norBuf[bInd] += faceNormal.x; norBuf[cInd] += faceNormal.x;
			norBuf[aInd + 1] += faceNormal.y; norBuf[bInd + 1] += faceNormal.y; norBuf[cInd + 1] += faceNormal.y;
			norBuf[aInd + 2] += faceNormal.z; norBuf[bInd + 2] += faceNormal.z; norBuf[cInd + 2] += faceNormal.z;
		}

		/* Normalize the norms */
		for (size_t i = 0; i < norBuf.size(); i+= 3) {
			glm::vec3 n;
			n.x = norBuf[i]; n.y = norBuf[i + 1]; n.z = norBuf[i + 2];
			glm::normalize(n);
			norBuf[i] = n.x; norBuf[i + 1] = n.y; norBuf[i + 2] = n.z;
		}

	}

	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*posBuf.size(), &posBuf[0]));
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
	CHECKED_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*norBuf.size(), &norBuf[0]));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));
	CHECKED_GL_CALL(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(float)*eleBuf.size(), &eleBuf[0]));
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));

}

void Shape::draw(const shared_ptr<Program> prog) const
{
	int h_pos, h_nor, h_tex;
	h_pos = h_nor = h_tex = -1;

   glBindVertexArray(vaoID);
	// Bind position buffer
	h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
	
	// Bind normal buffer
	if (prog->hasAttribute("vertNor")) {
		h_nor = prog->getAttribute("vertNor");
		if(h_nor != -1 && norBufID != 0) {
			GLSL::enableVertexAttribArray(h_nor);
			CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
			CHECKED_GL_CALL(glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
		}
	}

	if (prog->hasAttribute("vertTex")) {
		if (texBufID != 0) {	
			// Bind texcoords buffer
			h_tex = prog->getAttribute("vertTex");
			if(h_tex != -1 && texBufID != 0) {
				GLSL::enableVertexAttribArray(h_tex);
				CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufID));
				CHECKED_GL_CALL(glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0));
			}
		}
	}
	
	// Bind element buffer
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));
	
	// Draw
	CHECKED_GL_CALL(glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0));
	
	// Disable and unbind
	if(h_tex != -1) {
		GLSL::disableVertexAttribArray(h_tex);
	}
	if(h_nor != -1) {
		GLSL::disableVertexAttribArray(h_nor);
	}
	GLSL::disableVertexAttribArray(h_pos);
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}


void Shape::drawInstances(const shared_ptr<Program> prog, int instances, instance_render_data_t *data) const
{
	int h_pos, h_nor, h_tex, h_M;
	h_pos = h_nor = h_tex = -1;

    glBindVertexArray(vaoID);

	for (auto it = data->attributes.begin(); it != data->attributes.end(); it++) {
		prog->setInstanceValues(it->first, it->second);
    }
	if (prog->hasAttribute("M")) {
		prog->setInstanceModels(&data->M);
	}

	// Bind position buffer
	h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
	
	// Bind normal buffer
	if (prog->hasAttribute("vertNor")) {
		h_nor = prog->getAttribute("vertNor");
		if(h_nor != -1 && norBufID != 0) {
			GLSL::enableVertexAttribArray(h_nor);
			CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
			CHECKED_GL_CALL(glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
		}
	}

	if (texBufID != 0) {	
		// Bind texcoords buffer
		h_tex = prog->getAttribute("vertTex");
		if(h_tex != -1 && texBufID != 0) {
			GLSL::enableVertexAttribArray(h_tex);
			CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufID));
			CHECKED_GL_CALL(glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0));
		}
	}

	
	// Bind element buffer
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));
	

	
	// Draw
	CHECKED_GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0, instances));
	
	// Disable and unbind
	if(h_tex != -1) {
		GLSL::disableVertexAttribArray(h_tex);
	}
	if(h_nor != -1) {
		GLSL::disableVertexAttribArray(h_nor);
	}
	GLSL::disableVertexAttribArray(h_pos);
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
