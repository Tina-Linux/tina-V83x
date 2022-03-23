#ifndef BLUETOOTH_H
#define BLUETOOTH_H

/* bt control cmd */
typedef enum BT_CONTROL {
    BLE_OPEN_SERVER,
    BLE_CLOSE_SERVER,
    BLE_SERVER_SEND,
    BLE_IS_OPENED,
    GET_BT_MAC
}BtControl;

int bluetooth_control(BtControl cmd, void *data, int len);

#endif
