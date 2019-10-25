#include "3D_render.h"

#include <vector>

using namespace std;

#include "vec.tpp"
#include "bmp.h"


namespace ren3d{

    using Color = V3d;

    constexpr double MAX_RAY_DEPTH = 5;


    struct Sphere{

        V3d center;
        double radius, radius2;
        Color surface, emission;
        double transparency, reflection;

        Sphere(const V3d& center,
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

        bool intersect(const V3d& rayorig, const V3d& raydir, double& t0, double& t1) const{
            V3d l = center - rayorig;
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


    double radians(double deg){
        return deg * M_PI / 180;
    }

    Color trace(const V3d& rayorig, const V3d& raydir, const vector<Sphere>& spheres, const int& depth) {

        double tnear = INFINITY;
        const Sphere* sphere = NULL;

        for(size_t i = 0; i < spheres.size(); i++ ){
            double t0 = INFINITY, t1 = INFINITY;
            if(spheres[i].intersect(rayorig, raydir, t0, t1)){
                if(t0 < 0) t0 = t1;
                if(t0 < tnear){
                    tnear = t0;
                    sphere = &spheres[i];
                }
            }
        }

        if(!sphere) {
            //cout << "No match" << endl;
            return Color(0, 0.2, 0.2);
        }
        else{
            Color surfaceColor(0);
            V3d phit = rayorig + raydir * tnear;
            V3d nhit = phit - sphere->center;
            nhit.normalize();
            double bias = 1e-4;

            //bool inside = false;
            if(raydir.dot(nhit) > 0){
                nhit = -nhit;
                //inside = true;
            }

            if((sphere->transparency > 0 || sphere->reflection > 0) && depth < MAX_RAY_DEPTH){

            }
            else{

                for(size_t i = 0; i < spheres.size(); i++){
                    if( !(spheres[i].emission == Color(0))){

                        Color transmission(1);
                        V3d light_direction = spheres[i].center - phit;
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

                        surfaceColor += sphere->surface * transmission * max(double(0), nhit.dot(light_direction)) * spheres[i].emission;
                    }

                }
            }
            return surfaceColor + sphere->emission;
        }

    };

    void render(vector<Sphere> spheres){
        unsigned width = 640, height = 480;
        //unsigned width = 16, height = 12;
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
                pixel.x(min(1., pixel.x()));
                pixel.y(min(1., pixel.y()));
                pixel.z(min(1., pixel.z()));

                bmp::Color bmppix(pixel * 255);

                img.pixelArray.set(i, height-j, bmppix);
            }
        }

        img.write("test_render.bmp");
    }


    void Render3D::render_fct()
    {
        vector<Sphere> spheres;

        spheres.push_back(Sphere(V3d(0, -10004, -20),  10000, Color(0, 1, 0), Color(0), 0, 0));
        spheres.push_back(Sphere(V3d(0, 0,-20),            4, Color(1, 0, 0), Color(0), 0, 0));
        spheres.push_back(Sphere(V3d(5, -1, -15),          2, Color(0, 0, 1), Color(0), 0, 0));
        spheres.push_back(Sphere(V3d(0, 20, -30),          3, Color(0),       Color(3), 0, 0));

        ren3d::render(spheres);
    }

} // end ren3d
