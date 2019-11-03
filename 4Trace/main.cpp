#include <iostream>
#include <cmath>
#include <map>

#include <UnitTest.h>

#include "3D_render.h"

#include "vec.tpp"
#include "bmp.h"
#include "Glyphs.h"


using namespace std;

using Color = V3d;

constexpr double MAX_RAY_DEPTH = 5;

double mix(double a, double b, double mix){
    return b * mix + a * (1 - mix);
}

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
        bool inside = false;
        if(raydir.dot(nhit) > 0){
            nhit = -nhit;
            inside = true;
        }

        if((sphere->transparency > 0 || sphere->reflection > 0) && depth < MAX_RAY_DEPTH){
            double facingratio = -raydir.dot(nhit);
            double fresneleffect = mix(pow(1 - facingratio, 3), 1, sphere->reflection);

            Vector<double, dim> refldir = raydir - nhit * 2 * raydir.dot(nhit);
            refldir.normalize();

            Color reflection = trace(phit + nhit * bias, refldir, spheres, depth + 1);

            Color refraction(0);

            if(sphere->transparency > 0){
                double ior = 1.1;
                double eta = (inside)? ior : 1;
                double cosi = -nhit.dot(raydir);
                double k = 1 - eta * eta * (1 - cosi * cosi);
                Vector<double, dim> refdir = raydir * eta + nhit * (eta * cosi - sqrt(k));
                refdir.normalize();

                refraction = trace(phit - nhit * bias, refdir, spheres, depth + 1);
            }

            surfaceColor = (reflection * fresneleffect +
                            refraction * (1 - fresneleffect) * sphere->transparency) *
                            sphere->surface;

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
bmp::Image render(vector<Sphere<dim>> spheres){
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

            img.pixelArray.set(i, height- 1 - j, bmppix);
        }

    }

    return img;
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


        bmp::Image img = render<4>(spheres);
        img.write(string("ani_test") + numtostr(i + 5) + string(".bmp") );

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


    bmp::Image img = render<4>(spheres);
    img.write("test_render_draw_axis.bmp");

}


void draw_animation(){
    vector<Sphere<4>> spheres;

    for(int i = -10; i < 10; i ++){
        cout << "rendering frame: " << i + 10 << endl;
        // background sphere
        spheres.push_back(Sphere<4>(V4d(0,  -10004, -20, 0), 10000, Color(0, 1, 1), Color(0), 0, 0));
        // light
        spheres.push_back(Sphere<4>(V4d(0,      20, -20, 0 ),     3, Color(0),       Color(3), 0, 0));

        spheres.push_back(Sphere<4>(V4d(i / 2.0, 0, -30, 0),     4, Color(1, 0, 0), Color(0), 0, 0));
        spheres.push_back(Sphere<4>(V4d(5,      -1, -15, 0),     2, Color(0, 0, 1), Color(0), 0, 0));

        // actual thing
        spheres.push_back(Sphere<4>(V4d(0, 0, -20,  i / 5.),     2.5, Color(1, 1, 1), Color(0), 1.5, .1));

        bmp::Image img = render<4>(spheres);

        Glyphs gly;
        stringstream ss;
        ss << "Sphere position: " << V4d(0, 0, -20,     i / 5.);
        cout << ss.str() << endl;

        gly.imprint(img, ss.str(), V2<size_t>(320, 0), 0.33);
        img.write( "./test_ani/ani" + numtostr(i + 10) + ".bmp" );

        spheres.clear();
    }
}


void test_reflection(){
    std::vector<Sphere<3>> spheres;
    // position, radius, surface color, reflectivity, transparency, emission color
    spheres.push_back(Sphere<3>(V3d( 0.0, -10004, -20), 10000, Color(0.20, 0.20, 0.20), Color(0), 0, 0.0));
    spheres.push_back(Sphere<3>(V3d( 0.0,      0, -20),     4, Color(1.00, 0.32, 0.36), Color(0), 0, 0.5));
    spheres.push_back(Sphere<3>(V3d( 5.0,     -1, -15),     2, Color(0.90, 0.76, 0.46), Color(0), 0, 0.0));
    spheres.push_back(Sphere<3>(V3d( 5.0,      0, -25),     3, Color(0.65, 0.77, 0.97), Color(0), 0, 0.2));
    spheres.push_back(Sphere<3>(V3d(-5.5,      0, -15),     3, Color(0.90, 0.90, 0.90), Color(0), 0, 0.0));
    // light
    spheres.push_back(Sphere<3>(V3d( 0.0,     20, -20),     3, Color(0), Color(3), 0, 0));
    bmp::Image ren = render<3>(spheres);
    ren.write("test_render_reflection.bmp");
}

