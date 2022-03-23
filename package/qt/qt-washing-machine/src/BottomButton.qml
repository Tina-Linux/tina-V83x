import QtQuick 2.10

Item {
    width: 486
    height: 113
    property variant bottomButtonValue: ["15", "分", "洗涤", "2", "次", "漂洗", "10", "分", "脱水"]

    Image {
        id: imgBottomButtonId1
        x: 0
        source: "images/bottom_fun.png"

        Item {
            /* Dynamic calculation of the required width, depending on the text */
            width: txNumId1.width + txNumUnitId1.width + 5
            height: txNumId1.height
            anchors.top: parent.top
            anchors.topMargin: 10
            anchors.horizontalCenter: parent.horizontalCenter

            Text {
                id: txNumId1
                color: "#fab309"
                text: bottomButtonValue[0]
                font.pixelSize: 45
            }

            Text {
                id: txNumUnitId1
                color: "#ffffff"
                text: {
                    /* If it is Chinese, it will not display the unit */
                    if (isChinese(bottomButtonValue[0])) {
                        return "";
                    } else {
                        return bottomButtonValue[1];
                    }
                }
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                anchors.left: txNumId1.right
                anchors.leftMargin: 5
                font.pixelSize: 30
            }
        }

        Text {
            color: "#ffffff"
            text: bottomButtonValue[2]
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 35
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (lvImgId.visible) {
                    lvImgId.visible = false;
                } else {
                    /* Replace the data in the listview */
                    numModelId.updateModel(0);
                    lvImgId.bottomButtonClickNum = 0;
                    var rowCount = numModelId.count;
                    for(var i = 0; i < rowCount; i++) {
                        /* Determine where the current value is in the listmodel */
                        if (txNumId1.text === numModelId.get(i).number) {
                            /* Jump to this location */
                            listView.positionViewAtIndex(i, ListView.SnapPosition);
                            listView.currentIndex = i;
                            break;
                        }
                    }
                    /* Change the listview and play the display animation */
                    lvImgId.x = 0;
                    lvImgId.opacity = 0;
                    lvImgId.visible = true;
                    lvOpacityAnimatorId.running = true;
                }
            }
        }
    }

    Image {
        id: imgBottomButtonId2
        x: 162
        source: "images/bottom_fun.png"

        Item {
            width: txNumId2.width + txNumUnitId2.width + 5
            height: txNumId2.height
            anchors.top: parent.top
            anchors.topMargin: 10
            anchors.horizontalCenter: parent.horizontalCenter

            Text {
                id: txNumId2
                color: "#fab309"
                text: bottomButtonValue[3]
                font.pixelSize: 45
            }

            Text {
                id: txNumUnitId2
                color: "#ffffff"
                text: {
                    /* If it is Chinese, it will not display the unit */
                    if (isChinese(bottomButtonValue[3])) {
                        return "";
                    } else {
                        return bottomButtonValue[4];
                    }
                }
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                anchors.left: txNumId2.right
                anchors.leftMargin: 5
                font.pixelSize: 30
            }
        }

        Text {
            color: "#ffffff"
            text: bottomButtonValue[5]
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 35
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (lvImgId.visible) {
                    lvImgId.visible = false;
                } else {
                    numModelId.updateModel(1);
                    lvImgId.bottomButtonClickNum = 1;
                    var rowCount = numModelId.count;
                    for (var i = 0; i < rowCount; i++) {
                        if (txNumId2.text === numModelId.get(i).number) {
                            listView.positionViewAtIndex(i, ListView.Center);
                            listView.currentIndex = i;
                            break;
                        }
                    }
                    lvImgId.x = 162;
                    lvImgId.opacity = 0;
                    lvImgId.visible = true;
                    lvOpacityAnimatorId.running = true;
                }
            }
        }
    }

    Image {
        id: imgBottomButtonId3
        x: 324
        source: "images/bottom_fun.png"

        Item {
            width: txNumId3.width + txNumUnitId3.width + 5
            height: txNumId3.height
            anchors.top: parent.top
            anchors.topMargin: 10
            anchors.horizontalCenter: parent.horizontalCenter

            Text {
                id: txNumId3
                color: "#fab309"
                text: bottomButtonValue[6]
                font.pixelSize: 45
            }

            Text {
                id: txNumUnitId3
                color: "#ffffff"
                text: {
                    /* If it is Chinese, it will not display the unit */
                    if (isChinese(bottomButtonValue[6])) {
                        return "";
                    } else {
                        return bottomButtonValue[7];
                    }
                }
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                anchors.left: txNumId3.right
                anchors.leftMargin: 8
                font.pixelSize: 30
            }
        }

        Text {
            color: "#ffffff"
            text: bottomButtonValue[8]
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 35
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (lvImgId.visible) {
                    lvImgId.visible = false;
                } else {
                    numModelId.updateModel(2);
                    lvImgId.bottomButtonClickNum = 2;
                    var rowCount = numModelId.count;
                    for(var i = 0; i < rowCount; i++) {
                        if (txNumId3.text === numModelId.get(i).number) {
                            listView.positionViewAtIndex(i, ListView.Center);
                            listView.currentIndex = i;
                            break;
                        }
                    }
                    lvImgId.x = 324;
                    lvImgId.opacity = 0;
                    lvImgId.visible = true;
                    lvOpacityAnimatorId.running = true;
                }
            }
        }
    }

    /* Set whether to display */
    function setBottomButonVisible(oneVisible, towVisible, threeVisible) {
        imgBottomButtonId1.visible = oneVisible;
        imgBottomButtonId2.visible = towVisible;
        imgBottomButtonId3.visible = threeVisible;
    }

    /* Do not click and change text color */
    function setBottomButonEnable(oneEnable, towEnable, threeEnable) {
        if (!oneEnable) {
            imgBottomButtonId1.enabled = false;
            txNumId1.color = "#909090";
        }
        if (!towEnable) {
            imgBottomButtonId2.enabled = false;
            txNumId2.color = "#909090";
        }
        if (!threeEnable) {
            imgBottomButtonId3.enabled = false;
            txNumId3.color = "#909090";
        }
    }

    /* Dynamically change numbers based on selection */
    function setTextNumValue(index, value) {
        if (index === 0) {
            txNumId1.text = value;
            txNumUnitId1.text = isChinese(value) ? "" : bottomButtonValue[1];
        } else if (index === 1) {
            txNumId2.text = value;
            txNumUnitId2.text = isChinese(value) ? "" : bottomButtonValue[4];
        } else {
            txNumId3.text = value;
            txNumUnitId3.text = isChinese(value) ? "" : bottomButtonValue[7];
        }
    }

    /* Determine if the string is Chinese */
    function isChinese(str)
    {
        var patrn = /[\u4E00-\u9FA5]|[\uFE30-\uFFA0]/gi;
        if (!patrn.exec(str)) {
            return false;
        } else {
            return true;
        }
    }
}
