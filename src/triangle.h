#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <vector>

#include "rtweekend.h"
#include "hittable.h"
#include "material.h"

class triangle : public hittable {
public:
    triangle(point3 _v0, point3 _v1, point3 _v2, std::shared_ptr<material> m)
        : v0(_v0), v1(_v1), v2(_v2), mat(m)
    {
        vec3 e1 = v1 - v0;
        vec3 e2 = v2 - v0;
        area = cross(e1, e2).length() * 0.5;
        normal = unit_vector(cross(e1, e2));

        set_bounding_box();
    }

    void set_bounding_box() {
        interval ix(fmin(fmin(v0[0],v1[0]),v2[0]),fmax(fmax(v0[0],v1[0]),v2[0]));
        interval iy(fmin(fmin(v0[1],v1[1]),v2[1]),fmax(fmax(v0[1],v1[1]),v2[1]));
        interval iz(fmin(fmin(v0[2],v1[2]),v2[2]),fmax(fmax(v0[2],v1[2]),v2[2]));
        bbox = aabb(ix, iy, iz);
    }

    virtual aabb bounding_box() const override { return bbox; }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        //Möller–Trumbore intersection algorithm
        const float EPSILON = 0.0000001;
        vec3 e1, e2, pvec, tvec, qvec;
        double det, inv_det, u, v;
        e1 = v1 - v0;
        e2 = v2 - v0;
        pvec = cross(r.direction(), e2);
        det = dot(e1, pvec);
        inv_det = 1 / det;
        if (fabs(det) < EPSILON) return false;
        tvec = r.origin() - v0;
        u = inv_det * dot(tvec, pvec);
        if (u < 0.0 || u > 1) return false;
        qvec = cross(tvec, e1);
        v = inv_det * dot(r.direction(), qvec);
        if (v < 0.0 || v + u > 1) return false;

        double t = inv_det * dot(e2, qvec);

        if (!ray_t.contains(t))return false;

        rec.set_face_normal(r, normal);
        rec.t = t;
        rec.p = r.at(t);
        rec.mat = mat;
        rec.u = u;
        rec.v = v;
        triangle_uv(rec.p, rec.u, rec.v);
        return true;

    }

    double pdf_value(const point3& origin, const vec3& v) const override {
        hit_record rec;
        if (!this->hit(ray(origin, v), interval(0.001, infinity), rec))
            return 0;

        double distance_squared = rec.t * rec.t * v.length_squared();
        double cosine = fabs(dot(v, rec.normal) / v.length());

        return distance_squared / (cosine * area);
    }

    virtual vec3 random(const vec3& origin) const override {
        double r1 = random_double();
        double r2 = random_double();
        point3 random_point = v0 + r1 * (v1 - v0) + r2 * (v2 - v1);
        return random_point - origin;
    }

private:
    point3 v0, v1, v2;
    vec3 normal;
    double area;
    std::shared_ptr<material> mat;
    aabb bbox;
    vec3 auv = vec3(0, 0, -1), buv = vec3(0, 1, -1), cuv = vec3(1, 0, -1);

    void triangle_uv(const vec3& p, double& u, double& v) const {
        vec3 d(u * auv + v * buv + (1 - u - v) * cuv);
        u = d[0];
        v = d[1];
    }
};

#endif // TRIANGLE_H