void test_reflection_4(){
    std::vector<Sphere<4>> spheres;
    // position, radius, surface color, reflectivity, transparency, emission color
    spheres.push_back(Sphere<4>(V4d( 0.0, -10004, -20, 0), 10000, Color(0.20, 0.20, 0.20), Color(0), 0, 0.0));
    spheres.push_back(Sphere<4>(V4d( 0.0,      0, -20, 0),     5, Color(1.00, 0.32, 0.36), Color(0), 0, 0.5));
    spheres.push_back(Sphere<4>(V4d( 5.0,     -1, -15, 0),     2, Color(0.90, 0.76, 0.46), Color(0), 0, 0.0));
    spheres.push_back(Sphere<4>(V4d( 5.0,      0, -25, 0),     3, Color(0.65, 0.77, 0.97), Color(0), 0, 0.2));
    spheres.push_back(Sphere<4>(V4d(-5.5,      0, -15, 0),     3, Color(0.90, 0.90, 0.90), Color(0), 0, 0.0));
    // light
    spheres.push_back(Sphere<4>(V4d( 0.0,     20, -20, 0),     3, Color(0), Color(3), 0, 0));

    bmp::Image ren = render<4>(spheres);
    ren.write("test_render_reflection_4.bmp");
}

void render_cube_vertex(){

    std::vector<Sphere<4>> spheres;

    // background sphere
    spheres.push_back(Sphere<4>(V4d(0,  -10004, -20, 0), 10000, Color(0, 1, 1), Color(0), 0, 0));
    // light
    spheres.push_back(Sphere<4>(V4d(0,      20, -15, 0 ),     3, Color(0),       Color(3), 0, 0));

//    spheres.push_back(Sphere<4>(V4d(2,  0,  -20, 0),  .5, Color(1, 0, 0), Color(0), 0, 0));
//    spheres.push_back(Sphere<4>(V4d(-2, 0,  -20, 0),  .5, Color(1, 0, 0), Color(0), 0, 0));
//    spheres.push_back(Sphere<4>(V4d(0,  2,  -20, 0),  .5, Color(0, 1, 0), Color(0), 0, 0));
//    spheres.push_back(Sphere<4>(V4d(0,  -2, -20, 0),  .5, Color(0, 1, 0), Color(0), 0, 0));
//    spheres.push_back(Sphere<4>(V4d(0,  0,  -22, 0),  .5, Color(0, 0, 1), Color(0), 0, 0));
//    spheres.push_back(Sphere<4>(V4d(0,  0,  -18, 0),  .5, Color(0, 0, 1), Color(0), 0, 0));
//    spheres.push_back(Sphere<4>(V4d(0,  0,  -20, 2),  .5, Color(1, 0, 1), Color(0), 0, 0));
//    spheres.push_back(Sphere<4>(V4d(0,  0,  -20, -2), .5, Color(1, 0, 1), Color(0), 0, 0));

    V4d v0(-2, -2,  -18, -2);
    V4d v1( 2,  2,  -22,  2);

    V4d v2 (v1[0],v0[1],v0[2],v0[3]);
    V4d v3 (v1[0],v1[1],v0[2],v0[3]);
    V4d v4 (v0[0],v1[1],v0[2],v0[3]);
    V4d v5 (v0[0],v0[1],v1[2],v0[3]);
    V4d v6 (v1[0],v0[1],v1[2],v0[3]);
    V4d v7 (v1[0],v1[1],v1[2],v0[3]);
    V4d v8 (v0[0],v1[1],v1[2],v0[3]);

    V4d v9 (v0[0],v0[1],v0[2],v1[3]);
    V4d v10(v1[0],v0[1],v0[2],v1[3]);
    V4d v11(v1[0],v1[1],v0[2],v1[3]);
    V4d v12(v0[0],v1[1],v0[2],v1[3]);
    V4d v13(v0[0],v0[1],v1[2],v1[3]);
    V4d v14(v1[0],v0[1],v1[2],v1[3]);
    V4d v15(v0[0],v1[1],v1[2],v1[3]);

    double sz = 2.2;
    spheres.push_back(Sphere<4>(v0, sz, Color(1, 0, 1), Color(0), 0, 0));
    spheres.push_back(Sphere<4>(v1, sz, Color(1, 0, 1), Color(0), 0, 0));
    spheres.push_back(Sphere<4>(v2, sz, Color(1, 0, 1), Color(0), 0, 0));
    spheres.push_back(Sphere<4>(v3, sz, Color(1, 0, 1), Color(0), 0, 0));
    spheres.push_back(Sphere<4>(v4, sz, Color(1, 0, 1), Color(0), 0, 0));
    spheres.push_back(Sphere<4>(v5, sz, Color(1, 0, 1), Color(0), 0, 0));
    spheres.push_back(Sphere<4>(v6, sz, Color(1, 0, 1), Color(0), 0, 0));
    spheres.push_back(Sphere<4>(v7, sz, Color(1, 0, 1), Color(0), 0, 0));
    spheres.push_back(Sphere<4>(v8, sz, Color(1, 0, 1), Color(0), 0, 0));
    spheres.push_back(Sphere<4>(v9, sz, Color(1, 0, 1), Color(0), 0, 0));
    spheres.push_back(Sphere<4>(v10, sz, Color(1, 0, 1), Color(0), 0, 0));
    spheres.push_back(Sphere<4>(v11, sz, Color(1, 0, 1), Color(0), 0, 0));
    spheres.push_back(Sphere<4>(v12, sz, Color(1, 0, 1), Color(0), 0, 0));
    spheres.push_back(Sphere<4>(v13, sz, Color(1, 0, 1), Color(0), 0, 0));
    spheres.push_back(Sphere<4>(v14, sz, Color(1, 0, 1), Color(0), 0, 0));
    spheres.push_back(Sphere<4>(v15, sz, Color(1, 0, 1), Color(0), 0, 0));

    bmp::Image ren = render<4>(spheres);
    ren.write("test_render_hypercube.bmp");

}


