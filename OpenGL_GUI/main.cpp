#include <RectTest.h>
#include <json11.hpp>
#include <ctime>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <chrono>
#include <thread>

unsigned int Rect::VAO;

Camera2D* cam;

std::map<int, const char*> create_months()
{
  std::map<int, const char*> m;
  m[0] = "January";
  m[1] = "February";
  m[2] = "March";
  m[3] = "April";
  m[4] = "May";
  m[5] = "June";
  m[6] = "July";
  m[7] = "August";
  m[8] = "September";
  m[9] = "October";
  m[10] = "November";
  m[11] = "December";

  return m;
}

std::map<int, const char*> create_weekdays()
{
  std::map<int, const char*> m;
  m[0] = "Monday";
  m[1] = "Tuesday";
  m[2] = "Wednesday";
  m[3] = "Thursday";
  m[4] = "Friday";
  m[5] = "Saturday";
  m[6] = "Sunday";

  return m;
}

double getRand01(){ return (double)rand() / RAND_MAX; }

int main(void)
{
    std::map<int, const char*> monthMap = create_months();
    std::map<int, const char*> weekMap = create_weekdays();

    time_t now = time(0);
    tm *ltm = localtime(&now);

    char day[3];
    snprintf(day, 3, "%d", ltm->tm_mday);
    std::string head;
    
    head.append(day);
    head.append(" ");
    head.append(monthMap[ltm->tm_mon]);
    head.append(" ");

    char year[10];
    snprintf(year, 10, "%d", ltm->tm_year + 1900);
    head.append(year);
    head.append(" ");
    int thisDay = (ltm->tm_wday + 6) % 7;
    head.append(weekMap[thisDay]);

    WindowHandler* wh = new WindowHandler(NULL);
    cam = new Camera2D(glm::vec2(8.0f, -4.5f), wh);
    wh->cam = cam;

    Rect::VAO = Rect::getDefaultVAO();
    std::vector<Rect*> rects;

    TextRenderer* tr = new TextRenderer("../assets/fonts/jbmB.ttf", cam);

    GuiWorld* gw = new GuiWorld(wh, tr);

    Rect* r = new Rect(glm::vec2(8, -1), glm::vec2(10, 0.04), gw);
    r->colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0);
    r->uRadius = 0.5f;
    r->powVal = 1.0f;
    r->uTHold = 0.5f;
    r->clickable = false;
    rects.push_back(r);

    r = new Rect(glm::vec2(15, -0.5), glm::vec2(1.5, 0.5), gw);
    r->colour = glm::vec4(0.9f, 0.9f, 0.9f, 1.0);
    r->outColour = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    r->texts.push_back("Add");
    r->powVal = 1.0f;
    r->textSize = 2.5f;
    rects.push_back(r);

    r = new Rect(glm::vec2(1, -0.5), glm::vec2(1.5f, 0.5), gw);
    r->colour = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    r->fontColour = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);
    r->powVal = 1.0f;
    r->texts.push_back("Revert");
    r->textSize = 2.5f;
    rects.push_back(r);

    float xSize = 2.0f;
    float ySize = 1.0f;
    float margin = cam->baseX - xSize * 7;
    margin /= 8.0f;

    const char* texts1[7] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    // manual for now
    const char* texts2[7] = {"31/1/2022", "01/2/2022", "02/2/2022", "03/2/2022", "04/2/2022", "05/2/2022", "06/2/2022"};

    for (int i = 0; i < 7; i++){
        r = new Rect(glm::vec2(i * (xSize + margin) + margin + xSize / 2.0f, -1.75f), glm::vec2(xSize, ySize), gw);
        r->colour = glm::vec4(0.3f, 0.3f, 0.8f, 1.0);
        r->outColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0);
        r->uTHold = 0.05f;
        r->powVal = 2.0f;
        r->uRadius = 0.15f;
        r->textSize = 2.75f * ySize;
        r->texts.push_back(texts2[i]);
        r->texts.push_back(texts1[i]);
        r->clickable = false;
        if (i == thisDay){
            r->colour = glm::vec4(0.3f, 0.8f, 0.8f, 1.0);
            r->outColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0);;
            r->uTHold = 0.1f;
        }
        r->refresh();
        rects.push_back(r);
    }

    std::ifstream file("../assets/jsons/data.json", std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer(size + 1);
    file.read(buffer.data(), size);
    buffer.at(size) = '\0';
    const std::string simple_test = buffer.data();

    std::string err;
    const auto json = json11::Json::parse(simple_test, err);

    if (err.size() > 0)
        printf("Error : %s\n", err.c_str());

    int yPositions[] = {0, 0, 0, 0, 0, 0, 0};

    for (json11::Json j : json.array_items()){
        int wDay = j["weekDay"].int_value();
        std::string lecName = j["name"].string_value();
        std::string times = j["time"].string_value();
        yPositions[wDay] += 1;
        Rect* r = new Rect(glm::vec2(wDay * (xSize + margin) + margin + xSize / 2.0f, -1.75f - yPositions[wDay] * (ySize * 1.1f)),
            glm::vec2(xSize, ySize), gw);
        r->colour = glm::vec4(0.9, 0.9, 0.9, 1.0);
        r->outColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0);
        r->uTHold = 0.05f;
        r->powVal = 2.0f;
        r->uRadius = 0.2f;
        r->textSize = 2.75f * ySize;
        r->texts.push_back(times);
        r->texts.push_back(lecName);
        r->refresh();
        rects.push_back(r);
    }

    cam->update();
    bool camEnable = false;
    bool done = false;
    while (!done){
        done = wh->looper();

        //cam->update();
        
        gw->loop();
        if (wh->shouldRender){
            glfwSwapInterval(1);
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            for (int i = 0; i < rects.size(); i++){
                rects.at(i)->update(cam);
            }

            tr->RenderText(head, 8, -0.5, 6.0f, glm::vec3(1, 1, 1));

            glfwSwapBuffers(wh->window);
            wh->shouldRender = false;
        }   
        else{
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    glfwTerminate();
    return 0;
}