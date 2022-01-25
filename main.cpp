#include <BoxTest.h>

int main(void)
{
    WindowHandler* wh = new WindowHandler(NULL);
    Camera2D* cam = new Camera2D(glm::vec2(0, 0), wh);
    wh->cam = cam;

    Boxy* box = new Boxy(glm::vec2(0, 0), glm::vec2(2, 2));

    bool done = false;
    while (!done){
        glfwSwapInterval(1);

        glClear(GL_COLOR_BUFFER_BIT);

        done = wh->looper();
        
        cam->update();

        box->update(wh->cam);

        //box->pos = cam->getMouseCoords();

        glfwSwapBuffers(wh->window);
    }

    glfwTerminate();
    return 0;
}