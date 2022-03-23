import QtQuick 2.10
import QtQuick.Window 2.10

Window {
    visible: true
    width: 800
    height: 480
    title: qsTr("Washing-machine")

    /* Use Loader to load different windows, including desktops and funWin */
    Loader {
        id: ldMainWinId
        x: 0
        y: 0
        width: 800
        height: 480
        source: "Desktop.qml"
        property bool isMainWin: true
        /* Save the desktop is the first few pages, there are 0, 1, 2 */
        property int pageNum: 0
        /* Save the X-axis coordinates to ensure that they are still
         * in their original position when returning from funWin */
        property int desktopX: 0

        onLoaded: {
            /* Perform desktop window animation when returning from funWin */
            if (isMainWin) {
                item.startAnimator();
            }
        }
    }

    /* Sliding menu 800*480, after the click button is set, the souce is assigned */
    Loader {
        id: ldSideMenu
        x: 360
        visible: false
        property bool isOpen: false

        XAnimator {
            id: ldSideMenuAnimator
            target: ldSideMenu
            easing.type: Easing.OutQuad
            duration: 400

            onStopped: {
                if (!ldSideMenu.isOpen) {
                    ldSideMenu.isOpen = true;
                } else {
                    ldSideMenu.isOpen = false;
                    ldSideMenu.visible = false;
                    /* Source value is empty, saving memory */
                    ldSideMenu.source = ""
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                /* Click on the blank area to return */
                if (mouse.x < 440) {
                    ldSideMenuAnimator.from = 0;
                    ldSideMenuAnimator.to = 360;
                    ldSideMenuAnimator.running = true;
                }
            }
        }
    }
}
