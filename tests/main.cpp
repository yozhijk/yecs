#include <iostream>

#include "gtest/gtest.h"
#include "tests/tests.h"
#include "yecs/yecs.h"

using namespace std;
using namespace yecs;

struct Position
{
    float x, y;
};

struct Velocity
{
    float x, y;
};

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}