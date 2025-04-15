#ifndef PPM_WRITER
#define PPM_WRITER

#include "../src/common.hpp"
#include <string>
#include <fstream>
#include <iostream>

namespace Ppm {

void WriteTo(const std::string& filename, Mat<Rgb> mat) {
    // dumps a matrix as a .ppm format image - 
    // see https://netpbm.sourceforge.net/doc/ppm.html
    std::ofstream file(filename, std::ios::out);
    if (!file.is_open())
        throw std::runtime_error("ERROR: Ppm writer cannot write to file" + filename);
    
    file << "P3" << std::endl;
    file << mat.width << " " << mat.height << std::endl;
    constexpr unsigned max_intensity = 255;
    file << max_intensity << std::endl;
    
    // Write pixel data
    for (int y = 0; y < mat.height; y++) {
        for (int x = 0; x < mat.width; x++) {
            const Rgb& pixel = mat[y][x];
            file << static_cast<int>(pixel.r) << " "
                 << static_cast<int>(pixel.g) << " "
                 << static_cast<int>(pixel.b) << " ";
        }
        file << std::endl;
    }
    file.close();
}

} // namespace


#endif // PPM_WRITER
