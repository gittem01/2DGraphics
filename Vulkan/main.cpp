#include "ThinDrawer.h"

int main()
{
    ThinDrawer* thinDrawer = new ThinDrawer();

    while (!glfwWindowShouldClose(thinDrawer->window))
    {
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}