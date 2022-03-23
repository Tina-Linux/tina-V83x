#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

void setupChipmunk(cpSpace *space);
int proc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam);
void timer_callback(cpSpace *space);

#endif /* TEST_FRAMEWORK_H */
