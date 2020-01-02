#include <iostream>

#include "gtest/gtest.h"
#include "tests/tests.h"
#include "yecs/yecs.h"

using namespace std;
using namespace yecs;

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}