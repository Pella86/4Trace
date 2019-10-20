#include "UnitTest.h"

#include <iostream>
#include <fstream>
#include <iomanip>

#include <vec.tpp>
#include <mat.tpp>
#include <bmp.h>

using namespace std;


#define utv_test(title, cond) do{ \
    cout << "--- " << title << " ---" << endl; \
    cout << "Test: " << #cond << endl; \
    bool test_passed = true; \
    try{ \
        if(!(cond)) throw "Test: failed"; \
    } catch(const char* e){ \
        cout << e << endl; \
        test_passed = false; \
    } \
    if (test_passed) cout << "Test: passed" << endl; \
    }while(0);

void UnitTest::test_vectors(){
    V2<int> v2i{1, 2};
    utv_test("Test operatror[] integer vector", v2i[0] == 1);
    utv_test("Test y() integer vetor", v2i.y() == 2);

    V2<float> v2f{4.0, 5.0};
    utv_test("Test operator[] float vector", v2f[0] == 4.0);
    utv_test("Test y() float vector", v2f.y() == 5.0);

    // https://www.mathsisfun.com/algebra/vectors.html
    V2d magtest{6.0, 8.0};
    utv_test("Test magnitude", magtest.length() == 10.0);

    //http://www.fundza.com/vectors/normalize/
    V3d normtest{3., 1., 2.};
    normtest.normalize();
    utv_test("Test normalization", normtest.length() == 1);


    V2d angletest_H{2., 0.};
    V2d angletest_V{0., 3.};

    angletest_H.normalize();
    angletest_V.normalize();

    utv_test("Test angle", angletest_H.angle(angletest_V) == M_PI / 2.);

    // https://mathinsight.org/cross_product_examples
    V3d crosstest_a{3., -3., 1};
    V3d crosstest_b{4., 9., 2.};
    V3d crosstest_c{-12., 12., -4.};

    V3d crosstest_exp{-15., -2., 39.};
    V3d crosstest_res = crosstest_a.cross(crosstest_b);
    utv_test("Test cross product", crosstest_res == crosstest_exp);
    utv_test("Test area", close(crosstest_res.length(), 41.8330013267));

    V3d crosstest_exp2{0., 0., 0.};
    utv_test("Test cross product", crosstest_a.cross(crosstest_c) == crosstest_exp2);

    // https://www.onlinemathlearning.com/vector-addition.html
    V2d addtest_a{2, 3};
    V2d addtest_b{2, -2};

    V2d addtest_exp{4, 1};
    V2d addtest_res = addtest_a + addtest_b;

    utv_test("Test addition", addtest_res == addtest_exp);

    V2d subtest_a{3., 4.};
    V2d subtest_b{5., -1.};

    V2d subtest_exp{-2., 5};
    V2d subtest_res = subtest_a - subtest_b;

    utv_test("Test subtraction", subtest_res == subtest_exp);

    V3d test_scal_a{1., 2., 4.};
    utv_test("Test scalar multiplication", test_scal_a * 5. == V3d(5., 10., 20.))
}



void UnitTest::test_mat(){

    Matrix<double> mat(3,4);

    cout << mat << endl;


    //https://www.mathsisfun.com/algebra/matrix-multiplying.html
    Matrix<double> tprod_A({{1, 2, 3}, {4, 5, 6}});

    Matrix<double> tprod_B({{7, 8}, {9, 10}, {11, 12}});

    Matrix<double> tprod_exp({{58, 64},{139, 154}});
    Matrix<double> tprod_res = tprod_A * tprod_B;

    utv_test("Test matrix product", tprod_exp == tprod_res);


    Matrix<int> trtest({{1, 2}, {3, 4}, {5, 6}});
    Matrix<int> trtestexp({{1, 3, 5}, {2, 4, 6}});
    Matrix<int> transposed = trtest.transpose();

    utv_test("Test matrix transposition", transposed == trtestexp);

    V3d vec(1, 2, 3);
    Matrix44<double> transmat({{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {2, 2, 2, 1}});

    V3d res = vec * transmat * transmat;

    utv_test("Test vector matrix multiplication", res == V3d(5., 6., 7.));
}



void UnitTest::test_bmp(){

    constexpr size_t size_file_header = 14;
    constexpr size_t size_info_header = 40;

    string image_filename = "C:\\Users\\Mauro\\Desktop\\Vita Online\\Programming\\4trace\\4Trace\\test_bmp_images\\test_24bit_9x5.bmp";
    ifstream bmpfile (image_filename, ios::binary);

    // test headers
    if(bmpfile.is_open()){

        cout << "File is open" << endl;

        // red file header
        char b_file_header[size_file_header];
        bmpfile.read(b_file_header, size_file_header);

        bmp::FileHeader fh(b_file_header);

        cout << fh << endl;

        // read DIB header
        char b_info_header[size_info_header];
        bmpfile.read(b_info_header, size_info_header);

        bmp::InfoHeader ih(b_info_header);

        cout << ih << endl;
    }

    // test Image class
    bmp::Image im(image_filename);

    string image_copy_filename = "C:\\Users\\Mauro\\Desktop\\Vita Online\\Programming\\4trace\\4Trace\\test_bmp_images\\test_24bit_9x5_copy.bmp";
    im.write(image_copy_filename);
}
