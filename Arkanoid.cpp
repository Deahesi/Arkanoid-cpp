#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <Tchar.h>
#include <cmath>

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


PLAYER player;
BALL ball;

short level;
HANDLE wHnd;
HANDLE rHnd;

string map[HEIGHT];
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
    
    //cout << ::map[0][50];
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
    //cout << sizeof(map) / sizeof(char*);
    //stream.open("maps/" + to_string(level) + ".txt", fstream::in | fstream::app);

    //if (!stream.is_open()) {
    //    return false;
    //}

    //stream.seekg(0, stream.end);
    //int length = stream.tellg();
    //stream.seekg(0, stream.beg);

    //stream.read(map, length);
    //stream.close();

    player.vx = 0;
    player.width = PLAYER_WIDTH;
    player.x = (WIDTH / 2) - (PLAYER_WIDTH / 2);

    ball.vx = BALL_SPEED;
    ball.vy = BALL_SPEED;
    ball.x = (WIDTH / 2);
    ball.y = (HEIGHT / 2) + 10;

    return true;
}

void exit() {
    stream.open("save.txt", fstream::in);
    stream << level;
}

void render(CHAR_INFO(*consolebuffer)[WIDTH * HEIGHT], SMALL_RECT* windowSize) {
    refreshframe(consolebuffer);

    //ШАРИК НА ЭКРАН
    (*consolebuffer)[short(ball.x) + short(ball.y) * HEIGHT].Char.AsciiChar = 64;
    (*consolebuffer)[short(ball.x) + short(ball.y) * HEIGHT].Attributes = BACKGROUND_GREEN;


    for (short i = 0; i < player.width; i++) {
        (*consolebuffer)[short(player.x) + i + short(player.y) * HEIGHT].Char.AsciiChar = 254;
        (*consolebuffer)[short(player.x) + i + short(player.y) * HEIGHT].Attributes = FOREGROUND_RED | BACKGROUND_BLUE;
    }

    for (short y = 0; y < (sizeof(map) / sizeof(string)) / 10; y++) {
        for (short x = 0; x < map[y].size(); x++) {
            (*consolebuffer)[x + 1 + (y + 1) * HEIGHT].Char.AsciiChar = map[y][x];
            (*consolebuffer)[x + 1 + (y + 1) * HEIGHT].Attributes = FOREGROUND_RED | BACKGROUND_BLUE;
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

void logic() {
    if ((ball.x + ball.vx) >= (WIDTH - 1) || (ball.x + ball.vx) <= 0) {
        ball.vx = -ball.vx;
    }
    if ((ball.y + ball.vy) >= (HEIGHT - 1) || (ball.y + ball.vy) <= 0) {
        ball.vy = -ball.vy;
    }
    if ((ball.y + ball.vy) >= player.y && (((ball.x + ball.vx) >= player.x) && (ball.x + ball.vx) <= (player.x + player.width))) {
        ball.vy = -ball.vy;
    }

    ball.x += ball.vx;
    ball.y += ball.vy;
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
            logic();
            input();
        }
    }

}