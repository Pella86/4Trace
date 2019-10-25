#ifndef MAT_T
#define MAT_T

#include <iostream>
#include <vector>
#include <exception>
#include <string>

#include "vec.tpp"

class MatrixError : public std::exception {
private:
    std::string message;
public:

    MatrixError(std::string msg) : message(msg) {};

    virtual const char* what() const throw() {
        return message.c_str();
    }

};

/*******************************************************************************
Matrix class
  MxN general matrix
*******************************************************************************/

template<typename T>
class Matrix{
private:
    std::vector<T> mat;

    size_t MROW;
    size_t NCOL;

    inline size_t calc_index(size_t i, size_t j) const {
        return i * NCOL + j;
    }


public:
    size_t rows() const {return MROW;}
    size_t cols() const {return NCOL;}

    // ----------------------------- c'tors -----------------------------------
    Matrix() : MROW(0), NCOL(0){}

    Matrix(size_t m, size_t n) : MROW(m), NCOL(n) {
        // initialize the matrix
        for(size_t i = 0; i < MROW; i++){
            for(size_t j = 0; j < NCOL; j++){
                mat.push_back(T(0));
            }
        }
    }

    Matrix(std::initializer_list<std::initializer_list<T>> lst) :
        MROW(lst.size()),
        NCOL(lst.size()? lst.begin()->size(): 0) {
        // initialize the matrix

        for(const auto& l : lst){
            for(const auto& v : l){
                mat.push_back(v);
            }
        }
    }

    // matrix form vector c'tor
    template<size_t dim>
    Matrix(Vector<T, dim> const& v) :
        MROW(1),
        NCOL(v.dimension())
    {
        for(size_t i = 0; i < MROW; i++){
            for(size_t j = 0; j < NCOL; j++){
                mat.push_back(v[j]);
            }
        }
    }

    // ----------------------------- set/getters -------------------------------

    void set(size_t i, size_t j, T x){
        mat[calc_index(i, j)] = x;
    }

    T get(size_t i, size_t j) const {
        return mat[calc_index(i, j)];
    }

    // ------------------------------ operators --------------------------------

    friend Matrix operator*(Matrix const& A, Matrix const& B){

        if(A.cols() != B.rows()){throw MatrixError("Matrix multiplication impossible A.M != B.N");}

        Matrix res(A.rows(), B.cols());

        for(size_t i = 0; i < A.rows(); i++){
            for(size_t j = 0; j < B.cols(); j++){
                T sum = T(0);
                for(size_t mult_idx = 0; mult_idx < A.cols(); mult_idx++){
                    sum += (A.get(i, mult_idx) * B.get(mult_idx, j));
                }
                res.set(i, j, sum);
            }
        }
        return res;
    }

    bool operator==(const Matrix& imat){
        return mat == imat.mat;
    }

    // ------------------------------ methods --------------------------------
    Matrix transpose(){
        Matrix tr(NCOL, MROW);

        for(size_t i = 0; i < MROW; i++){
            for(size_t j = 0; j < NCOL; j++){
                tr.set(j, i, get(i, j));
            }
        }

        return tr;
    }

    friend std::ostream& operator<<(std::ostream& os, const Matrix<T>& mat){
        for(size_t i = 0; i < mat.rows(); i++){
            for(size_t j = 0; j < mat.cols() - 1; j++){
                os << mat.get(i, j) << " ";
            }
            os << mat.get(i, mat.cols() - 1);
            os << std::endl;
        }

        return os;
    }
};

/*******************************************************************************
 Square matrix class
*******************************************************************************/

template<typename T>
class SquaredMatrix : public Matrix<T> {

public:
    SquaredMatrix(size_t m) : Matrix<T>(m, m){};

    SquaredMatrix(std::initializer_list<std::initializer_list<T>> lst) : Matrix<T>(lst) {
        if(this->rows() != this->cols()){throw MatrixError("Not a square matrix.");}
    }

    void form_identity(){
        for(size_t i = 0; i < this->rows(); i++){
            for(size_t j = 0; j < this->cols(); j++){
                if(i == j) {this->set(i, j, T(1));}
                else {this->set(i, j, T(0));}
            }
        }
    }
};

/*******************************************************************************
 4x4 matrix class
*******************************************************************************/


template<typename T>
class Matrix44 : public SquaredMatrix<T> {
public:
    Matrix44() : SquaredMatrix<T>(4) {};
    Matrix44(std::initializer_list<std::initializer_list<T>> lst) : SquaredMatrix<T>(lst) {};

};


/*******************************************************************************
 Matrix vector multiplication
*******************************************************************************/

template<typename T>
V3<T> operator*(V3<T> const& vec, Matrix44<T> const& mat){
    // transform vector in row major matrix
    Matrix<T> fromv(1, 4);
    for(size_t i = 0; i < 3; i++){
        fromv.set(0, i, vec[i]);
    }
    fromv.set(0, 3, T(1));

    // calculate the multiplication
    Matrix<T> res = fromv * mat;

    // return the vector
    return V3<T>(res.get(0, 0), res.get(0, 1), res.get(0, 2));
}

#endif // MAT_T
