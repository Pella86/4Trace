#include <iostream>
#include <cmath>


//#include <UnitTest.h>

#include "vec.tpp"
#include "bmp.h"

using namespace std;

class Sphere{

};


double radians(double deg){
    return deg * M_PI / 180;
}


using Color = V3d;


Color trace(const V3d& rayorig, const V3d& raydir, const vector<Sphere>& spheres, const int& depth) {


    return Color(0);
};

void render(vector<Sphere> spheres){
    unsigned width = 16, height = 12;
    double invWidth = 1 / double(width), invHeight = 1 / double(height);

    double fov = 30.;
    double angle = tan(radians(fov) / 2.);

    double aspect_ratio = width / double(height);

    bmp::Image img(width, height);

    for(size_t i = 0; i < width; i++){
        for(size_t j = 0; j < height; j++){
            // norm to 1
            double half_image_px_x = (i + 0.5) * invWidth;
            double half_camera_px_x = 2 * half_image_px_x - 1;
            double adjusted_camera_px_x = half_camera_px_x * angle * aspect_ratio;

            double half_image_px_y = (j + 0.5) * invHeight;
            double half_camera_px_y = 2 * half_image_px_y - 1;
            double adjusted_camera_px_y = half_camera_px_y * angle;

            V3d raydir(adjusted_camera_px_x, adjusted_camera_px_y, -1);
            raydir.normalize();

            Color pixel = trace(V3d(0), raydir, spheres, 0);

            bmp::Color bmppix(pixel * 255);

            img.pixelArray.set(i, j, bmppix);
        }

    }



    img.write("test_render.bmp");






}




int main()
{
    // renderer

    vector<Sphere> s;

    render(s);


    cout << "Hello world!" << endl;

//    UnitTest ut;
//    ut.test_vectors();
//    ut.test_mat();
//    ut.test_bmp();

    return 0;
}
