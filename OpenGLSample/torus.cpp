// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "torus.h";

using namespace glm;

Torus::Torus() {}

void Torus::setCoords(double r, double c, int rSeg, int cSeg, int i, int j, GLfloat* vertices, GLfloat* uv) {
	const double PI = 3.1415926535897932384626433832795;
	const double TAU = 2 * PI;

	double x = (c + r * cos(i * TAU / rSeg)) * cos(j * TAU / cSeg);
	double y = (c + r * cos(i * TAU / rSeg)) * sin(j * TAU / cSeg);
	double z = r * sin(i * TAU / rSeg);

	vertices[0] = 2 * x;
	vertices[1] = 2 * y;
	vertices[2] = 2 * z;

	uv[0] = i / (double)rSeg;
	uv[1] = j / (double)cSeg;
}

int Torus::createObject(double r, double c, int rSeg, int cSeg, GLfloat** vertices, GLfloat** uv) {
	int count = rSeg * cSeg * 6;
	*vertices = (GLfloat*)malloc(count * 3 * sizeof(GLfloat));
	*uv = (GLfloat*)malloc(count * 2 * sizeof(GLfloat));

	for (int x = 0; x < cSeg; x++) { // through stripes
		for (int y = 0; y < rSeg; y++) { // through squares on stripe
			GLfloat* vertexPtr = *vertices + ((x * rSeg) + y) * 18;
			GLfloat* uvPtr = *uv + ((x * rSeg) + y) * 12;
			setCoords(r, c, rSeg, cSeg, x, y, vertexPtr + 0, uvPtr + 0);
			setCoords(r, c, rSeg, cSeg, x + 1, y, vertexPtr + 3, uvPtr + 2);
			setCoords(r, c, rSeg, cSeg, x, y + 1, vertexPtr + 6, uvPtr + 4);

			setCoords(r, c, rSeg, cSeg, x, y + 1, vertexPtr + 9, uvPtr + 6);
			setCoords(r, c, rSeg, cSeg, x + 1, y, vertexPtr + 12, uvPtr + 8);
			setCoords(r, c, rSeg, cSeg, x + 1, y + 1, vertexPtr + 15,
				uvPtr + 10);
		}
	}

	this->vertices = count;
	
	return count;
}