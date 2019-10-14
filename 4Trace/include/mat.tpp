#ifndef MAT_T
#define MAT_T

#include <iostream>


template<typename T = double, size_t M = 1, size_t N = 1>
class Matrix{
    T mat[M][N];
    const size_t MROW = M;
    const size_t NCOL = N;

    size_t calc_index(size_t i, size_t j){return i * M + j;}

public:

    const size_t rows() const {return MROW;}
    const size_t cols() const {return NCOL;}

    Matrix(){
        for(size_t i = 0; i < M; i++){
            for(size_t j = 0; j < N; j++){
                mat[i][j] = T(0);
            }
        }
    }

    Matrix(T imat[M][N]){
        for(size_t i = 0; i < M; i++){
            for(size_t j = 0; j < N; j++){
                mat[i][j] = imat[i][j];
            }
        }
    }

    const T* operator[](size_t i) const {return mat[i];}

    T* operator[](size_t i) {return mat[i];}

    template<typename MatrixType>
    Matrix<T, M, N>& operator*=(const MatrixType rhs_mat){
        if(NCOL == rhs_mat.MROW){throw "Matrix mult impossible M != N";}

        (*this) = (*this) * rhs_mat;

        return this;
    }

    template<typename MatrixType>
    friend Matrix<T, M, N> operator*(Matrix<T, M, N> A, MatrixType const& B){
        if(A.cols() != B.rows()){throw "Matrix mult impossible (N col != M row)";}

        Matrix<T, A.rows(), B.cols()> res_mat;

        for(size_t i = 0; i < A.MROW; i++){
            for(size_t j = 0; j < B.NCOL; j++){
                T sum = 0;
                for(size_t jj = 0; jj < B.NCOL; jj++){
                        sum += A[i][jj] * B[jj][j];
                }

                res_mat[i][j] = sum;
            }
        }
        return res_mat;
    }
    friend std::ostream& operator<<(std::ostream& os, Matrix<T, M, N> m){

        for(size_t i = 0; i < M; i++){
            for(size_t j = 0; j < N - 1; j++){
                std::cout << m[i][j] << " ";
            }
            std::cout << m[i][N - 1] << std::endl;
        }
        std::cout << std::endl;

        return os;
    }




};

#endif // MAT_T
