// proto.h
// author:  Maciej Szpakowski
// includes, defines, libraries linking

#pragma once

#define STRICT

// SETTINGS
#define POLL 1
#define TICKMS 250
#define WIDTH 40
#define HEIGHT 30
#define CELL 32
#define RABBIT_COUNT 20
#define WOLF_COUNT 5
#define WEASEL_COUNT 7
#define DINO_COUNT 1
#define DIRE ""

typedef struct S
{
    unsigned int grass : 7;
    unsigned int type : 3;
    unsigned int food : 10;
    unsigned int age : 8;
    unsigned int id : 16;
    unsigned int wheat : 7;

    S() :grass(0), type(0), food(0), age(0), id(0), wheat(0) {}
} FBITS;

#define FRABBIT DIRE "rabbit.png"
#define FWOLF DIRE "wolf.png"
#define FDINO DIRE "dino.png"
#define FWEASEL DIRE "weasel.png"
#define FWHEAT DIRE "wheat.png"

#define TWOLF 1
#define TDINO 2
#define TWEASEL 3
#define TRABBIT 4

#include <queue>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>

using std::chrono::milliseconds;

#include "exceptions.h"
#include "matrix.h"
#include "resources.h"
#include "entity.h"
#include "world.h"

#ifdef _WIN32
#ifdef _DEBUG

#pragma comment(lib,  "sfml-graphics-d.lib")
#pragma comment(lib, "sfml-window-d.lib")
#pragma comment(lib, "sfml-system-d.lib")

#else

#pragma comment(lib, "sfml-graphics.lib")
#pragma comment(lib,  "sfml-window.lib")
#pragma comment(lib, "sfml-system.lib")

#endif
#endif