void test_refraction(){
    std::vector<Sphere<3>> spheres;
    // position, radius, surface color, reflectivity, transparency, emission color
    spheres.push_back(Sphere<3>(V3d( 0.0, -10004, -20), 10000, Color(0.20, 0.20, 0.20), Color(0), 0, 0.0));
    spheres.push_back(Sphere<3>(V3d( 0.0,      0, -20),     4, Color(1.00, 0.32, 0.36), Color(0), 0, 0.5));
    spheres.push_back(Sphere<3>(V3d( 5.0,     -1, -15),     2, Color(0.90, 0.76, 0.46), Color(0), 0, 0.0));
    spheres.push_back(Sphere<3>(V3d( 5.0,      0, -25),     3, Color(0.65, 0.77, 0.97), Color(0), 0, 0.2));
    spheres.push_back(Sphere<3>(V3d(-5.5,      0, -15),     3, Color(0.90, 0.90, 0.90), Color(0), 1, 0.0));
    // light
    spheres.push_back(Sphere<3>(V3d( 0.0,     20, -20),     3, Color(0), Color(3), 0, 0));
    bmp::Image ren = render<3>(spheres);
    ren.write("test_render_refraction.bmp");
}

void test_refraction_4(){
    std::vector<Sphere<4>> spheres;
    // position, radius, surface color, reflectivity, transparency, emission color
    spheres.push_back(Sphere<4>(V4d( 0.0, -10004, -20, 0), 10000, Color(0.20, 0.20, 0.20), Color(0), 0, 0.0));
    spheres.push_back(Sphere<4>(V4d( 0.0,      0, -20, 0),     4, Color(1.00, 0.32, 0.36), Color(0), 1.5, 0));
    spheres.push_back(Sphere<4>(V4d( 5.0,     -1, -15, 0),     2, Color(0.90, 0.76, 0.46), Color(0), 0, 0.0));
    spheres.push_back(Sphere<4>(V4d( 5.0,      0, -25, 0),     3, Color(0.65, 0.77, 0.97), Color(0), 0, 0.2));
    spheres.push_back(Sphere<4>(V4d(-5.5,      0, -15, 0),     3, Color(0.90, 0.90, 0.90), Color(0), 0, 0.0));
    // light
    spheres.push_back(Sphere<4>(V4d( 0.0,     20, -20, 0),     3, Color(0), Color(3), 0, 0));
    bmp::Image ren = render<4>(spheres);

    ren.write("test_render_refraction_4.bmp");
}

int main()
{
    cout << "START RENDER" << endl;
    // renderer
    draw_animation();


    cout << "Hello world!" << endl;


//    UnitTest ut;
//    ut.test_vectors();
//    ut.test_mat();
//    ut.test_bmp();

    return 0;
}
