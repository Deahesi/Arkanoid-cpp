#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <TCHAR.h>
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

struct LEVEL {
    string text;
    bool hover;
};

struct MENUBTN {
    short x;
    short y;
    string text;
    bool hover;
} btns[3] = { {WIDTH / 2 - 4, HEIGHT / 2 - 2, "PLAY", true},{WIDTH / 2 - 4, HEIGHT / 2, "LEVEL", false},{WIDTH / 2 - 4, HEIGHT / 2 + 2, "EXIT", false}, };
short choosedbtn = 0;

PLAYER player;
vector<BALL> balls (0);
vector<MODIFICATOR> modificators;
vector<LEVEL> levels;
short choosedbtnlevel = 0;

short level;
bool menu = false;
bool changelevel = false;
bool exitstate = false;
bool choseimportantlevel = false;
HANDLE wHnd;
HANDLE rHnd;

string map[HEIGHT];
bool beated[HEIGHT][WIDTH];

fstream stream;
ifstream is;

WIN32_FIND_DATA FindFileData;
HANDLE hFind;

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

vector<LEVEL> getfiles() {
    int count = 0;
    vector<LEVEL> files;
    while (++count < 11) {
        is.open("maps/" + to_string(count) + ".txt");
        if (is.is_open()) {
            files.push_back({ to_string(count) + ".txt", false });
        }
        else {
            is.close();
            files[0].hover = true;
            return files;
        }
        is.close();
    }
    files[0].hover = true;
    return files;
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
    if (!choseimportantlevel) {
        stream.open("save.txt", fstream::in | fstream::out | fstream::app);
        if (!stream.is_open()) {
            stream << 1;
        }
        char lvl_str[3];
        stream >> lvl_str;

        level = atoi(lvl_str);
        stream.close();
    }
    choseimportantlevel = false;

    get_level(map, level);


    player.vx = 0;
    player.width = PLAYER_WIDTH;
    player.x = (WIDTH / 2) - (PLAYER_WIDTH / 2);

    fill(&beated[0][0], &beated[0][0] + sizeof(beated), false);

    if (balls.size() > 0)
        balls.clear();

    if (modificators.size() > 0)
        modificators.clear();

    balls.push_back({});
    balls[0].vx = BALL_SPEED;
    balls[0].vy = BALL_SPEED;
    balls[0].x = WIDTH / 2;
    balls[0].y = (HEIGHT / 2) - 10;

    return true;
}

void exit() {
    stream.open("save.txt", fstream::in);
    stream << level;
    stream.close();
}

void rendermenu(CHAR_INFO(*consolebuffer)[WIDTH * HEIGHT], SMALL_RECT* windowSize) {
    refreshframe(consolebuffer);

    for (short i = 0; i < sizeof(btns) / sizeof(MENUBTN); i++) {
        for (size_t ch = 0; ch < btns[i].text.size(); ch++) {
            (*consolebuffer)[btns[i].x + ch + btns[i].y * HEIGHT].Char.AsciiChar = btns[i].text[ch];
            (*consolebuffer)[btns[i].x + ch + btns[i].y * HEIGHT].Attributes = btns[i].hover ? FOREGROUND_BLUE | BACKGROUND_GREEN : FOREGROUND_GREEN | BACKGROUND_BLUE;
        }
    }

    WriteConsoleOutputA(wHnd, *consolebuffer, COORD{ WIDTH, HEIGHT }, COORD{ 0, 0 }, windowSize);
}

