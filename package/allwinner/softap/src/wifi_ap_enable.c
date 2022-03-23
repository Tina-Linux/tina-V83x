#include <stdio.h>
#include <stdlib.h>
#include "wifi_ap_enable.h"
#include "get_state_machine.h"

public void setSoftapEnabled(int enable) {
        final ContentResolver cr = mContext.getContentResolver();
        /**
         * Disable Wifi if enabling tethering
         */
        int wifiState = getWifiState();
		int wifistate = get_wifi_machine_state();
        if (enable && ((wifiState == WIFI_STATE_ENABLING) ||
                    (wifiState == WIFI_STATE_ENABLED))) {
            //setWifiEnabled(false);   //disable station mode, do not use in the demo release
            Settings.Global.putInt(cr, Settings.Global.WIFI_SAVED_STATE, 1);
        }

        if (mWifiManager.setWifiApEnabled(null, enable)) {
            /* Disable here, enabled on receiving success broadcast */
            mCheckBox.setEnabled(false);
        } else {
            mCheckBox.setSummary(R.string.wifi_error);
        }

        /**
         *  If needed, restore Wifi on tether disable
         */
        if (!enable) {
            int wifiSavedState = 0;
            withStaEnabled = false;
            try {
                wifiSavedState = Settings.Global.getInt(cr, Settings.Global.WIFI_SAVED_STATE);
                withStaEnabled = wifiSavedState == 1? true : false;
            } catch (Settings.SettingNotFoundException e) {
                ;
            }
            if (wifiSavedState == 1) {
                mWifiManager.setWifiEnabled(true);
                /* Disable here, enabled when sta state change to  WIFI_STATE_ENABLING*/
                mCheckBox.setEnabled(false);
                Settings.Global.putInt(cr, Settings.Global.WIFI_SAVED_STATE, 0);
            }
        }
    }
