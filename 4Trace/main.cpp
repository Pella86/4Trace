#include <iostream>
#include <vec.tpp>

#include <UnitTest.h>

using namespace std;

int main()
{
    cout << "Hello world!" << endl;

    UnitTest ut;
    ut.test_vectors();
    ut.test_mat();

    return 0;
}
