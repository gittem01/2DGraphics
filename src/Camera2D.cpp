#include <Camera2D.h>
#include <WindowHandler.h>

Camera2D::Camera2D(glm::vec2 pos, WindowHandler* wh) {
	this->pos = pos;
	this->wh = wh;
	this->window = wh->window;
}

void Camera2D::updateOrtho() {
	glm::vec2 xSides = defaultXSides / (zoom * zoom) + pos.x;
	glm::vec2 ySides = defaultYSides / (zoom * zoom) + pos.y;
	ortho = glm::ortho(xSides.x, xSides.y, ySides.x, ySides.y);
}

void Camera2D::changeZoom(float inc) {
	glm::vec2 mp = getCameraCoords(zoomPoint);

	float zoomAfter = limitZoom(zoom + inc);

	glm::vec2 xSidesBefore = defaultXSides / ((float)pow(zoom, 2)) + pos.x;
	glm::vec2 xSidesAfter = defaultXSides / ((float)pow(zoomAfter, 2)) + pos.x;

	glm::vec2 ySidesBefore = defaultYSides / ((float)pow(zoom, 2)) + pos.y;
	glm::vec2 ySidesAfter = defaultYSides / ((float)pow(zoomAfter, 2)) + pos.y;

	float xPerctBefore = (mp.x - pos.x) / ((float)xSidesBefore.y - xSidesBefore.x);
	float xPerctAfter = (mp.x - pos.x) / ((float)xSidesAfter.y - xSidesAfter.x);

	float yPerctBefore = (mp.y - pos.y) / ((float)ySidesBefore.y - ySidesBefore.x);
	float yPerctAfter = (mp.y - pos.y) / ((float)ySidesAfter.y - ySidesAfter.x);

	pos.x += (xPerctAfter - xPerctBefore) * (xSidesAfter.y - xSidesAfter.x);
	pos.y += (yPerctAfter - yPerctBefore) * (ySidesAfter.y - ySidesAfter.x);
	zoom += inc;
	zoom = limitZoom(zoom);
}

float Camera2D::limitZoom(float inZoom) {
	if (inZoom < zoomLimits.x) {
		inZoom = zoomLimits.x;
	}
	else if (inZoom > zoomLimits.y) {
		inZoom = zoomLimits.y;
	}
	return inZoom;
}

void Camera2D::update() {
	updateOrtho();

	int width, height;
	glfwGetWindowSize(window, &width, &height);

	dragFunc(width, height);
	if (wh->mouseData[5] != 0 && 
		!wh->keyData[GLFW_KEY_LEFT_CONTROL] &&
		!wh->keyData[GLFW_KEY_RIGHT_CONTROL] &&
		!wh->keyData[GLFW_KEY_LEFT_SHIFT]) {
		neededZoom += zoom * wh->mouseData[5] / 10.0f;
		zoomPoint.x = wh->mouseData[0]; zoomPoint.y = height - wh->mouseData[1];
	}

	if (neededZoom != 0) {
		changeZoom(zoomInc * neededZoom);
		neededZoom -= zoomInc*neededZoom;
	}
}

void Camera2D::dragFunc(int width, int height) {
	glm::vec2 diffVec = glm::vec2(dragTo->x - lastPos->x,
		dragTo->y - lastPos->y);
	glm::vec2 sideDiffs = glm::vec2(defaultXSides.y - defaultXSides.x,
		defaultYSides.y - defaultYSides.x);

	float lng = glm::length(diffVec);

	if (abs(lng) > 0) {
		pos.x -= diffVec.x * (sideDiffs.x / width) * dragSmth / (zoom * zoom);
		pos.y += diffVec.y * (sideDiffs.y / height) * dragSmth / (zoom * zoom);
		lastPos->x += diffVec.x * dragSmth;
		lastPos->y += diffVec.y * dragSmth;
	}


	if (wh->mouseData[4] == 2 && !wh->keyData[GLFW_KEY_LEFT_SHIFT]) {
		lastPos->x = wh->mouseData[0];
		lastPos->y = wh->mouseData[1];

		dragTo->x = wh->mouseData[0];
		dragTo->y = wh->mouseData[1];
	}
	else if (wh->mouseData[4] == 1 && !wh->keyData[GLFW_KEY_LEFT_SHIFT]) {
		dragTo->x = wh->mouseData[0];
		dragTo->y = wh->mouseData[1];
	}
	else if (wh->keyData[GLFW_KEY_LEFT_SHIFT]){
		pos.x -= wh->trackpadData[0] * (sideDiffs.x / width) * dragSmth / (zoom * zoom) * 5.0f;
		pos.y += wh->trackpadData[1] * (sideDiffs.y / height) * dragSmth / (zoom * zoom) * 5.0f;
	}
}

glm::vec2 Camera2D::getMouseCoords() {
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	float xPerct = wh->mouseData[0] / (float)width;
	float yPerct = 1 - wh->mouseData[1] / (float)height;

	glm::vec2 xSides = defaultXSides / (zoom * zoom) + pos.x;
	float xDiff = xSides.y - xSides.x;

	glm::vec2 ySides = defaultYSides / (zoom * zoom) + pos.y;
	float yDiff = ySides.y - ySides.x;

	float xPos = xSides.x + xPerct * xDiff;
	float yPos = ySides.x + yPerct * yDiff;

	return glm::vec2(xPos, yPos);
}

glm::vec2 Camera2D::getCameraCoords(glm::vec2 p) {
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	float xPerct = p.x / width;
	float yPerct = p.y / height;

	glm::vec2 xSides = defaultXSides / (zoom * zoom) + pos.x;
	float xDiff = xSides.y - xSides.x;

	glm::vec2 ySides = defaultYSides / (zoom * zoom) + pos.y;
	float yDiff = ySides.y - ySides.x;

	float xPos = xSides.x + xPerct * xDiff;
	float yPos = ySides.x + yPerct * yDiff;

	return glm::vec2(xPos, yPos);
}