#include <glm/glm.hpp>
#include <Shader.h>
#include <Camera2D.h>
#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H

struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};

class TextRenderer{
private:
    unsigned int VAO, VBO;
    std::map<int, Character> Characters;
    Shader* shader;
    Camera2D* cam;
    float quality;

    void init(std::string font_name);

public:
    TextRenderer(std::string fontFile, Camera2D* cam, float quality = 10.0f);
    ~TextRenderer();

    void RenderText(std::string text, float x, float y, float scale, glm::vec3 color);
};