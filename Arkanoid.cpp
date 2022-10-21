#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <Tchar.h>
#include <cmath>
#include <vector>

#define WIDTH 50
#define HEIGHT 50
#define ACCELERATION 1
#define PLAYER_WIDTH 10
#define BALL_SPEED 0.005
#define SPEED 1

using namespace std;

struct PLAYER {
    float x;
    const static short y = (HEIGHT - 4);
    float vx;
    short width;
};

struct BALL {
    float x;
    float y;
    float vx;
    float vy;
};

struct MODIFICATOR {
    float x;
    float y;
    float vy;
    int mod;
};


PLAYER player;
vector<BALL> balls (2);
vector<MODIFICATOR> modificators;

short level;
HANDLE wHnd;
HANDLE rHnd;

string map[HEIGHT];
bool beated[HEIGHT][WIDTH];

fstream stream;
ifstream is;

void get_level(string map[HEIGHT], unsigned short level) {
    is.open("maps/" + to_string(level) + ".txt");
    if (is.is_open()) {
        short y = 0;
        while (!is.eof()) {
            getline(is, ::map[y]);
            y++;
        }
        is.close();
    }
    else {
        is.close();
    }
}

void fillboard(CHAR_INFO(*consolebuffer)[WIDTH * HEIGHT]) {
    for (int x = 0; x < WIDTH; ++x) {
        (*consolebuffer)[x + HEIGHT * 0].Char.AsciiChar = 219;
        (*consolebuffer)[x + HEIGHT * (HEIGHT - 1)].Char.AsciiChar = 219;
    }
    for (int y = 0; y < HEIGHT; ++y) {
        (*consolebuffer)[0 + HEIGHT * y].Char.AsciiChar = 219;
        (*consolebuffer)[WIDTH - 1 + HEIGHT * y].Char.AsciiChar = 219;
    }
}

void refreshframe(CHAR_INFO(*consolebuffer)[WIDTH * HEIGHT]) {
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            (*consolebuffer)[x + HEIGHT * y].Char.AsciiChar = ' ';
            (*consolebuffer)[x + HEIGHT * y].Attributes = BACKGROUND_BLUE;
        }
    }
    fillboard(consolebuffer);
}

bool setup() {
    stream.open("save.txt", fstream::in | fstream::out | fstream::app);
    if (!stream.is_open()) {
        stream << 1;
    }
    char lvl_str[3];
    stream >> lvl_str;
    
    level = atoi(lvl_str);
    stream.close();


    get_level(map, level);


    player.vx = 0;
    player.width = PLAYER_WIDTH;
    player.x = (WIDTH / 2) - (PLAYER_WIDTH / 2);

    balls[0].vx = BALL_SPEED;
    balls[0].vy = BALL_SPEED;
    balls[0].x = (WIDTH / 2);
    balls[0].y = (HEIGHT / 2) + 10;

    balls[1].vx = BALL_SPEED;
    balls[1].vy = BALL_SPEED;
    balls[1].x = (WIDTH / 2) - 10;
    balls[1].y = (HEIGHT / 2) + 10;

    return true;
}

void exit() {
    stream.open("save.txt", fstream::in);
    stream << level;
    stream.close();
}

void render(CHAR_INFO(*consolebuffer)[WIDTH * HEIGHT], SMALL_RECT* windowSize) {
    refreshframe(consolebuffer);

    //ШАРИК НА ЭКРАН
    for (short i = 0; i < balls.size(); i++) {
        (*consolebuffer)[short(balls[i].x) + short(balls[i].y) * HEIGHT].Char.AsciiChar = 64;
        (*consolebuffer)[short(balls[i].x) + short(balls[i].y) * HEIGHT].Attributes = BACKGROUND_GREEN;
    }


    //МОДИФИКАТОРЫ
    for (short i = 0; i < modificators.size(); i++) {
        (*consolebuffer)[short(modificators[i].x) + short(modificators[i].y) * HEIGHT].Char.AsciiChar = '+';
        (*consolebuffer)[short(modificators[i].x) + short(modificators[i].y) * HEIGHT].Attributes = BACKGROUND_RED;
    }



    for (short i = 0; i < player.width; i++) {
        (*consolebuffer)[short(player.x) + i + short(player.y) * HEIGHT].Char.AsciiChar = 254;
        (*consolebuffer)[short(player.x) + i + short(player.y) * HEIGHT].Attributes = FOREGROUND_RED | BACKGROUND_BLUE;
    }

    for (short y = 0; y < (sizeof(map) / sizeof(string)) / 10; y++) {
        for (short x = 0; x < map[y].size(); x++) {
            if (!beated[y][x]) {
                (*consolebuffer)[x + 1 + (y + 1) * HEIGHT].Char.AsciiChar = map[y][x];
                (*consolebuffer)[x + 1 + (y + 1) * HEIGHT].Attributes = FOREGROUND_RED | BACKGROUND_BLUE;
            }
        }
    }


    WriteConsoleOutputA(wHnd, *consolebuffer, COORD{ WIDTH, HEIGHT }, COORD{ 0, 0 }, windowSize);
}

