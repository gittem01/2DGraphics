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
    GBody* next;
    bodyData* data;
} GBody;

class GuiWorld
{

friend class Rect;

private:
    
    GBody* bodyHead;
    GBody* bodyTail;

    GBody* hoveringBody;
    GBody* clickedBody;
    GBody* lastHover;

    float oldVals[3];

    WindowHandler* wh;
    TextRenderer* tr;
    int bodyCount;
    void addBody(GBody* body);

    GLFWcursor* arrowCursor;
    GLFWcursor* handCursor;

public:
    
    GuiWorld(WindowHandler* wh, TextRenderer* tr);

    void loop();
};