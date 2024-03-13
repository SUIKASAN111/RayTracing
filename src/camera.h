#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"

#include "color.h"
#include "hittable.h"
#include "material.h"
#include "pdf.h"

#include <iostream>

class camera {
public:
    double aspect_ratio = 1.0; // Ratio of image width over height
    int image_width = 100; // Rendered image width in pixel count
    int samples_per_pixel = 10; // Count of random samples for each pixel
    int max_depth = 10; // Maximum number of ray bounces into scene
    color background; // Scene background color

    double vfov = 90; // Vertical view angle (field of view)
    point3 lookfrom = point3(0, 0, 0); // Point camera is looking from
    point3 lookat = point3(0, 0, -1); // Point camera is looking at
    vec3 vup = vec3(0, 1, 0); // Camera-relative "up" direction

    double defocus_angle = 0; // Variation angle of rays through each pixel
    double focus_dist = 10; // Distance from camera lookfrom point to plane of perfect focus

    void render(const hittable& world, const hittable& lights) {
        initialize();

        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
        int sqrt_spp = std::sqrt(samples_per_pixel);
        for (int j = 0; j < image_height; ++j){
            std::clog << "\rScanline remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; ++i) {
                color pixel_color(0, 0, 0);
                for (int s_j = 0; s_j < sqrt_spp; ++s_j) {
                    for (int s_i = 0; s_i < sqrt_spp; ++s_i) {
                        ray r = get_ray(i, j, s_i, s_j);
                        pixel_color += ray_color(r, max_depth, world, lights);
                    }
                }
                write_color(std::cout, pixel_color, samples_per_pixel);
            }
        }

        std::clog << "\rDone.                 \n";
    }

private:
    int image_height; // Rendered image height
    int sqrt_spp; // Square root of number of samples per pixel
    double recip_sqrt_spp; // 1 / sqrt_spp
    point3 center; // Camera center
    point3 pixel00_loc; // Location of pixel 0, 0
    vec3 pixel_delta_u; // Offset to pixel to the right
    vec3 pixel_delta_v; // Offset to pixel below
    vec3 right, up, forward; // Camera frame basis vectors
    vec3 defocus_disk_u; // Defocus disk horizontal radius
    vec3 defocus_disk_v; // Defocus disk vertical radius

    void initialize() {
        image_height = static_cast<int>(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        center = lookfrom;

        // Determine viewport dimensions.
        // double focal_length = (lookat - lookfrom).length();
        double theta = degrees_to_radians(vfov);
        double h = tan(theta / 2);
        double viewport_height = 2 * h * focus_dist;
        double viewport_width = viewport_height * (static_cast<double>(image_width) / image_height);

        sqrt_spp = static_cast<int>(sqrt(samples_per_pixel));
        recip_sqrt_spp = 1.0 / sqrt_spp;

        // Calculate the unit basis vectors for the camera coordinate frame.
        forward = unit_vector(lookat - lookfrom);
        right = unit_vector(cross(forward, vup));
        up = cross(right, forward);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        vec3 viewport_u = viewport_width * right;
        vec3 viewport_v = viewport_height * -up;

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the location of the upper left pixel.
        point3 viewport_upper_left = center + (focus_dist * forward) - viewport_u / 2 - viewport_v / 2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Calculate the camera defocus disk basis vectors.
        double defocus_radius = focus_dist * tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = right * defocus_radius;
        defocus_disk_v = up * defocus_radius;
    }

    ray get_ray(int i, int j, int s_i, int s_j) const {
        // Get a randomly sampled camera ray for the pixel at location i, j, originating from the camera defocus disk
        point3 pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
        // point3 pixel_sample = pixel_center + pixel_sample_square();
        point3 pixel_sample = pixel_center + pixel_sample_square(s_i, s_j);

        // point3 ray_origin = center;
        point3 ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        vec3 ray_direction = pixel_sample - ray_origin;
        double ray_time = random_double();
        
        return ray(ray_origin, ray_direction, ray_time);
    }

    // vec3 pixel_sample_square() const {
    //     // Returns a random point in the square surrounding a pixel at the origin.
    //     double px = -0.5 + random_double();
    //     double py = -0.5 + random_double();
    //     return (px * pixel_delta_u) + (py * pixel_delta_v);
    // }

    vec3 pixel_sample_square(int s_i, int s_j) const {
        // Returns a random point in the square surrounding a pixel at the origin, given the two subpixel indices.
        double px = -0.5 + recip_sqrt_spp * (s_i + random_double());
        double py = -0.5 + recip_sqrt_spp * (s_j + random_double());
        return (px * pixel_delta_u) + (py * pixel_delta_v);
    }

    point3 defocus_disk_sample() const {
        // Returns a random point in the camera defocus disk.
        vec3 p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    color ray_color(const ray& r, int depth, const hittable& world, const hittable& lights) const {
        hit_record rec;

        if (depth <= 0)
            return color(0, 0, 0);

        if (!world.hit(r, interval(0.001, infinity), rec))
            return background;

        scatter_record srec;
        color color_from_emission = rec.mat->emitted(r, rec, rec.u, rec.v, rec.p);

        if (!rec.mat->scatter(r, rec, srec))
            return color_from_emission;

        if (srec.skip_pdf)
            return srec.attenuation * ray_color(srec.skip_pdf_ray, depth - 1, world, lights);

        auto light_ptr = std::make_shared<hittable_pdf>(lights, rec.p);
        mixture_pdf mixed_pdf(light_ptr, srec.pdf_ptr);

        ray scattered = ray(rec.p, mixed_pdf.generate(), r.time());
        double pdf_val = mixed_pdf.value(scattered.direction());

        double scattering_pdf = rec.mat->scattering_pdf(r, rec, scattered);

        color sample_color = ray_color(scattered, depth - 1, world, lights);
        color color_from_scatter = (srec.attenuation * scattering_pdf * sample_color) / pdf_val;

        return color_from_emission + color_from_scatter;
    }
};

#endif // CAMERA_H