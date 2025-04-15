#ifndef COMMON_HPP
#define COMMON_HPP

#include <cstdint>
#include <vector>

typedef struct {
    uint8_t r, g, b; 
} Rgb;


template <typename T> struct Mat {
    unsigned width, height;
    std::vector<std::vector<T>> data;
    Mat(unsigned) = delete;
    Mat(unsigned w, unsigned h) : width(w), height(h) {
        data.resize(height);
        for (auto& row : data) row.resize(width);
    }
    T at(unsigned x, unsigned y) { return data[x][y]; }
    // arrows the row to be moified 
    std::vector<T>& operator[](unsigned y) { return data[y]; }
    // returns a copy
    const std::vector<T>& operator[](unsigned y) const { return data[y]; }
};

#endif // COMMON_HPP
