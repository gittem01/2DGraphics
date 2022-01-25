#include <math.h>
#include "Shader.h"
#include "Camera2D.h"
#include <vector>

class Boxy{

public:
    unsigned int VAO;
	unsigned int TEX;
	unsigned int id;

	Shader* shader;

    glm::vec2 pos;
	float rotation;
	glm::vec2 size;
	glm::vec4 color;
	Boxy(glm::vec3 pos, glm::vec3 size);


    Boxy(glm::vec2 pos, glm::vec2 size) {
        this->VAO = this->getDefaultVAO();

        this->pos = glm::vec2(pos.x, pos.y);
        this->rotation = 0.0f;
        this->size = glm::vec2(size.x, size.y);
        this->color = glm::vec4(1, 1, 1, 1);

        this->shader = new Shader("../assets/shaders/basic/");
    }

    void update(Camera2D* cam) {
        shader->use();
        glm::mat4 ortho = cam->ortho;
        glm::mat4 model = this->getModel();

        shader->setMat4("ortho", ortho);
        shader->setMat4("model", model);

        glBindVertexArray(this->VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glm::mat4 getModel() {

        glm::mat4 model = glm::mat4(1);

        model = glm::translate(model, glm::vec3(pos.x, pos.y, 0));
        model = glm::scale(model, glm::vec3(size.x, size.y, 0));
        return model;
    }


    unsigned int getDefaultVAO() {
        float viis[] = {
            // Vertex positions
            -0.5f, +0.5f,
            +0.5f, +0.5f,
            +0.5f, -0.5f,
            -0.5f, -0.5f,
        };
        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0,
        };

        unsigned int tVAO;

        glGenVertexArrays(1, &tVAO);
        glBindVertexArray(tVAO);

        unsigned int VBO3;
        glGenBuffers(1, &VBO3);

        glBindBuffer(GL_ARRAY_BUFFER, VBO3);
        glBufferData(GL_ARRAY_BUFFER, sizeof(viis), viis, GL_DYNAMIC_DRAW);

        unsigned int EBO;
        glGenBuffers(1, &EBO);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        return tVAO;
    }
};