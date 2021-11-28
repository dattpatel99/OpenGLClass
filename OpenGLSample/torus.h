#pragma once
#include <GLFW/glfw3.h>
class Torus {
public:
	Torus();
	void setCoords(double r, double c, int rSeg, int cSeg, int i, int j, GLfloat* vertices, GLfloat* uv);
	int createObject(double r, double c, int rSeg, int cSeg, GLfloat** vertices, GLfloat** uv);
	int getVertices() { return this->vertices; };
private:
	int vertices;
};