#include <RectTest.h>

#define MAX_BODY_COUNT 10000

GuiWorld::GuiWorld(WindowHandler* wh, TextRenderer* tr){
    this->wh = wh;
    this->tr = tr;
    this->bodyCount = 0;
    this->bodies = (GBody**)malloc(sizeof(GBody*) * MAX_BODY_COUNT);

    arrowCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    handCursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    glfwSetCursor(wh->window, arrowCursor);
}

inline bool checkCircleCollision(GBody* b, glm::vec2 mp){
    float dx = mp.x - b->pos.x;
    float dy = mp.y - b->pos.y;
    float r = b->size.x;
    return (dx * dx + dy * dy) < (r * r);
}

inline bool checkRectangleCollision(GBody* b, glm::vec2 mp){
    float xLeft = b->pos.x - b->size.x / 2;
    float xRight = b->pos.x + b->size.x / 2;
    float yTop = b->pos.y + b->size.y / 2;
    float yBottom = b->pos.y - b->size.y / 2;

    return (mp.x > xLeft && mp.x < xRight && mp.y > yBottom && mp.y < yTop);
}

inline bool checkBodyCollision(GBody* b, glm::vec2 p){
    if (b->shape == RECTANGLE){
        return checkRectangleCollision(b, p);
    }
    else if (b->shape == CIRCLE){
        return checkCircleCollision(b, p);
    }
    else{
        return false;
    }
}

void GuiWorld::loop(){
    glm::vec2 mousePosition = wh->cam->getMouseCoords();
    bool onRect = false;
    for (int i = 0; i < bodyCount; i++){
        if (checkBodyCollision(*(bodies + i), mousePosition)){
            Rect* r = (Rect*)((bodyData*)(*(bodies + i))->data)->data;
            r->uTHold = 0.1f;
            onRect = true;
        }
        else{
            Rect* r = (Rect*)((bodyData*)(*(bodies + i))->data)->data;
            r->uTHold = 0.05f;
        }
    }
    if (onRect){
        glfwSetCursor(wh->window, handCursor);
    }
    else{
        glfwSetCursor(wh->window, arrowCursor);
    }
}