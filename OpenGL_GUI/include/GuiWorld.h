#include <glm/glm.hpp>
#include <Camera2D.h>

class TextRenderer;
class Rect;

enum bodyType{
    RECTANGLE = 1,
    CIRCLE = 2
};

enum objectType{
    RECT,
    OTHER, // use later
};

typedef struct bodyData{
    objectType type;
    void* data;
} bodyData;

typedef struct GBody{
    bodyType shape;
    glm::vec2 pos;
    glm::vec2 size; // use x only for circle
    bodyData* data;
} GBody;

class GuiWorld
{

friend class Rect;

private:
    
    GBody** bodies;
    WindowHandler* wh;
    TextRenderer* tr;
    int bodyCount;

    GLFWcursor* arrowCursor;
    GLFWcursor* handCursor;

public:
    
    GuiWorld(WindowHandler* wh, TextRenderer* tr);

    void loop();
};