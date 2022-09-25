#include "ThinDrawer.h"

int main()
{
    ThinDrawer* thinDrawer = new ThinDrawer();

    while (!glfwWindowShouldClose(thinDrawer->window))
    {
        glfwPollEvents();
        thinDrawer->renderLoop();
    }

    glfwTerminate();

    return 0;
}