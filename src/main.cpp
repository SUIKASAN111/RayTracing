#include "rtweekend.h"

void cornellbox_bunny();
void multi_light();


int main() {
    // cornellbox_bunny();
    multi_light();
}


void cornellbox_bunny() {
    hittable_list world;

    auto gray = std::make_shared<lambertian>(color(0.5, 0.5,0.5));
    auto red = std::make_shared<lambertian>(color(0.65, 0.05, 0.05));
    auto white = std::make_shared<lambertian>(color(0.73, 0.73, 0.73));
    auto green = std::make_shared<lambertian>(color(0.12, 0.45, 0.15));
    auto light = std::make_shared<diffuse_light>(color(15, 15, 15));

    // model
    model model("../../resources/models/bunny/bunny.obj", 1000, gray);
    world = model.getHittableList(vec3(0, 0, 0), vec3(400, -30, 180));

    // Cornell box sides
    world.add(std::make_shared<quad>(point3(555,0,0), vec3(0,0,555), vec3(0,555,0), green));
    world.add(std::make_shared<quad>(point3(0,0,555), vec3(0,0,-555), vec3(0,555,0), red));
    world.add(std::make_shared<quad>(point3(0,555,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(std::make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,0,-555), white));
    world.add(std::make_shared<quad>(point3(555,0,555), vec3(-555,0,0), vec3(0,555,0), white));

    // light
    world.add(std::make_shared<quad>(point3(213, 554, 227), vec3(130, 0, 0), vec3(0, 0, 105), light));

    // tall box
    std::shared_ptr<material> aluminum = std::make_shared<metal>(color(0.8, 0.85, 0.88), 0.0);
    std::shared_ptr<hittable> box1 = box(point3(0, 0, 0), point3(165, 330, 165), aluminum);
    box1 = std::make_shared<rotate_y>(box1, 15);
    box1 = std::make_shared<translate>(box1, vec3(265,0,295));
    world.add(box1);

    // sphere
    auto glass = std::make_shared<dielectric>(1.5);
    world.add(std::make_shared<sphere>(point3(190, 90, 190), 90, glass));

    // importance sample objects
    hittable_list lights;
    auto m = std::shared_ptr<material>();
    lights.add(std::make_shared<quad>(point3(343, 554, 332), vec3(-130, 0, 0), vec3(0, 0, -105), m));
    lights.add(std::make_shared<sphere>(point3(190, 90, 190), 90, m));

    // build bvh tree
    world = hittable_list(std::make_shared<bvh_node>(world));
        
    camera cam;

    cam.aspect_ratio = 1.0;
    cam.image_width = 600;
    cam.samples_per_pixel = 2000;
    cam.max_depth = 50;
    cam.background = color(0, 0, 0);

    cam.vfov = 40;
    cam.lookfrom = point3(278, 278, -800);
    cam.lookat = point3(278, 278, 0);
    cam.vup = vec3(0, 1, 0);

    cam.defocus_angle = 0;

    cam.render(world, lights);
}

void multi_light() {
    hittable_list world;

    auto gray = std::make_shared<lambertian>(color(0.5, 0.5,0.5));
    auto orange = std::make_shared<diffuse_light>(color(1, 0.67, 0.26));
    auto purple = std::make_shared<diffuse_light>(color(0.36, 0.3, 0.43));
    auto red = std::make_shared<lambertian>(color(.65, .05, .05));
    auto white = std::make_shared<lambertian>(color(.73, .73, .73));
    auto green = std::make_shared<lambertian>(color(.12, .45, .15));
    auto light_white = std::make_shared<diffuse_light>(color(15, 15, 15));
    auto light_white_weak = std::make_shared<diffuse_light>(color(5, 5, 5));


    // models
    // model model_backpack("../../resources/models/backpack/backpack.obj", 65, orange);
    model model_suzanee("../../resources/models/suzanne/suzanne.obj", 120, gray);
    // world = model_backpack.getHittableList(vec3(0, 0, 0), vec3(360, 200, 200));
    world = model_suzanee.getHittableList(vec3(0, -30, 0), vec3(250, 130, 350));
    // world.add(model_suzanee.getHittableList(vec3(0, -30, 0), vec3(175, 150, 300)));
    
    // Cornell box sides
    world.add(std::make_shared<quad>(point3(555,0,0), vec3(0,0,555), vec3(0,555,0), green));
    world.add(std::make_shared<quad>(point3(0,0,555), vec3(0,0,-555), vec3(0,555,0), red));
    world.add(std::make_shared<quad>(point3(0,555,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(std::make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,0,-555), white));
    world.add(std::make_shared<quad>(point3(555,0,555), vec3(-555,0,0), vec3(0,555,0), white));


    // Light
    world.add(std::make_shared<quad>(point3(213, 554, 227), vec3(130, 0, 0), vec3(0, 0, 105), light_white));
    // world.add(std::make_shared<quad>(point3(554, 213, 227), vec3(0, 0, 105), vec3(0, 130,0), light_white));
    world.add(std::make_shared<quad>(point3(1, 213, 330), vec3(0, 0, -105), vec3(0, 130,0), light_white));

    // Light Sources
    hittable_list lights;
    auto m = std::shared_ptr<material>();
    lights.add(std::make_shared<sphere>(vec3(100, 350, 510), 25, m));
    
    world = hittable_list(std::make_shared<bvh_node>(world));

    camera cam;

    cam.aspect_ratio = 1.0;
    cam.image_width = 600;
    cam.samples_per_pixel = 2000;
    cam.max_depth = 50;
    cam.background = color(0, 0, 0);

    cam.vfov     = 40;
    cam.lookfrom = point3(278, 278, -800);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0, 1, 0);

    cam.defocus_angle = 0;

    cam.render(world, lights);
}