#ifndef APOLLO_GENERAL_H
#define APOLLO_GENERAL_H

typedef struct Vector2 {
    double x;
    double y;

    void flip() {
        x *= -1;
        y *= -1;
    }

    inline Vector2 operator-(Vector2 a) const {
        return {x - a.x, y - a.y};
    }

    inline Vector2 operator+(Vector2 a) const {
        return {x + a.x, y + a.y};
    }

    inline Vector2 operator*(float a) const {
        return {a*x, a*y};
    }

    inline Vector2 operator*(Vector2 a) const {
        return {x * a.x, y * a.y};
    }

    inline bool operator==(Vector2 a) const {
        return (a.x == x) && (a.y == y);
    }

    inline bool operator!=(Vector2 a) const {
        return (a.x != x) || (a.y != y);
    }
} Vector2;
typedef Vector2 Point2;

typedef struct Matrix2 {
    Vector2 v1{};
    Vector2 v2{};

    Matrix2(double v1x, double v1y, double v2x, double v2y) {
        v1 = {v1x, v1y};
        v2 = {v2x, v2y};
    }

    Vector2 multiplyV2(Vector2 v) {
        return {v1.x * v.x + v2.x * v.y, v1.y * v.x + v2.y * v.y};
    }
} Matrix2;

#endif //APOLLO_GENERAL_H
