#include <stdlib.h>

#include "chipmunk-utils.h"

void create_baffle_board(cpSpace *space, int y_baseline, int y_height, int x1, int x2, float stiffness, float damping) {
    cpBody *baffleBody1, *baffleBody2;
    cpShape *baffleShape;
    cpConstraint *spring;
    int r;
    int x_ball;
    int y_ball;
    int w_seg = 5;

    r = y_height / 2;
    x_ball = x2 > x1 ? x1 + r : x1 - r;
    y_ball = y_baseline + r;

    /* Ball */
    baffleBody1 = cpBodyNew(1, INFINITY);
    baffleBody1->p = cpv(x_ball, y_ball);
    cpSpaceAddBody(space, baffleBody1);

    baffleShape = cpCircleShapeNew(baffleBody1, r, cpvzero);
    baffleShape->e = 0; baffleShape->u = 0; baffleShape->collision_type = 1;
    cpSpaceAddShape(space, baffleShape);

    /* Seg */
    baffleBody2 = cpBodyNew(INFINITY, INFINITY);
    baffleBody2->p = cpv(x2, y_ball);

    baffleShape = cpSegmentShapeNew(baffleBody2,
            cpv(x1 < x2 ? -w_seg : w_seg, -r), cpv(x1 < x2 ? -w_seg : w_seg, r), w_seg);
    baffleShape->e = 0; baffleShape->u = 0; baffleShape->collision_type = 1;
    cpSpaceAddStaticShape(space, baffleShape);

    /* Spring */
    spring = cpDampedSpringNew(baffleBody1, baffleBody2,
            x1 < x2 ? cpv(-r, 0) : cpv(r, 0), cpvzero,
            abs(x2 - x1), stiffness, damping);
    cpSpaceAddConstraint(space, spring);
}

cpShape *create_floor(cpSpace *space, int y_baseline, int x1, int x2) {
    cpBody *floorBody;
    cpShape *floorShape;

    floorBody = cpBodyNew(INFINITY, INFINITY);
    floorBody->p = cpv(x1, y_baseline);

    floorShape = cpSegmentShapeNew(floorBody, cpv(0, -10.0f), cpv(x2-x1, -10.0f), 10.0f);
    cpSpaceAddStaticShape(space, floorShape);

    return floorShape;
}

cpShape *create_ball(cpSpace *space, int x, int y, int r, int m) {
    cpBody *ballBody;
    cpShape *ballShape;

    ballBody = cpBodyNew(m, INFINITY);
    ballBody->p = cpv(x, y);
    cpSpaceAddBody(space, ballBody);

    ballShape = cpCircleShapeNew(ballBody, r, cpvzero);
    cpSpaceAddShape(space, ballShape);

    return ballShape;
}