void input() {
    DWORD numEvents = 0;
    DWORD numEventsRead = 0;
    GetNumberOfConsoleInputEvents(rHnd, &numEvents);


    if (numEvents != 0) {
        INPUT_RECORD* eventBuffer = new INPUT_RECORD[numEvents];
        ReadConsoleInput(rHnd, eventBuffer, numEvents, &numEventsRead);


        for (DWORD i = 0; i < numEventsRead; ++i) {
            if (eventBuffer[i].EventType == KEY_EVENT) {
                if (eventBuffer[i].Event.KeyEvent.bKeyDown) {

                    if (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_LEFT && player.x > 1) {
                        player.vx = -SPEED * ACCELERATION;
                        player.x += player.vx;
                    }
                    else if (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_RIGHT && player.x + player.width < WIDTH - 1) {
                        player.vx = SPEED * ACCELERATION;
                        player.x += player.vx;
                    }
                }
            }
        }

        delete[] eventBuffer;
    }
}

void logic(CHAR_INFO(*consolebuffer)[WIDTH * HEIGHT]) {
    for (short i = 0; i < balls.size(); i++) {
        if ((balls[i].x + balls[i].vx) >= (WIDTH - 1) || (balls[i].x + balls[i].vx) <= 0) {
            balls[i].vx = -balls[i].vx;
        }
        else if ((balls[i].y + balls[i].vy) >= (HEIGHT - 1) || (balls[i].y + balls[i].vy) <= 0) {
            balls[i].vy = -balls[i].vy;
        }
        else if ((balls[i].y + balls[i].vy) >= player.y && (((balls[i].x + balls[i].vx) >= player.x) && (balls[i].x + balls[i].vx) <= (player.x + player.width))) {
            balls[i].vy = -balls[i].vy;
        }
        else if ((*consolebuffer)[short(balls[i].x + 1) + (short(balls[i].y + 1)) * HEIGHT].Char.AsciiChar == '#') {
            beated[(short(balls[i].y))][short(balls[i].x)] = true;
            balls[i].vy = -balls[i].vy;
            if (modificators.size() < 50) {
                modificators.push_back({ balls[i].x, balls[i].y, 0.02, rand() % 3 + 1 });
            }
        }

        balls[i].x += balls[i].vx;
        balls[i].y += balls[i].vy;
    }

    for (short i = 0; i < modificators.size(); i++) {
        if ((modificators[i].y + modificators[i].vy) >= player.y && ((modificators[i].x >= player.x) && (modificators[i].x <= (player.x + player.width)))) {
            short size = balls.size();
            if (size + size * modificators[i].mod <= 99) {
                for (short j = size; j < size * modificators[i].mod; j++) {
                    balls.push_back(BALL { float(rand() % (WIDTH - 1) + 1), float(rand() % (HEIGHT - 10) + 10), BALL_SPEED, BALL_SPEED });
                }
            }
        }

        modificators[i].y += modificators[i].vy;
        if (modificators[i].y > player.y) {
            modificators.erase(modificators.begin() + i);
        }
    }
}


int _tmain(int argc, _TCHAR* argv[]) {
    wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
    rHnd = GetStdHandle(STD_INPUT_HANDLE);
    SMALL_RECT windowSize = { 0, 0, WIDTH - 1, HEIGHT - 1 };
    COORD bufferSize = { WIDTH, HEIGHT };

    SetConsoleTitle(TEXT("Console Arkanoid!"));
    SetConsoleWindowInfo(wHnd, TRUE, &windowSize);
    SetConsoleScreenBufferSize(wHnd, bufferSize);

    //Заполняем фон
    CHAR_INFO consolebuffer[WIDTH * HEIGHT];
    refreshframe(&consolebuffer);


 
    COORD startPos = { 0,0 };

    WriteConsoleOutputA(wHnd, consolebuffer, bufferSize, startPos, &windowSize);

    bool set = setup();
    if (set) {
        while (true) {
            render(&consolebuffer, &windowSize);
            logic(&consolebuffer);
            input();
        }
    }

}