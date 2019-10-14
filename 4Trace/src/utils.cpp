#include "utils.h"

constexpr double epsilon = 1e-10;

bool close(double value, double bound){
    if(value > (bound - epsilon) && value < (bound + epsilon)){
        return true;
    }
    else{
        return false;
    }
}

