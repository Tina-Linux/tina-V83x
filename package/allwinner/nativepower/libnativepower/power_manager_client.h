#ifndef _NATIVEPOWER_DAEMON_POWER_MANAGER_H_
#define _NATIVEPOWER_DAEMON_POWER_MANAGER_H_

#if __cplusplus
extern "C" {
#endif

int PowerManagerSuspend(long event_uptime);
int PowerManagerShutDown(void);
int PowerManagerReboot(void);
int PowerManagerAcquireWakeLock(const char *id);
int PowerManagerReleaseWakeLock(const char *id);
int PowerManagerUserActivity(void);
int PowerManagerSetAwakeTimeout(long timeout_s);
int PowerManagerSetScene(const char *scene);

#if __cplusplus
}				// extern "C"
#endif
#endif
