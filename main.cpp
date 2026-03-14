#include <iostream>
#include <raylib.h>
#include <vector>
#include <bits/stdc++.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

bool debugEnabled = false;

Sound pop_sound;

enum WALK {UP, DOWN, LEFT, RIGHT};

const int SCREEN_WIDTH = 2500;
const int SCREEN_HEIGHT = 1300;

float FPS = 60;
float gameTick = 10; // 10 ms = 1 tick

std::default_random_engine engine;

void setGameTick(float t)
{
    gameTick = t;
}

int convertFromGameTick(int FPS = 60)
{
    int e = (gameTick / 100 * FPS);
    return e == 0 ? 1 : e;
}

int randomNumber(int min, int max)
{
    std::uniform_int_distribution<> myRand(min, max);
    return myRand(engine);
}

template <class T>
T randomNumber(T min, T max)
{
    std::uniform_real_distribution<> myRand(min, max);
    return myRand(engine);
}

template <class T>
T RandomNumberWithout(T min, T max, std::vector<T> exclude) // returns a random number between min and max excluding wanted numbers
{
    std::vector<T> final_rand; // final set of random numbers
    bool remove_val;

    for(T candidate = min; candidate <= max; candidate++)
    {
        remove_val = false;
        for(int j = 0; j < T(exclude.size()); j++)
        {
            if(candidate == exclude[j])
            {
                remove_val = true;
                exclude.erase(find(exclude.begin(), exclude.end(), candidate)); // remove found element
                break;
            }
        }
        if(!remove_val)
            final_rand.push_back(candidate);
    }
    T random_num = randomNumber<int>(0, final_rand.size() - 1); // get a random index to vector of numbers to randomly select from

    return final_rand[random_num];
}

class Creature
{
    protected:
    float view_distance, hp, atk;
    Rectangle pos;
    Color col;

    int counter = 0, interval_selector = 0;
    
    public:

    static float walkLength;
    int wayCount[4] = {0};

    static int creature_size;

    Creature() {creature_size++;}

    Creature(float hp, float atk, float v_d = 50, Rectangle pos = {0, 0, 10, 10}, Color col = RED) 
    : view_distance(v_d), hp(hp), atk(atk), pos(pos), col(col) 
    {   
        creature_size++; 
        if(!IsSoundPlaying(pop_sound))
            PlaySound(pop_sound);
    }

    virtual ~Creature(){creature_size--;}

    friend void addPoints(std::vector<Creature *> &creatures, struct preDefInfo &a, int count);

    void setWalkLength(int n)
    {
        walkLength = n;
    }

    void setRandomColor()
    {
        col = Color
        {
            (unsigned char)randomNumber(100, 255),
            (unsigned char)randomNumber(100, 255),
            (unsigned char)randomNumber(100, 255),
            (unsigned char)255 // alpha
        };
    }

    void Wander() // basic random walk
    {
        int walk_direction = -1;
        std::vector<int> possible_directions;

        if(pos.y - 1 > 0)
           possible_directions.push_back(UP);
        if(pos.y + 1 < SCREEN_HEIGHT)
           possible_directions.push_back(DOWN);
        if(pos.x - 1 > 0)
           possible_directions.push_back(LEFT);
        if(pos.x + 1 < SCREEN_WIDTH)
            possible_directions.push_back(RIGHT);
        
        float deltaTime = GetFrameTime();

        walk_direction = possible_directions[randomNumber(0, possible_directions.size() - 1)];

        switch(walk_direction)
        {
            case UP:
                pos.y -= walkLength * deltaTime;
                break;
            case DOWN:
                pos.y += walkLength * deltaTime;
                break;
            case LEFT:
                pos.x -= walkLength * deltaTime;
                break;
            case RIGHT:
                pos.x += walkLength * deltaTime;
                break;
        }
    }

    void DrawCreature()
    {
        Vector2 center = {pos.x - pos.width/2, pos.y - pos.height/2};
        DrawRectangleV(center, {pos.width, pos.height}, col);
    }

    friend void deleteCreature(std::vector<Creature *> &creatures, struct preDefInfo &a, int count);
};

class Human : public Creature
{
    
    public:

    Human() {}

    ~Human() {}

    Human(float hp, float atk, float v_d = 50, Rectangle pos = {0, 0, 10, 10}) 
    : Creature(hp, atk, v_d, pos) {}
};

int Creature::creature_size = 0;
float Creature::walkLength = 800;

struct preDefInfo
{
    float hp, atk, view_dist;
    Rectangle body;
} preDefInfo {10, 2, 50, {400, 300, 5, 5}};

