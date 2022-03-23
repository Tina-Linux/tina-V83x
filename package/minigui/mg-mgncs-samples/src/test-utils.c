#include <stdio.h>
#include <string.h>
#include <sys/times.h>

#include <chipmunk/chipmunk.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "mgncs4touch/physics-animation/mspeedmeter.h"
#include "test-framework.h"
#include "chipmunk-utils.h"

cpBody *g_ballBody;
clock_t time_start = 0, time_end;
float pos_start, pos_end;
int g_x1 = 200, g_x2 = 550;
int R = 20;
int status;
int button_down;

void setupChipmunk(cpSpace *space) {
    cpShape *shape;
    int y_baseline = 150;

    space->gravity = cpv(0, -200);

    shape = create_floor(space, y_baseline, -100, g_x2 + 300);
    shape->u = 1;

    shape = create_floor(space, y_baseline+2*R+2*10, -100, g_x2 + 300);
    shape->u = 0;

    shape = create_block(space, g_x1, y_baseline, g_x1+100, y_baseline + 2*R, 10);
    shape->u = 1;
    g_ballBody = shape->body;

    create_baffle_board(space, y_baseline, 2*R, g_x2, g_x2+150, 200, 2);

    create_baffle_board(space, y_baseline, 2*R, g_x1, g_x1-150, 200, 2);
}

int proc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam){
    switch (message) {
        /*
        case MSG_LBUTTONDOWN:
            cpBodySetVel(g_ballBody, cpv(800, 0));
            time_start = times(NULL);
            pos_start = g_ballBody->p.x;
            break;
        case MSG_RBUTTONDOWN:
            cpBodySetVel(g_ballBody, cpv(-800, 0));
            time_start = times(NULL);
            pos_start = g_ballBody->p.x;
            break;
            */
        case MSG_LBUTTONDOWN:
            {
                button_down = 1;
            }
            break;
        case MSG_LBUTTONUP:
            {
                float v_x, v_y;
                QueryMouseMoveVelocity(&v_x, &v_y);
                printf("V=%.2f,%.2f\n", v_x, v_y);
                button_down = 0;

                if (v_x > 800){
                    v_x = 800;
                }else if (v_x < -800){
                    v_x = -800;
                }
                cpBodySetVel(g_ballBody, cpv(v_x, 0));
                time_start = times(NULL);
                pos_start = g_ballBody->p.x;
            }
        default:
            break;
    }
    return -1;
}

void timer_callback(cpSpace *space){
#if 0
    float v = cpBodyGetVel(g_ballBody).x;
    float p = cpBodyGetPos(g_ballBody).x;
    switch (status) {
        case 0:
            if (v > 0 && p > g_x2) {
                status = 1;
            }else if (v < 0 && p < g_x1) {
                status = -1;
            }
            break;
        case -1:
            if (v > 0 && p >= g_x1) {
                cpBodySleep(g_ballBody);
                g_ballBody->p.x = g_x1;
                status = 0;
            }
            break;
        case 1:
            if (v < 0 && p <= g_x2) {
                cpBodySleep(g_ballBody);
                g_ballBody->p.x = g_x2;
                status = 0;
            }
            break;
        default:
            break;
    }
#endif

    if (time_start) {
        time_end = times(NULL);
        if (time_end - time_start > 200) {
            pos_end = g_ballBody->p.x;
            printf("t=%.2f v=%.2f\n", (time_end - time_start) / 100.0f, (pos_end - pos_start) / (time_end - time_start) * 100.0f);
            time_start = 0;
        }
    }
}
