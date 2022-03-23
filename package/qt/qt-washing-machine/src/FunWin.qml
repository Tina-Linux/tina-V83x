import QtQuick 2.10

Image {
    property string funWinBg: "images/zhineng.png";
    property string funWinTitle: "智能洗衣专家";
    property string funWinDes: "全智能感知:给衣物最好的洗涤";
    /* Open window, 0 to 17 */
    property int funWinNum: 0

    id: funWinId
    width: 800
    height: 480
    fillMode: Image.PreserveAspectFit
    source: funWinBg

    /* Set the click area. If the listview is displayed,
     * click on other areas to hide the listview. */
    MouseArea {
        anchors.fill: parent
        onClicked: {
            if (funWinBottomBarId.getListViewVisible())
                funWinBottomBarId.setListViewVisible(false);
        }
    }

    HeadBar {
        id: funWinHeadBarId
        x: 0
        y: 0
    }

    Text {
        x: 20
        y: 174
        color: "#ffffff"
        text: funWinTitle
        font.pixelSize: 52
    }

    Text {
        x: 20
        y: 250
        color: "#ffffff"
        text: funWinDes
        font.pixelSize: 30
    }

    Text {
        color: "#ffffff"
        text: Qt.formatDateTime(new Date(), "hh:mm")
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: 100
        visible: {
            if (funWinBg === "images/zhineng.png")
                return false;
            else
                return true;
        }

        /* Timer update time */
        Timer {
            id: timer
            interval: 1000
            repeat: true
            onTriggered:{
                parent.text = Qt.formatDateTime(new Date(), "hh:mm");
            }
        }

        Component.onCompleted: {
            timer.start();
        }
    }

    RotateButton {
        y: 0
        anchors.horizontalCenterOffset: 265
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }

    BottomBar {
        id: funWinBottomBarId
        x: 0
        y: 367
        bottomBarNum: funWinNum
    }

    /* When funwin appears, play HeadBar down, BottomBar up animation */
    ParallelAnimation {
        running: true

        YAnimator {
            target: funWinHeadBarId
            easing.type: Easing.OutQuad
            duration: 400
            from:  -127
            to: 0
        }

        YAnimator {
            target: funWinBottomBarId
            easing.type: Easing.OutQuad
            duration: 400
            from: 603
            to: 367
        }
    }
}