void addPoints(std::vector<Creature *> &creatures, struct preDefInfo &a, int count = 1)
{
    std::uniform_real_distribution<> randomCoords(2, std::max(SCREEN_HEIGHT - 2, SCREEN_WIDTH - 2));
    for(int i = 0; i < count; i++)
    {
        creatures.push_back(new Human(a.hp, a.atk, a.view_dist, Rectangle{ static_cast<float>(randomCoords(engine)), static_cast<float>(randomCoords(engine)), a.body.width, a.body.height}));
        creatures.back()->setRandomColor();
    }
}

void deleteCreature(std::vector<Creature *> &creatures, struct preDefInfo &a, int count = 1)
{
    int delete_count = Creature::creature_size > count ? count : Creature::creature_size;
    for(int i = 0; i < delete_count; i++)
    {
        auto last_elem_p = (creatures.end() - 1);
        delete *last_elem_p;
        creatures.erase(last_elem_p);
    }
}

/*-----------------------------------------*/

int main () 
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Creature simulator");
    SetTargetFPS(FPS);
    engine.seed(time(0));
    float hp {10}, atk {2}, view_dist {50};
    Rectangle body {400, 300, 5, 5};

    InitAudioDevice(); // Initialize audio device and context
    
    if(!IsAudioDeviceReady())
    {
        std::cerr << "Sound couldn't be initialized.\n";
        return 1;
    }

    pop_sound = LoadSound("pop_sound.mp3");

    std::vector<Creature*> creatures;

    addPoints(creatures, preDefInfo, 10);

    int i = 0;
    bool showUI = true;
    float old_FPS;

    while (WindowShouldClose() == false)
    {   
        if(GuiButton({700, 300, 90, 30}, "add 1 point"))
            addPoints(creatures, preDefInfo);
        if(GuiButton({700, 330, 90, 30}, "add 10 points"))
            addPoints(creatures, preDefInfo, 10);
        if(GuiButton({700, 360, 90, 30}, "add 100 points"))
            addPoints(creatures, preDefInfo, 100);
        if(GuiButton({700, 390, 90, 30}, "add 1000 points"))
            addPoints(creatures, preDefInfo, 1000);
        if(GuiButton({700, 420, 90, 30}, "add 10000 points"))
            addPoints(creatures, preDefInfo, 10000);
        if(GuiButton({790, 300, 90, 30}, "delete 1 point"))
            deleteCreature(creatures, preDefInfo);
        if(GuiButton({790, 330, 90, 30}, "delete 10 points"))
            deleteCreature(creatures, preDefInfo, 10);
        if(GuiButton({790, 360, 90, 30}, "delete 100 points"))
            deleteCreature(creatures, preDefInfo, 100);
        if(GuiButton({790, 390, 90, 30}, "delete 1000 points"))
            deleteCreature(creatures, preDefInfo, 1000);
        
        old_FPS = FPS;
        GuiSliderBar({1200, 20, 200, 20}, "30", "300", &FPS, 30, 300);
        if(int(old_FPS) != int(FPS))
            SetTargetFPS(FPS);

        BeginDrawing();
            ClearBackground(BLACK);
            GuiSliderBar({50, 500, 200, 30}, "1", "100", &gameTick, 1.0f, 100.0f);
            GuiSliderBar({50, 400, 200, 30}, "500", "10000", &Creature::walkLength, 500.0f, 10000.0f);
            DrawText(TextFormat("%f", gameTick), 50, 450, 25, GREEN);
            DrawText(TextFormat("getfps: %d, fps: %f", GetFPS(), FPS), 1200, 40, 20, YELLOW);
            if(debugEnabled)
                DrawText(TextFormat("UP: %4d, DOWN: %4d, LEFT: %4d, RIGHT: %4d",
                    creatures[0]->wayCount[0],
                    creatures[0]->wayCount[1],
                    creatures[0]->wayCount[2], 
                    creatures[0]->wayCount[3]), 10, 370, 25, GREEN);
            if(debugEnabled)
                DrawText(TextFormat("Creature::creature_size: %d", Creature::creature_size), 300, 300, 20, RED);
            if(i++ % convertFromGameTick(FPS) == 0)
            {
                for(int j = 0; j < Creature::creature_size; j++)
                {
                    creatures[j]->Wander();
                }
            }
            for(int j = 0; j < Creature::creature_size; j++)
            {
                creatures[j]->DrawCreature();
            }
        EndDrawing();
    }

    for (int j = 0; j < Creature::creature_size; j++)
    {
        delete creatures[j];
    }
    creatures.clear();
    CloseAudioDevice(); // Close the audio device and context
    CloseWindow();
}