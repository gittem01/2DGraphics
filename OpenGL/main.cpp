#include <BoxTest.h>

#define IMG_WIDTH 160 // 16 * 30
#define IMG_HEIGHT 160 // 16 * 30

#define numOfBoxes IMG_WIDTH * IMG_HEIGHT

unsigned int Boxy::VAO;

enum drawType{
    INSTANCED,
    NON_INSTANCED,
};


Camera2D* cam;

glm::vec2 positions[numOfBoxes];
glm::vec2 sizes[numOfBoxes];
glm::vec4 colours[numOfBoxes];

Boxy* boxes[numOfBoxes];
Boxy* instancedBox;
Shader* instancedShader;

GLuint pos_SBO;
GLuint colour_SBO;
GLuint size_SBO;

double getRand01(){ return (double)rand() / RAND_MAX; }

void setCommonVariables()
{
    Boxy::VAO = Boxy::getDefaultVAO();

    for(int i = 0; i < numOfBoxes; i++){
        //positions[i] = glm::vec2(getRand01() * 160 - 80, getRand01() * 90 - 45);
        sizes[i] = glm::vec2(cam->baseX / IMG_WIDTH, cam->baseX / IMG_WIDTH);
        colours[i] = glm::vec4(getRand01(), getRand01(), getRand01(), getRand01());
    }
    for (int i = 0; i < IMG_WIDTH; i++){
        float camPosX = (((float)i / IMG_WIDTH) * cam->baseX) - cam->baseX/2.0f + (cam->baseX / 2.0f) / IMG_WIDTH;
        for (int j = 0; j < IMG_HEIGHT; j++){
            float camPosY = (((float)j / IMG_HEIGHT) * cam->baseY) - (cam->baseY / 2.0f) +(cam->baseX / 2.0f) / IMG_WIDTH;
            positions[j * IMG_WIDTH + i] = glm::vec2(camPosX, camPosY);
        }
    }
}

void boxInit()
{
    for (int i = 0; i < numOfBoxes; i++){
        Boxy* box = new Boxy(positions[i], sizes[i]);
        box->colour = colours[i];
        boxes[i] = box;
    }
}

void boxyUpdate()
{
    for (int i = 0; i < numOfBoxes; i++){
        boxes[i]->update(cam);
    }
}

void instancedInit()
{
    instancedShader = new Shader("../assets/shaders/instanced/");
    instancedBox = new Boxy(glm::vec2(), glm::vec2());

    glGenBuffers(1, &pos_SBO);
    glBindBuffer(GL_ARRAY_BUFFER, pos_SBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glGenBuffers(1, &size_SBO);
    glBindBuffer(GL_ARRAY_BUFFER, size_SBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sizes), sizes, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glGenBuffers(1, &colour_SBO);
    glBindBuffer(GL_ARRAY_BUFFER, colour_SBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colours), colours, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
}

void instancedUpdate()
{
    instancedShader->use();
    glm::mat4 ortho = cam->ortho;

    instancedShader->setMat4("ortho", ortho);

    for (int i = 0; i < numOfBoxes; i++){
        colours[i].w += ((float)rand() / RAND_MAX) * 0.10f;
        if (colours[i].w > 1.0f) colours[i].w = 0.0f;
    }

    glBindBuffer(colour_SBO, GL_ARRAY_BUFFER);
    void *ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    memcpy(ptr, colours, sizeof(colours));
    glUnmapBuffer(GL_ARRAY_BUFFER);

    glBindVertexArray(instancedBox->VAO);
    glBindBuffer(pos_SBO, GL_ARRAY_BUFFER);
    glBindBuffer(size_SBO, GL_ARRAY_BUFFER);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, numOfBoxes);
}

int main(void)
{
    WindowHandler* wh = new WindowHandler(NULL);
    cam = new Camera2D(glm::vec2(0, 0), wh);
    wh->cam = cam;

    setCommonVariables();

    boxInit();
    instancedInit();

    drawType dt = INSTANCED;
    int switchKey = GLFW_KEY_SPACE;

    bool done = false;
    while (!done){
        glfwSwapInterval(1);

        glClear(GL_COLOR_BUFFER_BIT);

        done = wh->looper();
        
        cam->update();

        if (wh->keyData[switchKey] == 2){
            // switch between instanced and non-instanced (credit : github copilot)
            if (dt == INSTANCED){
                for (int i = 0; i < numOfBoxes; i++){
                    boxes[i]->colour.w = colours[i].w;
                }
                dt = NON_INSTANCED;
            }
            else{
                for (int i = 0; i < numOfBoxes; i++){
                    colours[i].w = boxes[i]->colour.w;
                }
                dt = INSTANCED;
            }
        }

        switch (dt){
            case INSTANCED:
                instancedUpdate();
                break;
            case NON_INSTANCED:
                boxyUpdate();
                break;
        }

        glfwSwapBuffers(wh->window);
    }

    glfwTerminate();
    return 0;
}