#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <WindowHandler.h>

class Camera2D
{

public:
	glm::vec2 pos;
	glm::vec2 defaultXSides = glm::vec2(-8.0f, +8.0f);
	glm::vec2 defaultYSides = glm::vec2(-8.0f, +8.0f);
	
	float baseX = 16.0f;
	float baseY = 16.0f;

	glm::vec2 zoomLimits = glm::vec2(0.5f, 10.0f);

	glm::mat4 ortho;

	GLFWwindow* window;
	float zoom = 1.0f;
	float zoomInc = 0.2;
	float dragSmth = 0.5f;
	float neededZoom = 0;
	glm::vec2 zoomPoint;
	glm::vec2* lastPos = new glm::vec2(-1, -1);
	glm::vec2* dragTo = new glm::vec2(0, 0);

	WindowHandler* wh;

	Camera2D(glm::vec2 pos, WindowHandler* wh);
	void update();
	glm::vec2 getCameraCoords(glm::vec2 p);
	glm::vec2 getMouseCoords();
	void updateOrtho();
	void changeZoom(float inc);
	float limitZoom(float inZoom);
	void dragFunc(int width, int height);
	void cursorOutFunc(int width, int height);
};

