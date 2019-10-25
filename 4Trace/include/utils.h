#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>

constexpr double M_PI = 3.14159265358979323846; // i should rename this or enable a if else to detecet PC or UNIX

bool close(double value, double bound);

inline double radians(double deg){return deg * M_PI / 180;}

template <typename T>
std::string numtostr ( T Number ) {
    std::ostringstream ss;
    ss << Number;
    return ss.str();
}

#endif // UTILS_H
