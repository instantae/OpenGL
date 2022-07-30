#include "common_includes.h"

struct Cubes
{
	Cubes()
	{
	}

	Cubes(glm::vec3 pos, glm::vec3 scale, glm::vec3 rot, glm::vec4 rgba) : position(pos), scale(scale), rotation(rot), color(rgba)
	{
		this->calcMatrix();
	}

	glm::vec3 position = glm::vec3(0, 0, 0);
	glm::vec3 scale = glm::vec3(1, 1, 1);
	glm::vec3 rotation = glm::vec3(0, 0, 0);
	glm::vec4 color = glm::vec4(1.0);

	glm::mat4 modelMatrix = glm::mat4(1.0f);

	glm::vec3 const GetPosition() { return position; }

	void calcMatrix()
	{
		modelMatrix = glm::translate(glm::mat4(1.0f), position);
		modelMatrix = glm::rotate(modelMatrix, rotation.x, glm::vec3(1.0, 0.0, 0.0));
		modelMatrix = glm::rotate(modelMatrix, rotation.y, glm::vec3(0.0, 1.0, 0.0));
		modelMatrix = glm::rotate(modelMatrix, rotation.z, glm::vec3(0.0, 0.0, 1.0));
		modelMatrix = glm::scale(modelMatrix, scale);
	}
};