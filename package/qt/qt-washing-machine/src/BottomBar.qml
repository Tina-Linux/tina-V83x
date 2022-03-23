import QtQuick 2.10
import QtQuick.Controls 2.3
import "DefaultData.js" as DataFunctions

Item {
    width: 800
    height: 113
    property int bottomBarNum: 0

    Image {
        id: lvImgId
        y: -226
        height: 226
        opacity: 0
        visible: false
        source: "images/bottom_fun.png"
        /* Save which button was clicked, the value is 0, 1, 2 */
        property int bottomButtonClickNum: 0

        Component {
            id: petDelegate

            Item {
                id: wrapper
                width: 162
                height: 55

                Text {
                    id: listViewItem
                    text: number
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pointSize: 10
                    color: "#fab309"
                }

                /* amplification the item if it is the current item */
                states: State {
                    name: "Current"
                    when: wrapper.ListView.isCurrentItem
                    PropertyChanges { target: listViewItem; font.pointSize: 20 }
                    PropertyChanges { target: listViewItem; y: -15 }
                }

                /* Over-magnification */
                transitions: Transition {
                    NumberAnimation { properties: "font.pointSize"; duration: 100 }
                }
            }
        }

        ListView {
            id: listView
            width: 162
            height: 226
            clip: true
            /* Set the location of the selected item */
            preferredHighlightBegin : 100
            preferredHighlightEnd: 160

            model: DataModel {id: numModelId}
            delegate: petDelegate
            focus: true

            highlightFollowsCurrentItem: false
            /* currentIndex dynamic change */
            highlightRangeMode: ListView.StrictlyEnforceRange

            onCurrentIndexChanged: {
                /* Dynamically change the value of bottomButton when currentIndex changes */
                if (swBottomId.currentIndex === 0) {
                    bottomButtonId1.setTextNumValue(lvImgId.bottomButtonClickNum,
                                                    numModelId.get(currentIndex).number);
                } else {
                    bottomButtonId2.setTextNumValue(lvImgId.bottomButtonClickNum,
                                                    numModelId.get(currentIndex).number);
                }
            }
        }

        /* Listview appears animation */
        OpacityAnimator on opacity {
            id: lvOpacityAnimatorId
            from: 0
            to: 1
            running: false
            duration: 400
        }
    }

    /* Slide the page change control left and right */
    SwipeView {
        id: swBottomId
        width: 486
        height: 113
        clip: true
        interactive: false

        BottomButton {
            id: bottomButtonId1
            bottomButtonValue: DataFunctions.getBottomButtonValue(bottomBarNum, 0)

            Component.onCompleted: {
                /* Set the visible and clickable state of the button according to the bottomBarNum */
                if (bottomBarNum === 0 || bottomBarNum === 16)
                    setBottomButonVisible(false, false, false);
                if (bottomBarNum === 13 || bottomBarNum === 14
                        || bottomBarNum === 17)
                    setBottomButonVisible(true, true, false);
                if(bottomBarNum === 3)
                    setBottomButonEnable(false, true, true);
                if(bottomBarNum === 6)
                    setBottomButonEnable(true, true, false);
                if(bottomBarNum === 14)
                    setBottomButonEnable(false, true, true);
                if(bottomBarNum === 17)
                    setBottomButonEnable(false, false, true);
            }
        }

        BottomButton {
            id: bottomButtonId2
            bottomButtonValue: DataFunctions.getBottomButtonValue(bottomBarNum, 1)

            Component.onCompleted: {
                /* Set the visible and clickable state of the button according to the bottomBarNum */
                if (bottomBarNum === 0 || bottomBarNum === 16)
                    setBottomButonVisible(false, false, false);
                if (bottomBarNum === 13 || bottomBarNum === 14
                        || bottomBarNum === 17)
                    setBottomButonVisible(true, true, false);
                if(bottomBarNum === 6)
                    setBottomButonEnable(true, false, true);
            }
        }
    }

    Image {
        x: 486
        source: "images/next.png"
        visible: (bottomBarNum === 0) ||
                 (bottomBarNum === 13) ||
                 (bottomBarNum === 14) ||
                 (bottomBarNum === 16) ||
                 (bottomBarNum === 17) ? false : true

        MouseArea {
            anchors.fill: parent
            property bool isClicked: false
            onClicked: {
                if (!lvImgId.visible) {
                    if (isClicked) {
                        parent.source = "images/next.png"
                        isClicked = false
                        swBottomId.currentIndex = 0
                    } else {
                        parent.source = "images/pre.png"
                        isClicked = true
                        swBottomId.currentIndex = 1
                    }
                } else {
                    lvImgId.visible = false;
                }
            }
        }
    }

    Image {
        id: image
        x: 553
        source: "images/begin.png"

        Text {
            id: txBeginId
            color: "#ffffff"
            text: qsTr("开始")
            anchors.left: parent.left
            anchors.leftMargin: 25
            anchors.verticalCenterOffset: 0
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: 50
        }

        MouseArea {
            anchors.fill: parent
            property bool isClicked: false

            onClicked: {
                if (isClicked) {
                    parent.source = "images/begin.png"
                    isClicked = false
                    txBeginId.text = "开始"
                } else {
                    parent.source = "images/pause.png"
                    isClicked = true
                    txBeginId.text = "结束"
                }
            }
        }
    }

    function setListViewVisible(visible) {
        lvImgId.visible = visible;
    }

    function getListViewVisible(visible) {
        return lvImgId.visible;
    }
}
