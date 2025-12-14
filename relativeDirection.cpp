#include "pch.h"
#include "relativeDirection.h"
#include "Vanilla1121_functions.h"

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

// Calculate relative direction from player to target
// Returns: -1 for behind player, 0 for front, -2 for left, 2 for right
// Returns -999 for error
int UnitXP_relativeDirection(const void* playerVoid, const void* targetVoid) {
    if (!playerVoid || !targetVoid) {
        return -999;
    }

    uint32_t playerUnit = reinterpret_cast<uint32_t>(playerVoid);
    uint32_t targetUnit = reinterpret_cast<uint32_t>(targetVoid);

    if ((playerUnit & 1) != 0 || (targetUnit & 1) != 0) {
        return -999;
    }

    if (playerUnit == targetUnit) {
        return -999;
    }

    if (vanilla1121_objectType(playerUnit) != OBJECT_TYPE_Unit &&
        vanilla1121_objectType(playerUnit) != OBJECT_TYPE_Player) {
        return -999;
    }
    if (vanilla1121_objectType(targetUnit) != OBJECT_TYPE_Unit &&
        vanilla1121_objectType(targetUnit) != OBJECT_TYPE_Player) {
        return -999;
    }

    // Get positions
    C3Vector playerPos = vanilla1121_unitPosition(playerUnit);
    C3Vector targetPos = vanilla1121_unitPosition(targetUnit);
    
    // Get player facing
    float playerFacing = vanilla1121_unitFacing(playerUnit);

    // Calculate angle to target
    float dx = targetPos.x - playerPos.x;
    float dy = targetPos.y - playerPos.y;
    float angleToTarget = std::atan2(dy, dx);

    // Calculate relative angle (difference between facing and target direction)
    float relativeAngle = angleToTarget - playerFacing;

    // Normalize to -π to π range
    while (relativeAngle > M_PI) relativeAngle -= 2.0f * static_cast<float>(M_PI);
    while (relativeAngle < -M_PI) relativeAngle += 2.0f * static_cast<float>(M_PI);

    // Convert to degrees
    float degrees = relativeAngle * (180.0f / static_cast<float>(M_PI));

    // Return the raw angle in degrees (rounded to nearest integer)
    // Positive values = target is to the right
    // Negative values = target is to the left
    // Values near ±180 = target is behind
    // Values near 0 = target is in front
    return static_cast<int>(degrees);
}

// String API wrapper
int UnitXP_relativeDirection(const string unit0, const string unit1) {
    uint64_t guid0 = 0, guid1 = 0;

    if (unit0.empty() || unit1.empty()) {
        return -999;
    }

    if (unit0.find(u8"0x") != unit0.npos) {
        stringstream ss{ unit0 };
        ss >> hex >> guid0;
        if (ss.fail()) {
            return -999;
        }
    } else {
        guid0 = UnitGUID(unit0.data());
        if (guid0 == 0) {
            return -999;
        }
    }

    if (unit1.find(u8"0x") != unit1.npos) {
        stringstream ss{ unit1 };
        ss >> hex >> guid1;
        if (ss.fail()) {
            return -999;
        }
    } else {
        guid1 = UnitGUID(unit1.data());
        if (guid1 == 0) {
            return -999;
        }
    }

    return UnitXP_relativeDirection(
        reinterpret_cast<void*>(vanilla1121_getVisiableObject(guid0)),
        reinterpret_cast<void*>(vanilla1121_getVisiableObject(guid1)));
}
