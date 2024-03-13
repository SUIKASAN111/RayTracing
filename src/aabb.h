#ifndef AABB_H
#define AABB_H

#include "rtweekend.h"

class aabb {
public:
    interval x, y, z;

    aabb() {} // the default AABB is empty, since intervals are empty by default.

    aabb(const interval& ix, const interval& iy, const interval& iz)
        : x(ix), y(iy), z(iz)
    {
        pad_to_minimums();
    }

    aabb(const point3& a, const point3& b) {
        // Treat the two points a and b as extrema for the bounding box
        x = interval(fmin(a[0], b[0]), fmax(a[0], b[0]));
        y = interval(fmin(a[1], b[1]), fmax(a[1], b[1]));
        z = interval(fmin(a[2], b[2]), fmax(a[2], b[2]));
        pad_to_minimums();
    }

    aabb(const aabb& box1, const aabb& box2) {
        x = interval(box1.x, box2.x);
        y = interval(box1.y, box2.y);
        z = interval(box1.z, box2.z);
    }
    
    const interval& axis(int n) const {
        if (n == 1) return y;
        if (n == 2) return z;
        return x;
    }

    bool hit(const ray& r, interval ray_t) const {
        for (int i = 0; i < 3; ++i) {
            double invD = 1 / r.direction()[i];
            double orig = r.origin()[i];

            double t0 = (axis(i).min - orig) * invD;
            double t1 = (axis(i).max - orig) * invD;

            if (invD < 0)
                std::swap(t0, t1);

            if (t0 > ray_t.min) ray_t.min = t0;
            if (t1 < ray_t.max) ray_t.max = t1;

            if (ray_t.max <= ray_t.min)
                return false;
        }
        return true;
    }

     int longest_axis() const {
        // Returns the index of the longest axis of the bounding box.
        if (x.size() > y.size())
            return x.size() > z.size() ? 0 : 2;
        else
            return y.size() > z.size() ? 1 : 2;
    }

    static const aabb empty, universe;

private:
    void pad_to_minimums() {
        // Adjust the AABB so that no side is narrower than some delta, padding if necessary.

        double delta = 0.0001;
        if (x.size() < delta) x = x.expand(delta);
        if (y.size() < delta) y = y.expand(delta);
        if (z.size() < delta) z = z.expand(delta);
    }
};

const aabb aabb::empty    = aabb(interval::empty,    interval::empty,    interval::empty);
const aabb aabb::universe = aabb(interval::universe, interval::universe, interval::universe);

aabb operator+(const aabb& bbox, const vec3& offset) {
    return aabb(bbox.x + offset.x(), bbox.y + offset.y(), bbox.z + offset.z());
}

aabb operator+(const vec3& offset, const aabb& bbox) {
    return bbox + offset;
}

aabb operator*(const aabb& bbox, double scale) {
    return aabb(bbox.x * scale, bbox.y * scale, bbox.z * scale);
}

aabb operator*(double scale, const aabb& bbox) {
    return bbox * scale;
}

#endif // AABB_H