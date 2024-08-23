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

    inline bool operator==(Vector2 a) const {
        return (a.x == x) && (a.y == y);
    }
} Vector2;
typedef Vector2 Point2;

#endif //APOLLO_GENERAL_H
