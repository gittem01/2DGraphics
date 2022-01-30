#include <math.h>
#include "Shader.h"
#include "Camera2D.h"
#include <vector>
#include <TextRenderer.h>

class Rect
{

public:
    static unsigned int VAO;

	Shader* shader;
    TextRenderer* tr;

    glm::vec2 pos;
	float rotation;
	glm::vec2 size;
	glm::vec4 colour;
    glm::vec4 outColour;
    glm::vec4 fontColour;

    float powVal = 3.0f;
    float uRadius = 0.2f;
    float uTHold = 0.1f;
    int innerGlow = 0;

    std::vector<std::string> texts;
    float textSize = 3.0f;
    float extraYMargin = 0.0f;

    Rect(glm::vec2 pos, glm::vec2 size, TextRenderer* tr)
    {
        this->pos = glm::vec2(pos.x, pos.y);
        this->rotation = 0.0f;
        this->size = glm::vec2(size.x, size.y);
        this->colour = glm::vec4(0.2f, 0.2f, 0.2f, 0.0);
        this->outColour = glm::vec4(1, 1, 1, 1.0);
        this->fontColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    
        this->shader = new Shader("../assets/shaders/Rectangle/");
        this->tr = tr;
    }

    void update(Camera2D* cam)
    {
        shader->use();
        glm::mat4 ortho = cam->ortho;
        glm::mat4 model = this->getModel();

        shader->setMat4("ortho", ortho);
        shader->setMat4("model", model);
        shader->setVec3("bgColour", 0.2f, 0.2f, 0.2f);
        shader->setVec2("scale", size.x, size.y);
        shader->setFloat("powVal", powVal);
        shader->setFloat("uRadius", uRadius);
        shader->setFloat("uThold", uTHold);
        shader->setInt("innerGlow", innerGlow);

        shader->setVec4("inColour", colour.x, colour.y, colour.z, colour.w);
        shader->setVec4("outColour", outColour.x, outColour.y, outColour.z, outColour.w);

        glBindVertexArray(Rect::VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        if (texts.size() == 1)
        {
            tr->RenderText(texts[0], pos.x, pos.y + extraYMargin, textSize, fontColour);
        }
        else if (texts.size() == 2){
            for (int i = 1; i < 3; i++){
                tr->RenderText(texts[i - 1], pos.x, pos.y + (i * 2 - 3) * size.y * 0.2f + extraYMargin, textSize, fontColour);
            }
        }
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
        // Vertex positions
        float viis[] = {
            -0.5f, +0.5f, 0.0f, 0.0f, // top left
            +0.5f, +0.5f, 1.0f, 0.0f, // top right
            -0.5f, -0.5f, 0.0f, 1.0f, // bottom left
            +0.5f, -0.5f, 1.0f, 1.0f, // bottom right
        };

        unsigned int tVAO;

        glGenVertexArrays(1, &tVAO);
        glBindVertexArray(tVAO);

        unsigned int VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(viis), viis, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        return tVAO;
    }
};