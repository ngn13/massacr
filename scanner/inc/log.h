#pragma once
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../inc/op.h"

#define RED "\x1b[31m"
#define BOLD "\x1b[1m"
#define BLUE "\x1b[34m"
#define GRAY "\x1b[37m"
#define RESET "\x1b[0m"
#define GREEN "\x1b[32m"
#define SPACE "                      "

void info(const char *, ...);
void error(const char *, ...);
void debug(const char *, ...);
