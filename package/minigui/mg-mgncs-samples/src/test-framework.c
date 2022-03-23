#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <chipmunk/chipmunk.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "mgncs4touch/physics-animation/mspeedmeter.h"
#include "test-framework.h"

#define TIMER_MS 50

static SPEEDMETER s_speedmeter;
static cpSpace *s_space;

static void updateSeg(cpShape *shape, void *data) {
    HDC hdc = (HDC)data;
    cpVect a, b;
    int H = GetGDCapability(hdc, GDCAP_MAXY) + 1;

    a = cpSegmentShapeGetA(shape);
    b = cpSegmentShapeGetB(shape);

    SetPenColor(hdc, RGB2Pixel(hdc, 0x30, 0x30, 0x30));
    SetPenWidth(hdc, 2 * cpSegmentShapeGetRadius(shape));
    LineEx(hdc,
            shape->body->p.x + a.x, H - (shape->body->p.y + a.y),
            shape->body->p.x + b.x, H - (shape->body->p.y + b.y));
}

static void updatePolyShape(cpShape *shape, void *data) {
    HDC hdc = (HDC)data;
    int H = GetGDCapability(hdc, GDCAP_MAXY) + 1;
    int i;
    int count = cpPolyShapeGetNumVerts(shape);
    cpBody *body = shape->body;

    SetPenColor(hdc, RGB2Pixel(hdc, 0x00, 0xff, 0x00));
    SetPenWidth(hdc, 2);
    for (i=0; i<count; ++i) {
        cpVect v1 = cpPolyShapeGetVert(shape, i);
        cpVect v2 = cpPolyShapeGetVert(shape, (i+1)%count);
        LineEx(hdc,
                body->p.x + v1.x, H - (body->p.y + v1.y),
                body->p.x + v2.x, H - (body->p.y + v2.y));
    }
}

static void updateBall(cpShape *shape, void *data) {
    HDC hdc = (HDC)data;
    int H = GetGDCapability(hdc, GDCAP_MAXY) + 1;

    SetBrushColor(hdc, RGB2Pixel(hdc, 0xff, 0, 0));
    FillCircle(hdc, shape->body->p.x, H - shape->body->p.y, cpCircleShapeGetRadius(shape));
}

static void updateShapes(void *obj, void *data) {
    cpShape *shape = (cpShape *)obj;

    switch(shape->klass->type){
        case CP_CIRCLE_SHAPE:
            updateBall(shape, data);
            break;
        case CP_SEGMENT_SHAPE:
            updateSeg(shape, data);
            break;
        case CP_POLY_SHAPE:
            updatePolyShape(shape, data);
            break;
        default:
            assert(0);
            break;
    }
}

static void updateConstraint(cpConstraint *constraint, void *data) {
    HDC hdc = (HDC)data;
    const cpConstraintClass *klass = constraint->klass;
    int H = GetGDCapability(hdc, GDCAP_MAXY) + 1;

    cpBody *body_a = constraint->a;
    cpBody *body_b = constraint->b;

    if (klass == cpDampedSpringGetClass()) {
        cpDampedSpring *spring = (cpDampedSpring *)constraint;
        cpVect a = cpvadd(body_a->p, cpvrotate(spring->anchr1, body_a->rot));
        cpVect b = cpvadd(body_b->p, cpvrotate(spring->anchr2, body_b->rot));

        SetPenColor(hdc, RGB2Pixel(hdc, 0xa0, 0xa0, 0xa0));
        SetPenWidth(hdc, 5);
        LineEx(hdc, a.x, H-a.y,
                b.x, H-b.y);
    }
}

static void onTimer(HWND hWnd){
    cpSpaceStep(s_space, TIMER_MS / 1000.0f);

    timer_callback(s_space);

    RECT rc;
    HDC hdc;
    HDC clientdc = GetClientDC(hWnd);
    HDC memdc = CreateCompatibleDC(clientdc);

    hdc = memdc;

    GetClientRect(hWnd, &rc);

    SetBrushColor(hdc, RGBA2Pixel(hdc, 0x8f, 0x8f, 0x8f, 0xff));
    FillBox(hdc, rc.left, rc.top, RECTW(rc), RECTH(rc));

    cpSpaceHashEach(s_space->staticShapes, &updateShapes, (void *)hdc);
    cpSpaceHashEach(s_space->activeShapes, &updateShapes, (void *)hdc);

    cpArray *constraints = s_space->constraints;
    int i, count;
    for(i=0, count=constraints->num; i<count; i++){
        updateConstraint((cpConstraint *)constraints->arr[i], (void *)hdc);
    }

    BitBlt(memdc, 0, 0, 0, 0, clientdc, 0, 0, -1);
    DeleteMemDC(memdc);
    ReleaseDC(clientdc);
}

static int ChipmunkWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    SpeedMeterProc(hWnd, message, wParam, lParam);

    if (proc(hWnd, message, wParam, lParam) >= 0) {
        return 0;
    }

    switch (message) {
        case MSG_CREATE:
            SetTimer(hWnd, 1, 10);

            cpInitChipmunk();
            s_space = cpSpaceNew();
            setupChipmunk(s_space);

            s_speedmeter = mSpeedMeter_create(1000, 10);
            break;
        case MSG_LBUTTONDOWN:
            mSpeedMeter_reset(s_speedmeter);
            mSpeedMeter_append(s_speedmeter, LOSWORD(lParam), HISWORD(lParam), GetTickCount() * 10);
            break;
        case MSG_MOUSEMOVE:
            mSpeedMeter_append(s_speedmeter, LOSWORD(lParam), HISWORD(lParam), GetTickCount() * 10);
            break;
        case MSG_LBUTTONUP:
            mSpeedMeter_append(s_speedmeter, LOSWORD(lParam), HISWORD(lParam), GetTickCount() * 10);
            mSpeedMeter_stop(s_speedmeter);
            break;
        case MSG_TIMER:
            if (wParam == 1) {
                KillTimer(hWnd, 1);
                SetTimer(hWnd, 2, TIMER_MS / 10);
            }else{
                onTimer(hWnd);
                if (0){
                    HDC hdc = HDC_SCREEN;
                    SetBrushColor(hdc, RGBA2Pixel(hdc, 0x0f, 0x8f, 0x8f, 0xff));
                    printf("...\n");
                    // SetBrushColor(hdc, RGBA2Pixel(hdc, 0xff, 0, 0, 0xff));
                    FillBox(hdc, 0, 0, 1000, 1000);
                }
            }
            break;
            /*
        case MSG_PAINT:
            return 0;
            */
        case MSG_ERASEBKGND:
            return 0;
        default:
            break;
    }
    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

int MiniGUIMain (int argc, const char* argv[])
{
    MSG Msg;
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;

#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER , argv[0], 0 , 0);
#endif

    CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = argv[0];
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = ChipmunkWinProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = g_rcScr.right;
    CreateInfo.by = g_rcScr.bottom;
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = HWND_DESKTOP;

    hMainWnd = CreateMainWindow (&CreateInfo);

    if (hMainWnd == HWND_INVALID)
        return -1;

    ShowWindow(hMainWnd, SW_SHOWNORMAL);

    while (GetMessage(&Msg, hMainWnd)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    MainWindowThreadCleanup (hMainWnd);

    return 0;
}
