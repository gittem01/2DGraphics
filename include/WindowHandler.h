#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <set>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Camera2D;

class WindowHandler
{
public:
	int* keyData = (int*)calloc(512, sizeof(int));
	int* mouseData = (int*)calloc(7, sizeof(int));
	int* trackpadData = (int*)calloc(2, sizeof(int));
	int* lastMousePos = (int*)calloc(2, sizeof(int));
	glm::vec2 windowSizes = glm::vec2(1280, 720);
	bool releaseQueue[3] = {false, false, false};
	float dpiScaling = 1.0f;
	std::set<int> newPressIndices;
	GLFWwindow* window;
	Camera2D* cam;

	WindowHandler(Camera2D* cam);

	void massInit();
	bool looper();
	void handleMouseData();
	void handleKeyData();

	static void mouseEventCallback(GLFWwindow*, double, double);
	static void buttonEventCallback(GLFWwindow*, int, int, int);
	static void scrollEventCallback(GLFWwindow*, double, double);
	static void glfwKeyEventCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void windowSizeEventCallback(GLFWwindow*, int, int);
	static void glfwWindowFocusCallback(GLFWwindow*, int);
};