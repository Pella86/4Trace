#include <iostream>
#include <cmath>

#include <UnitTest.h>

#include "3D_render.h"

#include "vec.tpp"
#include "bmp.h"


using namespace std;

using Color = V3d;

constexpr double MAX_RAY_DEPTH = 5;

/*******************************************************************************
Sphere class
*******************************************************************************/

template<size_t dim>
struct Sphere{

    Vector<double, dim> center;
    double radius, radius2; // radius and radius squared
    Color surface, emission;
    double transparency, reflection;

    Sphere(const Vector<double, dim>& center,
          double radius,
          const Color& surface,
          const Color& emission,
          double transparency,
          double reflection ):
              center(center),
              radius(radius),
              radius2(radius * radius),
              surface(surface),
              emission(emission),
              transparency(transparency),
              reflection(reflection)
              {}


    bool intersect(const Vector<double, dim>& rayorig, const Vector<double, dim>& raydir, double& t0, double& t1) const{
        Vector<double, dim> l = center - rayorig;
        double tca = l.dot(raydir);
        if(tca < 0) {return false;}
        else{

            double d2 = l.dot(l) - tca * tca;
            if(d2 > radius2) {return false;}
            else{
                double thc = sqrt(radius2 - d2);
                t0 = tca - thc;
                t1 = tca + thc;
                return true;
            }
        }
    }
};


/*******************************************************************************
trace function:
    calculates the color of the ray coming from a pixel
*******************************************************************************/


template<size_t dim>
Color trace(const Vector<double, dim>& rayorig, const Vector<double, dim>& raydir, const vector<Sphere<dim>>& spheres, const int& depth) {

    double tnear = INFINITY; // distance to the closest sphere
    const Sphere<dim>* sphere = NULL;

    // calculate the intersection parameter with the closest sphere
    for(size_t i = 0; i < spheres.size(); i++ ){
        double t0 = INFINITY, t1 = INFINITY;
        if(spheres[i].intersect(rayorig, raydir, t0, t1)){
            if(t0 < 0) t0 = t1;
            if(t0 < tnear){
                tnear = t0; // distance from rayorig
                sphere = &spheres[i]; // closest sphere
            }
        }
    }

    // if there's no sphere return the background color
    if(!sphere) {
        return Color(0, 0.2, 0.2);
    }
    else{
        Color surfaceColor(0);
        Vector<double, dim> phit = rayorig + raydir * tnear;
        Vector<double, dim> nhit = phit - sphere->center;
        nhit.normalize();
        double bias = 1e-4;

        // switch to decide if the sphere is hit from the inside ths will flip
        // the normal
        //bool inside = false;
        if(raydir.dot(nhit) > 0){
            nhit = -nhit;
            //inside = true;
        }

        if((sphere->transparency > 0 || sphere->reflection > 0) && depth < MAX_RAY_DEPTH){

        }
        else{
            // the sphere has a diffuse color (neither reflective nor transparent)
            for(size_t i = 0; i < spheres.size(); i++){

                // if is a light (emission > 0)
                if( !(spheres[i].emission == Color(0))){

                    Color transmission(1); // 0 if there is an object obstructing the light ray
                    Vector<double, dim> light_direction = spheres[i].center - phit;
                    light_direction.normalize();

                    for(size_t j = 0; j < spheres.size(); j++){

                        if(i != j){
                            double t0, t1;

                            if(spheres[j].intersect( (phit + nhit * bias), light_direction, t0, t1) ){
                                transmission = Color(0);
                                break;
                            }
                        }

                    }

                    // calculate how the light changes the color
                    surfaceColor += sphere->surface * transmission * max(double(0), nhit.dot(light_direction)) * spheres[i].emission;
                }

            }
        }
        return surfaceColor + sphere->emission;
    }
};

/*******************************************************************************
render function
    renders the image and saves it
*******************************************************************************/

template<size_t dim>
void render(vector<Sphere<dim>> spheres, string filename){
    unsigned width = 640, height = 480;

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

            Vector<double, dim> raydir;
            raydir[0] = adjusted_camera_px_x;
            raydir[1] = adjusted_camera_px_y;
            raydir[2] = -1;

            raydir.normalize();

            Color pixel = trace(Vector<double, dim>(0), raydir, spheres, 0);

            // limit the color to a value between 0 and 1;
            pixel.x(min(1., pixel.x()));
            pixel.y(min(1., pixel.y()));
            pixel.z(min(1., pixel.z()));


            // convert the color to bmp color (8 bit per color)
            bmp::Color bmppix(pixel * 255);

            img.pixelArray.set(i, height-j, bmppix);
        }

    }

    img.write(filename);
}


/*******************************************************************************
scenes
*******************************************************************************/

void example_animation(){
    for(int i = -5; i < 5; i++){

        cout << "rendering image " << i << " ..."<< endl;
        vector<Sphere<4>> spheres;

        spheres.push_back(Sphere<4>(V4d(0, -10004, -20, 0),  10000, Color(0, 1, 0), Color(0), 0, 0));
        spheres.push_back(Sphere<4>(V4d(0,      0, -20, 0),      4, Color(1, 0, 0), Color(0), 0, 0));
        spheres.push_back(Sphere<4>(V4d(5,     -1, -15, 0),      2, Color(0, 0, 1), Color(0), 0, 0));
        spheres.push_back(Sphere<4>(V4d(0,     20, -30, 0 + i*2),      3, Color(0),       Color(3), 0, 0));


        render<4>(spheres, string("ani_test") + numtostr(i + 5) + string(".bmp") );

    }

}

void draw_axis(){

    vector<Sphere<4>> spheres;

    // background sphere
    spheres.push_back(Sphere<4>(V4d(0, -10004, -20, 0),  10000, Color(0, 1, 1), Color(0), 0, 0));
    // light
    spheres.push_back(Sphere<4>(V4d(0,     20, 10, 0 ),     3, Color(0),       Color(3), 0, 0));

    for(int i = -10; i < 10; i ++){
        spheres.push_back(Sphere<4>(V4d(i, 0, -20,     0),      .1, Color(1, 0, 0), Color(0), 0, 0));
        spheres.push_back(Sphere<4>(V4d(0, i, -20,     0),      .1, Color(0, 1, 0), Color(0), 0, 0));
        spheres.push_back(Sphere<4>(V4d(0, 0, -20 + i, 0),      .1, Color(0, 0, 1), Color(0), 0, 0));
        spheres.push_back(Sphere<4>(V4d(i, 0, -20,     i/5.),      1, Color(1, 1, 0), Color(0), 0, 0));

    }


    render<4>(spheres, "test_render_draw_axis.bmp" );





}






int main()
{
    cout << "START RENDER" << endl;
    // renderer

    draw_axis();



    cout << "Hello world!" << endl;


//    UnitTest ut;
//    ut.test_vectors();
//    ut.test_mat();
//    ut.test_bmp();

    return 0;
}
