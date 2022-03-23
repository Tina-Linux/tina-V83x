#include "laundry_view.h"
#include "resource.h"

static void my_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {
    if (nc == STN_CLICKED) {
        switch (id) {
        case ID_LAUNDRY_VIEW_ONE:
        case ID_LAUNDRY_VIEW_TOW:
        case ID_LAUNDRY_VIEW_THR:
            PostMessage(GetParent(GetParent(GetParent(hwnd))), MSG_LAUNDRY_CLICK, id,
                    0);
            break;
        }
    }
}

static int LaundryViewProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam) {

    LaundryData *laundryData = NULL;
    laundryData = (LaundryData*) GetWindowAdditionalData(hwnd);

    switch (message) {
    case MSG_CREATE: {
        HWND laundryPartHwnd;

        if (laundryData->laundryPartData[0]) {
            laundryPartHwnd = CreateWindowEx(LAUNDRY_PART_VIEW, "",
            WS_CHILD | WS_VISIBLE | SS_NOTIFY,
            WS_EX_TRANSPARENT, ID_LAUNDRY_VIEW_ONE, 0, 0, 162, 100, hwnd,
                    (DWORD) laundryData->laundryPartData[0]);
            SetWindowFont(laundryPartHwnd, getLogFont(ID_FONT_SIMSUN_45));
            SetNotificationCallback(laundryPartHwnd, my_notif_proc);
            laundryData->laundryPartData[0]->laundryPartHwnd = laundryPartHwnd;
        }

        if (laundryData->laundryPartData[1]) {
            laundryPartHwnd = CreateWindowEx(LAUNDRY_PART_VIEW, "",
            WS_CHILD | WS_VISIBLE | SS_NOTIFY,
            WS_EX_TRANSPARENT, ID_LAUNDRY_VIEW_TOW, 162, 0, 162, 100, hwnd,
                    (DWORD) laundryData->laundryPartData[1]);
            SetWindowFont(laundryPartHwnd, getLogFont(ID_FONT_SIMSUN_45));
            SetNotificationCallback(laundryPartHwnd, my_notif_proc);
            laundryData->laundryPartData[1]->laundryPartHwnd = laundryPartHwnd;
        }

        if (laundryData->laundryPartData[2]) {
            laundryPartHwnd = CreateWindowEx(LAUNDRY_PART_VIEW, "",
            WS_CHILD | WS_VISIBLE | SS_NOTIFY,
            WS_EX_TRANSPARENT, ID_LAUNDRY_VIEW_THR, 324, 0, 162, 100, hwnd,
                    (DWORD) laundryData->laundryPartData[2]);
            SetWindowFont(laundryPartHwnd, getLogFont(ID_FONT_SIMSUN_45));
            SetNotificationCallback(laundryPartHwnd, my_notif_proc);
            laundryData->laundryPartData[2]->laundryPartHwnd = laundryPartHwnd;
        }

        return 0;
    }
    }
    return DefaultControlProc(hwnd, message, wParam, lParam);
}

BOOL RegisterLaundryView() {
    WNDCLASS MyClass;
    MyClass.spClassName = LAUNDRY_VIEW;
    MyClass.dwStyle = WS_NONE;
    MyClass.dwExStyle = WS_EX_NONE;
    MyClass.hCursor = 0;
    MyClass.iBkColor = PIXEL_lightgray;
    MyClass.WinProc = LaundryViewProc;

    return RegisterWindowClass(&MyClass);
}

void UnregisterLaundryView(void) {
    UnregisterWindowClass(LAUNDRY_VIEW);
}
