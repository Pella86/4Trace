#include <iostream>
#include <cmath>
#include <map>

#include <UnitTest.h>

#include "3D_render.h"

#include "vec.tpp"
#include "bmp.h"


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
        //bool inside = false;
        if(raydir.dot(nhit) > 0){
            nhit = -nhit;
            //inside = true;
        }

        if((sphere->transparency > 0 || sphere->reflection > 0) && depth < MAX_RAY_DEPTH){
            double facingratio = -raydir.dot(nhit);
            double fresneleffect = mix(pow(1 - facingratio, 3), 1, 0.1);

            Vector<double, dim> refldir = raydir - nhit * 2 * raydir.dot(nhit);
            refldir.normalize();
            Color reflection = trace(phit + nhit * bias, refldir, spheres, depth + 1);
            Color refraction(0);
            if(sphere->transparency > 0){
                // do nothing for now
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



class Glyphs{
private:

    bmp::Image base;
    const size_t dimx = 32;
    const size_t dimy = 32;
    const size_t rows = 8;
    const size_t cols = 16;

    std::vector<bmp::Image> images;

    bmp::Image cut_glyph(size_t icol, size_t irow);

public:
    Glyphs();

    bmp::Image get_char(char c);

    void imprint(bmp::Image& imp_image, char c, V2<size_t> position, double scale);

    void imprint(bmp::Image& imp_image, string str, V2<size_t> position, double scale);


};


Glyphs::Glyphs(){
    base = bmp::Image("./bmp_font/bmp_if_font_5.bmp");

    for(size_t irow = 0; irow < rows; irow++){
        for(size_t icol = 0; icol < cols; icol++){
            bmp::Image glyph;
            glyph = cut_glyph(icol, irow);

            images.push_back(glyph);
        }
    }
}

bmp::Image Glyphs::cut_glyph(size_t icol, size_t irow){

    bmp::Image glyph(dimx, dimy);

    for(size_t i = 0; i < dimx; i++){
        for(size_t j = 0; j < dimy; j++){

            bmp::Color px = base.pixelArray.get(i + dimx * icol, j + dimy * irow);

            glyph.pixelArray.set(i, j, px);
        }
    }

    return glyph;
}

bmp::Image Glyphs::get_char(char c){
    size_t pos = (int) c;
    if(pos < 96){pos -= 1;}
    return images[pos];
}

void Glyphs::imprint(bmp::Image& imp_image, char c, V2<size_t> position, double scale){

    bmp::Image character = get_char(c);
    bmp::Image scaled;

    size_t dimx_char = int(double(dimx) * scale);
    size_t dimy_char = int(double(dimy) * scale);

    if(scale != 1){


        // scale
        scaled = bmp::Image(dimx_char, dimy_char);

        for(size_t i = 0; i < dimx_char; i++){
            for(size_t j = 0; j < dimy_char; j++){
                size_t src_x = double(i) / double(dimx_char) * dimx;
                size_t src_y = double(j) / double(dimy_char) * dimy;

                src_x = min( (size_t) dimx - 1, src_x);
                src_y = min( (size_t) dimy - 1, src_y);

                bmp::Color px = character.pixelArray.get(src_x, src_y);
                scaled.pixelArray.set(i, j, px);
            }
        }
    }
    else{
        scaled = character;
    }



    for(size_t i = 0; i < dimx_char; i++){
        for(size_t j = 0; j < dimy_char; j++){
            bmp::Color px = scaled.pixelArray.get(i, j);
            size_t xpos = i + position.x();
            size_t ypos = j + position.y();

            bool check_border_x = (xpos < (size_t) imp_image.width() );
            bool check_border_y = (ypos < (size_t) imp_image.height());

            if(check_border_x && check_border_y ){
                imp_image.pixelArray.set(xpos, ypos, px);
            }
        }
    }
}

void Glyphs::imprint(bmp::Image& imp_image, string str, V2<size_t> position, double scale){

    size_t dimx_char = int(double(dimx) * scale);

    //size_t dimy_char = int(double(dimy) * scale);

    for(size_t i = 0; i < str.size(); i++){
        V2<size_t> char_pos = position;
        char_pos.x() = position.x() + i * dimx_char;
        imprint(imp_image, str[i], char_pos, scale);
    }
}

void draw_animation(){
    vector<Sphere<4>> spheres;



    for(int i = -10; i < 10; i ++){
        cout << "rendering frame: " << i + 10 << endl;
        // background sphere
        spheres.push_back(Sphere<4>(V4d(0, -10004, -20, 0),  10000, Color(0, 1, 1), Color(0), 0, 0));
        // light
        spheres.push_back(Sphere<4>(V4d(0,     20, 10, 0 ),     3, Color(0),       Color(3), 0, 0));

        spheres.push_back(Sphere<4>(V4d(i / 2.0,      0, -30, 0),      4, Color(1, 0, 0), Color(0), 0, 0));
        spheres.push_back(Sphere<4>(V4d(5,     -1, -15, 0),      2, Color(0, 0, 1), Color(0), 0, 0));

        // actual thing
        spheres.push_back(Sphere<4>(V4d(0, 0, -20,     i / 5.),      2, Color(1, 1, 0), Color(0), 0, 0));

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
    ren.write("test_render_refraction.bmp");
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
    ren.write("test_render_refraction_4.bmp");
}


int main()
{
    cout << "START RENDER" << endl;
    // renderer
    test_reflection();
    test_reflection_4();

    cout << "Hello world!" << endl;


//    UnitTest ut;
//    ut.test_vectors();
//    ut.test_mat();
//    ut.test_bmp();

    return 0;
}
