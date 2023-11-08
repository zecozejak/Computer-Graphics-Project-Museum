#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include "glm/glm.hpp"
#include <GL/glew.h>

class Model
{
public:
	std::vector<glm::vec4> modelVerts;		// Wspó³rzêdnie wierczho³ków
	std::vector<glm::vec4> modelNorms;		// Wektory normalne
	std::vector<glm::vec2> modelTexCoords;	// Wspó³rzêdne teksturowania
	std::vector<unsigned int> modelIndices;	// Inedksy?
	GLuint tex0;							// Przechowywana tekstura
};

#endif
