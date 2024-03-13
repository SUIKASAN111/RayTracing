#ifndef INTERVAL_H
#define INTERVAL_H

extern const double infinity;

class interval {
public:
    double min, max;

    interval() : min(+infinity), max(-infinity) {}

    interval(double _min, double _max) : min(_min), max(_max) {}

    interval(const interval& a, const interval& b)
        : min(fmin(a.min, b.min)), max(fmax(a.max, b.max)) {}

    bool contains(double x) const {
        return min <= x && x <= max;
    }

    bool surrounds(double x) const {
        return min < x && x < max;
    }

    double clamp(double x) const {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }

    double size() const {
        return max - min;
    }

    interval expand(double delta) {
        double padding = delta / 2;
        return interval(min - padding, max + padding);
    }

    static const interval empty, universe;
};

const interval interval::empty(+infinity, -infinity);
const interval interval::universe(-infinity, +infinity);

interval operator+(const interval& ival, double displacement) {
    return interval(ival.min + displacement, ival.max + displacement);
}

interval operator+(double displacement, const interval& ival) {
    return ival + displacement;
}

interval operator*(const interval& ival, double scale) {
    return interval(ival.min * scale, ival.max * scale);
}

interval operator*(double scale, const interval& ival) {
    return ival* scale;
}

#endif // INTERVAL_H