#include <chrono>
#include <iostream>
#include <vector>
#include <cmath>

struct Point {
    float x = 0, y = 0;        // position
    float radius = 1.0f;       // size
    float vx = 0, vy = 0;      // velocity
    float ax = 0, ay = 0;      // acceleration

    float mass = 1.0f;         // mass
    float restitution = 0.8f;  // bounciness
    float friction = 0.0f;     // friction on collisions
    bool fixed = false;         // if true, point does not move
    float damping = 0.99f;     // velocity damping
};

struct Square {
    Point point1, point2, point3, point4;
    float sideLength = 5.0f;

    void enforceConstraints() {
        auto enforceDistance = [](Point& p1, Point& p2, float targetDistance) {
            float dx = p2.x - p1.x;
            float dy = p2.y - p1.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist == 0.0f) return;
            float diff = (dist - targetDistance) / dist;

            if (!p1.fixed) {
                p1.x += dx * 0.5f * diff;
                p1.y += dy * 0.5f * diff;
            }
            if (!p2.fixed) {
                p2.x -= dx * 0.5f * diff;
                p2.y -= dy * 0.5f * diff;
            }
        };

        // side constraints
        enforceDistance(point1, point2, sideLength);
        enforceDistance(point2, point3, sideLength);
        enforceDistance(point3, point4, sideLength);
        enforceDistance(point4, point1, sideLength);

        // diagonal constraints
        float diagonal = sideLength * std::sqrt(2.0f);
        enforceDistance(point1, point3, diagonal);
        enforceDistance(point2, point4, diagonal);
    }
};

struct ParticleSystem {
    std::vector<Point> points;
    std::vector<Square> squares;

    void add(const Point& p) {
        points.push_back(p);
    }

    void addSquare(const Square& s) {
        squares.push_back(s);
    }

    void update(float dt) {
        const float baseGravity = 980.0f; // pixels per second^2

        // update free points
        for (auto& p : points) {
            if (p.fixed) continue;
            
            float gravity = baseGravity * (1.0f + (p.radius - 1.0f) * 0.05f);

            p.ay = gravity;

            p.vx += p.ax * dt;
            p.vy += p.ay * dt;

            p.vx *= p.damping;
            p.vy *= p.damping;

            p.x += p.vx * dt;
            p.y += p.vy * dt;

            if (p.y + p.radius > 720) {
                p.y = 720 - p.radius;
                p.vy *= -p.restitution;
                p.vx *= (1 - p.friction);

                if (std::abs(p.vy) < 0.1f) p.vy = 0;
                if (std::abs(p.vx) < 0.01f) p.vx = 0;
            }
        }

        // update squares
// update squares
        for (auto& s : squares) {
            // 1. apply gravity & move points
            for (auto* pt : {&s.point1, &s.point2, &s.point3, &s.point4}) {
                if (pt->fixed) continue;

                // subtle gravity scaling by radius
                pt->ay = baseGravity * (1.0f + (pt->radius - 1.0f) * 0.05f);

                pt->vx += pt->ax * dt;
                pt->vy += pt->ay * dt;

                pt->vx *= pt->damping;
                pt->vy *= pt->damping;

                pt->x += pt->vx * dt;
                pt->y += pt->vy * dt;
            }

            // 2. floor collision
            float floorY = 720.0f;
            float minY = std::min({s.point1.y, s.point2.y, s.point3.y, s.point4.y});
            if (minY + s.point1.radius > floorY) {
                float correction = floorY - (minY + s.point1.radius);
                for (auto* pt : {&s.point1, &s.point2, &s.point3, &s.point4}) {
                    if (pt->fixed) continue;
                    pt->y += correction;
                    pt->vy *= -pt->restitution;
                    pt->vx *= (1 - pt->friction);

                    if (std::abs(pt->vy) < 0.1f) pt->vy = 0;
                    if (std::abs(pt->vx) < 0.01f) pt->vx = 0;
                }
            }

            // 3. enforce constraints multiple times
            for (int i = 0; i < 10; ++i) {
                s.enforceConstraints();
            }
        }

        
    }
};
