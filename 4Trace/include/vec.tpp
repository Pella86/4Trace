#ifndef VEC_T
#define VEC_T

# define M_PI           3.14159265358979323846

#include <iostream>
#include <cstdint>
#include <vector>
#include <cmath>
#include <numeric>
#include <exception>

#include <utils.h>


template<typename T, size_t dim>
class Vector{
    T coords[dim];
public:
    Vector() {
        for(size_t i = 0; i < dim; i++){
            coords[i] = T(0);
        }
    }

    const size_t dimension() const {return dim;}

    T operator[](size_t index) const {
        return coords[index];
    }

    T& operator[](size_t index) {
        return coords[index];
    }

    T&  operator=(const T& other){
        if(this != other){
            for(size_t i = 0; i < dim; i++){
                coords[i] = other[i];
            }
        }
        return *this;
    }

    bool operator==(const Vector<T, dim>& rhs) const {
        for(size_t i = 0; i < dim; i++){
            if(coords[i] != rhs[i]){
                return false;
            }
        }
        return true;
    }

    bool cmp_close(const Vector<T, dim>& rhs) const {
        for(size_t i = 0; i < dim; i++){
            if(!close(coords[i], rhs[i])){
                return false;
            }
        }
        return true;
    }

    Vector<T, dim>& operator+=(const Vector<T, dim>& rhs){
        for(size_t i = 0; i < dim; i++){
            coords[i] += rhs[i];
        }
        return *this;
    }

    friend Vector<T, dim> operator+(Vector<T, dim> lhs, const Vector<T, dim>& rhs ){
        lhs += rhs;
        return lhs;
    }

    Vector<T, dim>& operator-=(const Vector<T, dim>& rhs){
        for(size_t i = 0; i < dim; i++){
            coords[i] -= rhs[i];
        }
        return *this;
    }

    friend Vector<T, dim> operator-(Vector<T, dim> lhs, const Vector<T, dim>& rhs ){
        lhs -= rhs;
        return lhs;
    }

    Vector<T, dim>& operator*=(T scalar){
        for(size_t i = 0; i < dim; i++){
            coords[i] = coords[i] * scalar;
        }
        return *this;
    }

    friend Vector<T, dim> operator*(Vector<T, dim> lhs, T scalar){
        lhs *= scalar;
        return lhs;
    }

    double length_squared() const {
        double l = 0;
        for(size_t i = 0; i < dim; i++){
            l += pow(coords[i], 2);
        }
        return l;
    }

    double length() const {return sqrt(length_squared());}

    void normalize(){
        double l = length();
        if(l == 0){
            throw "Normalize divide by 0";
        }

        double invl = 1/l;
        for(size_t i = 0; i < dim; i++){
            coords[i] *= invl;
        }
    }

    T dot(const Vector<T, dim>& other) const {

        T sum = T(0);
        for(size_t i = 0; i < dim; i ++){
            sum += coords[i] * other[i];
        }
        return sum;
    }

    double angle(const Vector<T, dim>& other, bool check_norm = true) const{
        if(check_norm){
            if(!close(length(), 1)){
                throw "Angle calculation: the vector is not normalized";
            }

            if(!close(other.length(), 1)){
                throw "Angle calculation: the other vector is not normalized";
            }
        }
        return acos(dot(other));
    }

    friend std::ostream& operator<<(std::ostream& os, const Vector<T, dim> &other){
        os << "(";
        for(size_t i = 0; i < dim; i++){
            os << other[i];
            if(i != dim - 1){
                os << ",";
            }
        }
        os << ")";
        return os;
    }
};


template<typename T>
class V2 : public Vector<T, 2> {
public:
    V2(T x, T y) {
        (*this)[0] = x;
        (*this)[1] = y;
    }

    V2(Vector<T, 2> v){
        for(size_t i = 0; i < 2; i++){
           (*this)[i] = v[i];
        }
    }

    T x(){ return (*this)[0];}
    void x(T ix){ (*this)[0] = ix;}
    T y(){ return (*this)[1];}
    void y(T iy){ (*this)[1] = iy;}
};

using V2d = V2<double> ;


template<typename T>
class V3 : public Vector<T, 3> {
public:
    V3(T x, T y, T z) {
        (*this)[0] = x;
        (*this)[1] = y;
        (*this)[2] = z;
    }

    V3(Vector<T, 3> v){
        for(size_t i = 0; i < 3; i++){
           (*this)[i] = v[i];
        }
    }

    T x(){ return (*this)[0];}
    void x(T ix){ (*this)[0] = ix;}
    T y(){ return (*this)[1];}
    void y(T iy){ (*this)[1] = iy;}
    T z(){ return (*this)[2];}
    void z(T iz){ (*this)[2] = iz;}

    V3 cross(V3 other){
        T rx = y() * other.z() - z() * other.y();
        T ry = z() * other.x() - x() * other.z();
        T rz = x() * other.y() - y() * other.x();

        return V3(rx, ry, rz);
    }

};

using V3d = V3<double>;

#define vDIM 4

template<typename T>
class V4 : public Vector<T, vDIM> {
public:
    V4(T x, T y, T z, T w) {
        (*this)[0] = x;
        (*this)[1] = y;
        (*this)[2] = z;
        (*this)[3] = w;
    }

    V4(Vector<T, 4> v){
        for(size_t i = 0; i < 4; i++){
           (*this)[i] = v[i];
        }
    }

    T x(){ return (*this)[0];}
    void x(T ix){ (*this)[0] = ix;}
    T y(){ return (*this)[1];}
    void y(T iy){ (*this)[1] = iy;}
    T z(){ return (*this)[2];}
    void z(T iz){ (*this)[2] = iz;}
    T w(){ return (*this)[3];}
    void w(T iw){ (*this)[3] = iw;}
};

using V4d = V4<double>;

#endif // VEC_T