void renderlevel(CHAR_INFO(*consolebuffer)[WIDTH * HEIGHT], SMALL_RECT* windowSize, vector<LEVEL> files) {
    refreshframe(consolebuffer);
    for (short i = 0; i < files.size(); i++) {
        for (short j = 0; j < files[i].text.size(); j++) {
            (*consolebuffer)[short(WIDTH / 2 - 5 + j) + short(i * 2 + 5) * HEIGHT].Char.AsciiChar = files[i].text[j];
            (*consolebuffer)[short(WIDTH / 2 - 5 + j) + short(i * 2 + 5) * HEIGHT].Attributes = files[i].hover ? FOREGROUND_BLUE | BACKGROUND_GREEN : FOREGROUND_GREEN | BACKGROUND_BLUE;
        }

    }

    WriteConsoleOutputA(wHnd, *consolebuffer, COORD{ WIDTH, HEIGHT }, COORD{ 0, 0 }, windowSize);
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
                    else if (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE) {
                        menu = !menu;
                    }
                    else if (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_UP && menu) {
                        btns[choosedbtn].hover = false;
                        choosedbtn = choosedbtn > 0 ? choosedbtn - 1 : sizeof(btns) / sizeof(MENUBTN) - 1;
                        btns[choosedbtn].hover = true;
                    }
                    else if (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_DOWN && menu) {
                        btns[choosedbtn].hover = false;
                        choosedbtn = choosedbtn < sizeof(btns) / sizeof(MENUBTN) - 1 ? choosedbtn + 1 : 0;
                        btns[choosedbtn].hover = true;
                    }
                    else if (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_UP && changelevel) {
                        levels[choosedbtnlevel].hover = false;
                        choosedbtnlevel = choosedbtnlevel > 0 ? choosedbtnlevel - 1 : levels.size() - 1;
                        levels[choosedbtnlevel].hover = true;
                    }
                    else if (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_DOWN && changelevel) {
                        levels[choosedbtnlevel].hover = false;
                        choosedbtnlevel = choosedbtnlevel < levels.size() - 1 ? choosedbtnlevel + 1 : 0;
                        levels[choosedbtnlevel].hover = true;
                    }
                    else if (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_RETURN && changelevel) {
                        level = choosedbtnlevel + 1;
                        menu = false;
                        changelevel = false;
                        choseimportantlevel = true;
                    }
                    else if (eventBuffer[i].Event.KeyEvent.wVirtualKeyCode == VK_RETURN && menu) {
                        switch (choosedbtn) {
                        case 0:
                            menu = false;
                            break;
                        case 1:
                            menu = false;
                            changelevel = true;
                            break;
                        case 2:
                            exitstate = true;
                            break;
                        }
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
        else if ((balls[i].y + 1) >= player.y && (((balls[i].x + balls[i].vx) >= player.x) && (balls[i].x + balls[i].vx) <= (player.x + player.width))) {
            balls[i].vy = -balls[i].vy;
        }
        else if (balls[i].y > player.y) {
            if (balls.size() > 1) {
                balls.erase(balls.begin() + i);
            }
            else {
                menu = true;
                return;
            }
        }
        else if ((*consolebuffer)[short(balls[i].x + 1) + (short(balls[i].y + 1)) * HEIGHT].Char.AsciiChar == '#') {
            beated[(short(balls[i].y))][short(balls[i].x)] = true;
            balls[i].vy = -balls[i].vy;
            if (modificators.size() < 50 && (rand() % 3 + 1) == 2) {
                modificators.push_back({ balls[i].x, balls[i].y, 0.004, rand() % 3 + 1 });
            }
        }

        balls[i].x += balls[i].vx;
        balls[i].y += balls[i].vy;
    }

    for (short i = 0; i < modificators.size(); i++) {
        if ((modificators[i].y + modificators[i].vy) >= player.y && ((modificators[i].x >= player.x) && (modificators[i].x <= (player.x + player.width)))) {
            short size = balls.size();
            if (size + (size * modificators[i].mod) <= 99) {
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

    while (!exitstate) {
        bool set = setup();
        while (!menu) {
            render(&consolebuffer, &windowSize);
            logic(&consolebuffer);
            input();
        }
        while (menu && !exitstate) {
            rendermenu(&consolebuffer, &windowSize);
            input();
        }
        if (changelevel) {
            levels = getfiles();
            while (changelevel && !exitstate) {
                renderlevel(&consolebuffer, &windowSize, levels);
                input();
            }
        }
    }


}