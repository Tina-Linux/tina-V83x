// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/*
 * This demo application is the same as the basic-first-sample in cortana-sdk
 */

#include "cortana-core/cortana-core.h"
#ifdef WIN32
#include <conio.h>
#else
#include <sys/select.h>
#endif
#include <stdio.h>

/**
 * A user defined callback to handle various Cortana state transitions.
 * @param handle The Cortana handle.
 * @param pContext A pointer to the application-defined callback context.
 * @param state The current Cortana state transition.
 * @param reason The reason for the transition.
 */
static void OnCortanaState(
    CORTANA_HANDLE          handle,
    void*                   pContext,
    CORTANA_STATE           state,
    CORTANA_REASON_STATE    reason)
{
    switch (state)
    {
    case CORTANA_STATE_READY:
        printf("\x1B[32mCORTANA_STATE_READY - Turn off LED\x1B[0m\n");
        break;
    case CORTANA_STATE_LISTENING:
        printf("\x1B[34mCORTANA_STATE_LISTENING - Turn on main LED\x1B[0m\n");
        break;
    case CORTANA_STATE_THINKING:
        printf("\x1B[36mCORTANA_STATE_THINKING - Spin main LED\x1B[0m\n");
        break;
    case CORTANA_STATE_SPEAKING:
        printf("\x1B[36mCORTANA_STATE_SPEAKING - Flash main LED\x1B[0m\n");
        break;
    }
}

/**
 * A helper function to detect key events.
 * @return 0 when no keypress is detected, non-zero when a key event is ready.
 */
#ifdef WIN32
#define kbhit _kbhit
#else
int kbhit()
{
    struct timeval timeout;
    fd_set fdSet;

    FD_ZERO(&fdSet);
    FD_SET(0, &fdSet);

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    return select(1, &fdSet, NULL, NULL, &timeout);
}
#endif

/**
 * A user defined callback to process keypress events and dispatch them.
 * @param handle The Cortana handle.
 * @param pContext A pointer to the application-defined callback context.
 * @return A return code or zero if successful.  Any non-zero value will terminate Cortana.
 */
static int OnIdle(CORTANA_HANDLE handle, void* pContext)
{
    static int mute = 0;

    if (kbhit())
    {
        switch (getchar())
        {

        case 's':
            cortana_onaction(handle, CORTANA_ACTION_SHORT_PRESS, NULL);
            break;

        case 'l':
            cortana_onaction(handle, CORTANA_ACTION_LONG_PRESS, NULL);
            break;

        case 'm':
            mute = !mute;
            cortana_onaction(handle, mute ? CORTANA_ACTION_VOICE_MUTE : CORTANA_ACTION_VOICE_UNMUTE, NULL);
            break;

        case '+':
            cortana_onaction(handle, CORTANA_ACTION_VOLUME_UP, NULL);
            break;

        case '-':
            cortana_onaction(handle, CORTANA_ACTION_VOLUME_DOWN, NULL);
            break;

        case 'q':
            printf("Quitting Cortana!");
            return 1;

        default:
            break;
        }
    }

    return 0; // A return value of zero keeps Cortana running.
}

/**
 * A defined list of callbacks we are interested in.
 */
static CORTANA_CALLBACKS callbacks =
{
    sizeof(CORTANA_CALLBACKS),      // size
    CORTANA_CALLBACKS_VERSION,      // version
    NULL,                           // OnInitialized
    NULL,                           // OnShutdown
    NULL,                           // OnError
    NULL,                           // OnSpeech
    NULL,                           // OnAudioInputState
    NULL,                           // OnAudioOutputState
    NULL,                           // OnKeywordState
    NULL,                           // OnOEMGetProperty
    OnCortanaState,                 // OnCortanaState
    NULL,                           // OnView
    NULL,                           // OnSkill
    OnIdle,                         // OnIdle
    NULL,                           // OnVolumeChanged
    NULL,                           // OnTimerChanged
    NULL,                           // OnAudioDuck
    NULL,                           // OnTelephonyState
    NULL,                           // OnsystemStateChanged
    NULL,                           // OnGetVolume
    NULL,                           // OnBluetoothEvent
    NULL,                           // OnTimezoneSet
};


/**
 * Our main entry point.
 */
int main(int argc, char* argv[])
{

    /**
     * cortana_run starts the full cortana process and will block until shutdown.
     */
    cortana_run(&callbacks, CORTANA_START_FLAGS_DEFAULT, NULL);
    return 0;
}
