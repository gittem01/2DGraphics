#include <RectTest.h>

GuiWorld::GuiWorld(WindowHandler* wh, TextRenderer* tr){
    this->wh = wh;
    this->tr = tr;
    this->bodyCount = 0;

    // head body
    this->bodyHead = (GBody*)malloc(sizeof(GBody));
    this->bodyHead->next = nullptr;
    // tail Body
    this->bodyTail = bodyHead;

    this->hoveringBody = nullptr;
    this->clickedBody = nullptr;
    this->lastHover = nullptr;

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

void GuiWorld::addBody(GBody* body){
    body->next = nullptr;
    bodyTail->next = body;
    bodyTail = body;
    bodyCount++;
}

void GuiWorld::loop(){
    glm::vec2 mousePosition = wh->cam->getMouseCoords();
    bool onRect = false;
    bool isHovering = false;
    if (hoveringBody != nullptr){
        bool stillHovering = checkBodyCollision(hoveringBody, mousePosition);
        if (stillHovering){
            isHovering = true;
        }
        else{
            Rect* r = (Rect*)((bodyData*)(hoveringBody)->data)->data;
            if (r->clickable){
                r->uTHold = oldVals[0];
                r->uRadius = oldVals[1];
            }
            hoveringBody = nullptr;
        }
    }

    if (!isHovering){
        for (GBody* body = bodyHead->next; body; body = body->next){
            if (checkBodyCollision(body, mousePosition)){
                Rect* r = (Rect*)((bodyData*)(body)->data)->data;
                hoveringBody = body;
                break;
            }
        }
    }

    if (hoveringBody && lastHover != hoveringBody && (!clickedBody || clickedBody == hoveringBody)){
        lastHover = hoveringBody;
        // hover started
        Rect* r = (Rect*)((bodyData*)(hoveringBody)->data)->data;
        if (r->clickable){
            oldVals[0] = r->uTHold;
            oldVals[1] = r->uRadius;
            r->uTHold = 0.1f;
            r->uRadius = 0.25f;
            glfwSetCursor(wh->window, handCursor);
        }
    }
    if (wh->mouseData[2] == 2){
        clickedBody = hoveringBody;
    }
    else if (!hoveringBody && lastHover){
        // hover ended
        lastHover = nullptr;
        glfwSetCursor(wh->window, arrowCursor);
    }
    if (hoveringBody == clickedBody && clickedBody){
        Rect* r = (Rect*)((bodyData*)(clickedBody)->data)->data;
        if (wh->mouseData[2] == 2){
            oldVals[2] = r->powVal;
            r->powVal = 4.0f;
            // click
        }
    }
    else if (clickedBody){
        Rect* r = (Rect*)((bodyData*)(clickedBody)->data)->data;
        r->powVal = oldVals[2];
    }
    if (wh->mouseData[2] == 0 && clickedBody){
        Rect* r = (Rect*)((bodyData*)(clickedBody)->data)->data;
        bool isIn = checkBodyCollision(clickedBody, mousePosition);
        if (isIn){
            // click and release on same body
            r->powVal = oldVals[2];
        }
        clickedBody = nullptr;
    }
}