#include <iostream>
#include "math.hpp"

int main (int argc, char *argv[]) {
    Vec3f v1 = {1, 1, 0};
    Vec3f v2 = {1, 0, 0};
    v1.ReflectAbout(v2);
    v2+=v1;
    std::cout << v2.x << std::endl;

    return 0;
}
