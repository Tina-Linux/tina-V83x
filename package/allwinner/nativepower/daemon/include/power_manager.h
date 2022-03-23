#ifndef _NATIVEPOWER_DAEMON_POWER_MANAGER_H_
#define _NATIVEPOWER_DAEMON_POWER_MANAGER_H_

#define NATIVE_POWER_DISPLAY_LOCK "NativePower.Display.lock"

int releaseWakeLock(const char* id);
int acquireWakeLock(const char* id);
int goToSleep(long event_time_ms, int reason, int flags);
int shutdown();
int reboot();
int userActivity();
int isActivityTimeValid();
int invalidateActivityTime();
int init_awake_timeout();
int set_awake_timeout(long timeout_s);

#endif
