#include <math.h>
#include "Shader.h"
#include "Camera2D.h"
#include <vector>

class Boxy
{

public:
    static unsigned int VAO;

	Shader* shader;

    glm::vec2 pos;
	float rotation;
	glm::vec2 size;
	glm::vec4 colour;
    
    Boxy(glm::vec2 pos, glm::vec2 size)
    {
        this->pos = glm::vec2(pos.x, pos.y);
        this->rotation = 0.0f;
        this->size = glm::vec2(size.x, size.y);
        this->colour = glm::vec4(1, 1, 1, 1);

        this->shader = new Shader("../assets/shaders/basic/");
    }

    void update(Camera2D* cam)
    {
        shader->use();
        glm::mat4 ortho = cam->ortho;
        glm::mat4 model = this->getModel();

        shader->setMat4("ortho", ortho);
        shader->setMat4("model", model);

        shader->setVec4("colour", colour.x, colour.y, colour.z, colour.w);

        glBindVertexArray(Boxy::VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        colour.w = colour.w + ((float)rand() / RAND_MAX) * 0.10f;
        if (colour.w > 1.0f) colour.w = 0.0f;
    }

    glm::mat4 getModel()
    {

        glm::mat4 model = glm::mat4(1);

        model = glm::translate(model, glm::vec3(pos.x, pos.y, 0));
        model = glm::scale(model, glm::vec3(size.x, size.y, 0));
        return model;
    }


    static unsigned int getDefaultVAO()
    {
        float viis[] = {
            // Vertex positions
            -0.5f, +0.5f,
            +0.5f, +0.5f,
            +0.5f, -0.5f,
            +0.5f, -0.5f,
            -0.5f, -0.5f,
            -0.5f, +0.5f,
        };
        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0,
        };

        unsigned int tVAO;

        glGenVertexArrays(1, &tVAO);
        glBindVertexArray(tVAO);

        unsigned int VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(viis), viis, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        return tVAO;
    }
};