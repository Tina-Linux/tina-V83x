import QtQuick 2.10

Image {
    id: image
    source: "images/funbtn.png"

    Image {
        id: imgRotateId
        anchors.horizontalCenterOffset: 65
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        source: "images/anim_00.png"

        /* Rotate 360 degree animation */
        RotationAnimator on rotation {
            id: ranimatorId
            running : false
            from: 0;
            to: 360;
            duration: 300

            onStopped: {
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
        id: txFunId
        color: "#ffffff"
        text: "功能"
        anchors.horizontalCenterOffset: -40
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: 50
    }

    MouseArea {
        anchors.fill: parent

        /* Press to enlarge text and image */
        onPressed: {
            parent.source = "images/funbtn_big.png"
            imgRotateId.source = "images/plus.png"
            txFunId.font.pixelSize = 60
        }

        /* Release reduced text and images */
        onReleased: {
            parent.source = "images/funbtn.png"
            imgRotateId.source = "images/anim_00.png"
            txFunId.font.pixelSize = 50
            ranimatorId.running = true
        }
    }
}
