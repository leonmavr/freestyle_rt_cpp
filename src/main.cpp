#include <iostream>
#include "vec.hpp"
#include "mat3x3.hpp"

int main (int argc, char *argv[]) {
    Mat3x3 mat1, mat2;
    mat1.rows[0] = {3,2,1};
    auto mat_t = mat1.Transpose();
    std::cout << mat_t << std::endl;
    return 0;
}
