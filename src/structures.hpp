#include <chrono>
#include <iostream>
#include <vector>
#include <cmath>

struct Point {
    float x, y;
    float radius;
    float vx, vy;
    float ax, ay;
    float mass = 1.0f;
    float restitution = 0.8f;
    float friction = 0;
    bool fixed = false;
    float damping = 0.99f;
    bool dragged = false;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
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
            if (!p1.fixed) { p1.x += dx * 0.5f * diff; p1.y += dy * 0.5f * diff; }
            if (!p2.fixed) { p2.x -= dx * 0.5f * diff; p2.y -= dy * 0.5f * diff; }
        };
        enforceDistance(point1, point2, sideLength);
        enforceDistance(point2, point3, sideLength);
        enforceDistance(point3, point4, sideLength);
        enforceDistance(point4, point1, sideLength);
        float diagonal = sideLength * std::sqrt(2.0f);
        enforceDistance(point1, point3, diagonal);
        enforceDistance(point2, point4, diagonal);
    }
};

struct ParticleSystem {
    std::vector<Point> points;
    std::vector<Square> squares;

    void add(const Point& p) { points.push_back(p); }
    void addSquare(const Square& s) { squares.push_back(s); }

    void update(float dt) {
        const float baseGravity = 980.0f;
        for (auto& p : points) {
            if (p.fixed || p.dragged) continue;
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

        for (size_t i = 0; i < points.size(); ++i) {
            for (size_t j = i + 1; j < points.size(); ++j) {
                Point &p1 = points[i];
                Point &p2 = points[j];
                if (p1.fixed && p2.fixed) continue;
                float dx = p2.x - p1.x;
                float dy = p2.y - p1.y;
                float distSq = dx*dx + dy*dy;
                float minDist = p1.radius + p2.radius;
                if (distSq < minDist*minDist) {
                    float dist = std::sqrt(distSq);
                    float overlap = 0.5f * (minDist - dist);
                    if (!p1.fixed) { p1.x -= dx/dist * overlap; p1.y -= dy/dist * overlap; }
                    if (!p2.fixed) { p2.x += dx/dist * overlap; p2.y += dy/dist * overlap; }
                    float nx = dx / dist;
                    float ny = dy / dist;
                    float p = 2.0f * (p1.vx*nx + p1.vy*ny - p2.vx*nx - p2.vy*ny) / (p1.mass + p2.mass);
                    if (!p1.fixed) { p1.vx -= p * p2.mass * nx; p1.vy -= p * p2.mass * ny; }
                    if (!p2.fixed) { p2.vx += p * p1.mass * nx; p2.vy += p * p1.mass * ny; }
                }
            }
        }

        for (auto& s : squares) {
            for (auto* pt : {&s.point1, &s.point2, &s.point3, &s.point4}) {
                if (pt->fixed || pt->dragged) continue;
                pt->ay = baseGravity * (1.0f + (pt->radius - 1.0f) * 0.05f);
                pt->vx += pt->ax * dt;
                pt->vy += pt->ay * dt;
                pt->vx *= pt->damping;
                pt->vy *= pt->damping;
                pt->x += pt->vx * dt;
                pt->y += pt->vy * dt;
            }

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

            for (int i = 0; i < 10; ++i) s.enforceConstraints();

            for (auto& p : points) {
                if (p.fixed) continue;
                auto checkAndResolveCollision = [&](Point& p1, Point& p2) {
                    float edgeDx = p2.x - p1.x;
                    float edgeDy = p2.y - p1.y;
                    float pointDx = p.x - p1.x;
                    float pointDy = p.y - p1.y;
                    float edgeLengthSquared = edgeDx * edgeDx + edgeDy * edgeDy;
                    float projection = (pointDx * edgeDx + pointDy * edgeDy) / edgeLengthSquared;
                    projection = std::max(0.0f, std::min(1.0f, projection));
                    float closestX = p1.x + projection * edgeDx;
                    float closestY = p1.y + projection * edgeDy;
                    float distX = p.x - closestX;
                    float distY = p.y - closestY;
                    float distanceSquared = distX * distX + distY * distY;
                    if (distanceSquared < p.radius * p.radius) {
                        float distance = std::sqrt(distanceSquared);
                        float overlap = p.radius - distance;
                        if (distance > 0) { p.x += (distX / distance) * overlap; p.y += (distY / distance) * overlap; }
                        else { p.x += overlap; p.y += overlap; }
                        float normalX = distX / distance;
                        float normalY = distY / distance;
                        float dotProduct = p.vx * normalX + p.vy * normalY;
                        p.vx -= 2 * dotProduct * normalX;
                        p.vy -= 2 * dotProduct * normalY;
                        p.vx *= p.restitution;
                        p.vy *= p.restitution;
                    }
                };

                checkAndResolveCollision(s.point1, s.point2);
                checkAndResolveCollision(s.point2, s.point3);
                checkAndResolveCollision(s.point3, s.point4);
                checkAndResolveCollision(s.point4, s.point1);
            }
        }

        for (auto &s1 : squares) {
            for (auto &s2 : squares) {
                if (&s1 == &s2) continue;
                Point* pts1[4] = {&s1.point1, &s1.point2, &s1.point3, &s1.point4};
                Point* pts2[4] = {&s2.point1, &s2.point2, &s2.point3, &s2.point4};
                for (auto* p1 : pts1) {
                    for (auto* p2 : pts2) {
                        float dx = p2->x - p1->x;
                        float dy = p2->y - p1->y;
                        float distSq = dx*dx + dy*dy;
                        float minDist = p1->radius + p2->radius;
                        if (distSq < minDist*minDist) {
                            float dist = std::sqrt(distSq);
                            float overlap = 0.5f * (minDist - dist);
                            if (!p1->fixed) { p1->x -= dx/dist * overlap; p1->y -= dy/dist * overlap; }
                            if (!p2->fixed) { p2->x += dx/dist * overlap; p2->y += dy/dist * overlap; }
                            float nx = dx / dist;
                            float ny = dy / dist;
                            float p = 2.0f * (p1->vx*nx + p1->vy*ny - p2->vx*nx - p2->vy*ny) / (p1->mass + p2->mass);
                            if (!p1->fixed) { p1->vx -= p * p2->mass * nx; p1->vy -= p * p2->mass * ny; }
                            if (!p2->fixed) { p2->vx += p * p1->mass * nx; p2->vy += p * p1->mass * ny; }
                        }
                    }
                }
            }
        }
    }
};
