#include "ThinDrawer.h"

int main()
{
    ThinDrawer* thinDrawer = new ThinDrawer();

    while (!glfwWindowShouldClose(thinDrawer->wh->window))
    {
        thinDrawer->renderLoop();
    }

    glfwTerminate();

    return 0;
}