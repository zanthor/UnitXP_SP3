#pragma once

#include <string>
#include <sstream>
#include "Vanilla1121_functions.h"

using namespace std;

// Calculate relative direction from player to target
// Returns: -1 for behind player, 0 for front, -2 for left, 2 for right
// Returns -999 for error
int UnitXP_relativeDirection(const void* player, const void* target);
int UnitXP_relativeDirection(const string unit0, const string unit1);
