import QtQuick 2.10

Image {
    property string sourceNormal: "images/tongsuo_n.png";
    property string sourceActive: "images/tongsuo_p.png";
    property string title: "童锁";

    width: 100
    height: 100
    source: sourceNormal

    Text {
        id: element
        y: 52
        color: "#274a72"
        text: title
        fontSizeMode: Text.HorizontalFit
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: 20
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onPressed: parent.source = sourceActive
        onReleased:  parent.source = sourceNormal
    }
}
