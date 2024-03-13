#ifndef HITTABLE_H
#define HITTABLE_H

#include "rtweekend.h"

#include "aabb.h"

class material;

class hit_record {
public:
    point3 p;
    vec3 normal;
    std::shared_ptr<material> mat;
    double t;
    double u;
    double v;
    bool front_face;

    void set_face_normal(const ray& r, const vec3& outward_normal) {
        // Sets the hit record normal vector.
        // NOTE: the parameter `outward_normal` is assumed to have unit length.

        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable {
public:
    virtual ~hittable() = default;

    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;

    virtual aabb bounding_box() const = 0;

    virtual double pdf_value(const point3& origin, const vec3& v) const {
        return 0.0;
    }

    virtual vec3 random(const vec3& origin) const {
        return vec3(1, 0, 0);
    }
};

class translate : public hittable {
public:
    translate(std::shared_ptr<hittable> p, const vec3& displacement)
        : object(p), offset(displacement)
    {
        bbox = object->bounding_box() + offset;
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // Move the ray backwards by the offset
        ray offset_r(r.origin() - offset, r.direction(), r.time());

        // Determine where (if ant) an intersection occurs along the offset ray
        if (!object->hit(offset_r, ray_t, rec))
            return false;

        // Move the intersection point forwards by the offset
        rec.p += offset;

        return true;
    }

    aabb bounding_box() const override { return bbox; }

private:
    std::shared_ptr<hittable> object;
    vec3 offset;
    aabb bbox;
};

// class scale : public hittable {
// public:
//     scale(std::shared_ptr<hittable> p, double _scale)
//         : object(p), scale_val(_scale)
//     {
//         bbox = object->bounding_box() * scale_val;
//     }

//     bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
//         point3 scaled_origin = r.origin() / scale_val;
//         point3 scaled_dest = r.at(1) / scale_val;
//         vec3 scaled_direction = unit_vector(scaled_dest - scaled_origin);
//         ray scaled_r(scaled_origin, scaled_direction);

//         if (!object->hit(scaled_r, ray_t, rec))
//             return false;

//         double scaled_distance = (rec.p - scaled_origin).length();
//         rec.p *= scale_val;
//         double distance = (rec.p - r.origin()).length();
//         rec.t = rec.t * distance / scaled_distance;

//         if (!ray_t.contains(rec.t))
//             return false;

//         return true;
//     }

//     aabb bounding_box() const override { return bbox; }

// private:
//     std::shared_ptr<hittable> object;
//     double scale_val;
//     aabb bbox;
// };

class rotate_y : public hittable {
public:
    rotate_y(std::shared_ptr<hittable> p, double angle) : object(p) {
        double radians = degrees_to_radians(angle);
        sin_theta = sin(radians);
        cos_theta = cos(radians);
        bbox = object->bounding_box();

        point3 min(infinity, infinity, infinity);
        point3 max(-infinity, -infinity, -infinity);

        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                for (int k = 0; k < 2; ++k) {
                    double x = i * bbox.x.max + (1 - i) * bbox.x.min;
                    double y = j * bbox.y.max + (1 - j) * bbox.y.min;
                    double z = k * bbox.z.max + (1 - k) * bbox.z.min;

                    double newx = cos_theta * x + sin_theta * z;
                    double newz = -sin_theta * x + cos_theta * z;

                    vec3 tester(newx, y, newz);

                    for (int c = 0; c < 3; ++c) {
                        min[c] = fmin(min[c], tester[c]);
                        max[c] = fmax(max[c], tester[c]);
                    }
                }
            }
        }

        bbox = aabb(min, max);
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // Change the ray from world space to object space
        point3 origin = r.origin();
        vec3 direction = r.direction();

        origin[0] = cos_theta * r.origin()[0] - sin_theta * r.origin()[2];
        origin[2] = sin_theta * r.origin()[0] + cos_theta * r.origin()[2];

        direction[0] = cos_theta * r.direction()[0] - sin_theta * r.direction()[2];
        direction[2] = sin_theta * r.direction()[0] + cos_theta * r.direction()[2];

        ray rotated_r(origin, direction, r.time());

        // Determine where (if any) an intersection occurs in object space
        if (!object->hit(rotated_r, ray_t, rec))
            return false;

        // Change the intersection point from object space to world space
        auto p = rec.p;
        p[0] = cos_theta * rec.p[0] + sin_theta * rec.p[2];
        p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];

        // Change the normal from object space to world space
        auto normal = rec.normal;
        normal[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
        normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];

        rec.p = p;
        rec.normal = normal;

        return true;
    }

    aabb bounding_box() const override { return bbox; }
    
private:
    std::shared_ptr<hittable> object;
    double sin_theta;
    double cos_theta;
    aabb bbox;
};

class rotate_ : public hittable {
public:
    rotate_(std::shared_ptr<hittable> p, double ax, double ay, double az) : ptr(p){
        ax = degrees_to_radians(ax); ay = degrees_to_radians(ay); az = degrees_to_radians(az);
        sinx = sin(ax); siny = sin(ay); sinz = sin(az);
        cosx = cos(ax); cosy = cos(ay); cosz = cos(az);
        bbox = ptr->bounding_box();
        vec3 min(+infinity), max(-infinity);
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                for (int k = 0; k < 2; k++) {
                    auto x = i * bbox.x.max + (1 - i) * bbox.x.min;
                    auto y = j * bbox.y.max + (1 - j) * bbox.y.min;
                    auto z = k * bbox.z.max + (1 - k) * bbox.z.min;

                    auto tester = transform(x, y, z);

                    for (int c = 0; c < 3; c++) {
                        min[c] = fmin(min[c], tester[c]);
                        max[c] = fmax(max[c], tester[c]);
                    }
                }
            }
        }
        bbox = aabb(min, max);
    }
    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        auto origin = r.origin();
        auto direction = r.direction();
        origin = invtransform(origin);
        direction = invtransform(direction);

        ray rotated_r(origin, direction, r.time());

        if (!ptr->hit(rotated_r, ray_t, rec))
            return false;

        auto p = rec.p;
        auto normal = rec.normal;

        p = transform(p);
        normal = transform(normal);

        rec.p = p;
        rec.set_face_normal(rotated_r, normal);

        return true;
    }

    virtual aabb bounding_box() const override {
        return bbox;
    }

public:
    std::shared_ptr<hittable> ptr;
    double sinx, cosx, siny, cosy, sinz, cosz;
    aabb bbox;
private:
    vec3 transform(double x, double y, double z) const {
        double x1 = x;
        double y1 = cosx * y + sinx * z;
        double z1 = -sinx * y + cosx * z;

        x = cosy * x1 + siny * z1;
        y = y1;
        z = -siny * x1 + cosy * z1;

        x1 = cosz * x + sinz * y;
        y1 = -sinz * x + cosz * y;
        z1 = z;

        return vec3(x1, y1, z1);
    }
    vec3 transform(vec3& v) const {
        return transform(v.x(), v.y(), v.z());
    }
    vec3 invtransform(double x, double y, double z) const {
        double x1 = x;
        double y1 = cosx * y - sinx * z;
        double z1 = sinx * y + cosx * z;

        x = cosy * x1 - siny * z1;
        y = y1;
        z = siny * x1 + cosy * z1;

        x1 = cosz * x - sinz * y;
        y1 = sinz * x + cosz * y;
        z1 = z;

        return vec3(x1, y1, z1);
    }
    vec3 invtransform(vec3& v) const {
        return invtransform(v.x(), v.y(), v.z());
    }
};

#endif // HITTABLE_H