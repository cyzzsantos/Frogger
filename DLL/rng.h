#pragma once
#include <random>
#include <iostream>
#include "Constants.h"

typedef struct
{
	int num1;
	int num2;
}
Results;

Results rng(int min, int max);
int rngSetup(int min, int max);