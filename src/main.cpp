// clang-format off
#include "CrazyEyes.hpp"
// clang-format on

CrazyEyes eyes;

void forever(void) {
    while (true) {
        delay(1);
    }
}

void setup(void) {
    if (!eyes.begin()) {
        forever();
    }
}

void loop(void) {
    eyes.update();
    delay(100);
}
