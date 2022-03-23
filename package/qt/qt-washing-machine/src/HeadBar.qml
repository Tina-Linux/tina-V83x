import QtQuick 2.10

Image {
    x: 0
    y: 0
    width: 800
    height: 127
    fillMode: Image.PreserveAspectFit
    source: "images/headbkg.png"

    Image{
        x: 20
        y: 14
        width: 109
        height: 60
        source: "images/favor_n.png"

        MouseArea {
            anchors.fill: parent
            onPressed: parent.source = "images/favor_p.png"
            onReleased:  parent.source = "images/favor_n.png"
        }
    }

    Image{
        x: 345
        y: 14
        width: 109
        height: 60
        source: "images/home_n.png"

        MouseArea {
            anchors.fill: parent
            onPressed: parent.source = "images/home_p.png"
            onReleased:  parent.source = "images/home_n.png"
            onClicked: {
                if (ldMainWinId.isMainWin) {
                    if (ldMainWinId.pageNum === 0)
                        return;

                    /* At the time of the desktop, slide to page 0 */
                    ldMainWinId.pageNum = 0;
                    animator.from = itDesktop.x;
                    animator.to = 0;
                    animator.running = true;
                } else {
                    /* Click home at funWin to return to desktop */
                    ldMainWinId.isMainWin = true;
                    ldMainWinId.source = "Desktop.qml";
                }
            }
        }
    }

    Image{
        x: 601
        y: 14
        width: 109
        height: 60
        source: "images/setup_n.png"

        MouseArea {
            anchors.fill: parent
            onPressed: parent.source = "images/setup_p.png"
            onReleased:  parent.source = "images/setup_n.png"
            onClicked: {
                /* Open if the menu window is not open */
                if (!ldSideMenu.visible) {
                    ldSideMenu.source = "SideMenu.qml"
                    ldSideMenu.visible = true;
                    ldSideMenuAnimator.from = 360;
                    ldSideMenuAnimator.to = 0;
                    ldSideMenuAnimator.running = true;
                }
            }
        }
    }

    Text {
        x: 0
        y: 33
        color: "#ffffff"
        text: Qt.formatDateTime(new Date(), "hh:mm:ss");
        anchors.right: parent.right
        anchors.rightMargin: 10
        font.pixelSize: 20

        /* Timer update time */
        Timer {
            id: timer
            interval: 1000
            repeat: true
            onTriggered:{
                parent.text = Qt.formatDateTime(new Date(), "hh:mm:ss");
            }
        }

        Component.onCompleted: {
            timer.start();
        }
    }
}
