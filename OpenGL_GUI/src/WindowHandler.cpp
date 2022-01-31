#include "WindowHandler.h"
#include <Camera2D.h>

WindowHandler::WindowHandler(Camera2D* cam)
{
    this->cam = cam;
    this->massInit();

    // starting cursor from the middle of the screen instead of the top left corner
    this->mouseData[0] = DBL_MAX;
    this->mouseData[1] = DBL_MAX;
}

void WindowHandler::handleMouseData()
{
    this->mouseData[5] = 0;
    this->trackpadData[0] = 0;
    this->trackpadData[1] = 0;
    for (int i = 2; i < 5; i++) {
        if (this->mouseData[i] == 2) {
            this->mouseData[i] = 1;
        }
        if (releaseQueue[i - 2]){
            mouseData[i] = 0;
            releaseQueue[i - 2] = false;
        }
    }
    if (lastMousePos[0] == mouseData[0] && lastMousePos[1] == mouseData[1]) {
        mouseData[6] = 0;
    }
    else {
        mouseData[6] = 1;
    }
}

void WindowHandler::handleKeyData()
{
    for (int i : newPressIndices) {
        keyData[i] = 1;
    }
    newPressIndices.clear();
}

bool WindowHandler::looper()
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    handleMouseData();
    handleKeyData();
    
    
    glfwPollEvents();

    bool done = glfwWindowShouldClose(window);  

    return done;
}

void WindowHandler::massInit()
{
    if (!glfwInit()) {
        printf("GLFW init error\n");
        std::exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(windowSizes.x, windowSizes.y, "Auto Zoom", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        printf("Window creation error\n");
        std::exit(-1);
    }

    glfwSetWindowAspectRatio(window, windowSizes.x, windowSizes.y);

    glfwGetWindowContentScale(window, &dpiScaling, NULL);

    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(this->window, WindowHandler::mouseEventCallback);
    glfwSetMouseButtonCallback(this->window, WindowHandler::buttonEventCallback);
    glfwSetScrollCallback(this->window, WindowHandler::scrollEventCallback);
    glfwSetWindowFocusCallback(this->window, WindowHandler::glfwWindowFocusCallback);
    glfwSetKeyCallback(this->window, WindowHandler::glfwKeyEventCallback);
    glfwSetWindowSizeCallback(this->window, WindowHandler::windowSizeEventCallback);
    glfwSetWindowUserPointer(window, this);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD");
        std::exit(-1);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, (int)(windowSizes.x * dpiScaling), (int)(windowSizes.y * dpiScaling));
}


void WindowHandler::mouseEventCallback(GLFWwindow* window, double xpos, double ypos)
{
    WindowHandler* thisClass = (WindowHandler*)glfwGetWindowUserPointer(window);
    thisClass->mouseData[0] = xpos;
    thisClass->mouseData[1] = ypos;
}

void WindowHandler::buttonEventCallback(GLFWwindow* window, int button, int action, int mods)
{
    WindowHandler* thisClass = (WindowHandler*)glfwGetWindowUserPointer(window);
    if (action){
        thisClass->mouseData[button + 2] = 2;
    }
    else {
        thisClass->releaseQueue[button] = true;
    }
}

void WindowHandler::scrollEventCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    WindowHandler* thisClass = (WindowHandler*)glfwGetWindowUserPointer(window);
    thisClass->mouseData[5] = yoffset;
    thisClass->trackpadData[0] = xoffset;
    thisClass->trackpadData[1] = yoffset;
}

void WindowHandler::glfwKeyEventCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    WindowHandler* thisClass = (WindowHandler*)glfwGetWindowUserPointer(window);
    if (action == 1) {
        thisClass->keyData[key] = 2;
        thisClass->newPressIndices.insert(key);
    }
    else if (!action) {
        thisClass->keyData[key] = 0;
    }
}

void WindowHandler::glfwWindowFocusCallback(GLFWwindow* window, int isFocused)
{
    WindowHandler* thisClass = (WindowHandler*)glfwGetWindowUserPointer(window);
    if (!isFocused) {
        thisClass->handleMouseData();
    }
}

void WindowHandler::windowSizeEventCallback(GLFWwindow* window, int width, int height)
{
    WindowHandler* thisClass = (WindowHandler*)glfwGetWindowUserPointer(window);
    if (width < 500){
        width = 500;
        height = (thisClass->cam->baseY / thisClass->cam->baseX) * 500;
        glfwSetWindowSize(window, width, height);
    }

    glViewport(0, 0, (int)(width * thisClass->dpiScaling), (int)(height * thisClass->dpiScaling));
    thisClass->windowSizes.x = width;
    thisClass->windowSizes.y = height;

    float ratio = (float)height / width;

    thisClass->cam->defaultYSides = glm::vec2(-thisClass->cam->baseX * ratio * 0.5f, thisClass->cam->baseX * ratio * 0.5f);
    thisClass->cam->update();
    thisClass->shouldRender = true;
}