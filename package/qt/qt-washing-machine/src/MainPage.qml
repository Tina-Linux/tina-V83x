import QtQuick 2.10

Item {
    width: 800
    height: 316
    property string oneBg: "images/fun_zhineng.png";
    property string towBg: "images/fun_mianma.png";
    property string threeBg: "images/fun_hunhexi.png";
    property string fourBg: "images/fun_kuaixi.png";
    property string fivesBg: "images/fun_dajian.png";
    property string sixBg: "images/fun_chenshan.png";

    Image {
        id: image1
        width: 241
        height: 148
        anchors.left: parent.left
        anchors.leftMargin: 20
        fillMode: Image.PreserveAspectFit
        source: oneBg

        MouseArea {
            anchors.fill: parent
            onClicked: {
                ldMainWinId.isMainWin = false;
                if (ldMainWinId.pageNum === 0)
                    ldMainWinId.setSource("FunWin.qml", {"funWinBg": "images/zhineng.png",
                                              "funWinTitle": "智能洗衣专家",
                                              "funWinDes": "全智能感知:给衣物最好的洗涤",
                                              "funWinNum": 0});
                else if (ldMainWinId.pageNum === 1)
                    ldMainWinId.setSource("FunWin.qml", {"funWinBg": "images/huoxingmei.png",
                                              "funWinTitle": "活性酶",
                                              "funWinDes": "较脏且耐洗的衣物",
                                              "funWinNum": 6});
                else if (ldMainWinId.pageNum === 2)
                    ldMainWinId.setSource("FunWin.qml", {"funWinBg": "images/piaoxi.png",
                                              "funWinTitle": "漂洗+脱水",
                                              "funWinDes": "单独的漂洗加脱水",
                                              "funWinNum": 12});
            }
        }
    }

    Image {
        id: image2
        width: 241
        height: 148
        anchors.left: parent.left
        anchors.leftMargin: 281
        fillMode: Image.PreserveAspectFit
        source: towBg

        MouseArea {
            anchors.fill: parent
            onClicked: {
                ldMainWinId.isMainWin = false;
                if (ldMainWinId.pageNum === 0)
                    ldMainWinId.setSource("FunWin.qml", {"funWinBg": "images/mianma.png",
                                              "funWinTitle": "棉麻",
                                              "funWinDes": "日常棉麻衣物",
                                              "funWinNum": 1});
                else if (ldMainWinId.pageNum === 1)
                    ldMainWinId.setSource("FunWin.qml", {"funWinBg": "images/yangmao.png",
                                              "funWinTitle": "羊毛",
                                              "funWinDes": "可机洗的羊毛类衣物",
                                              "funWinNum": 7});
                else if (ldMainWinId.pageNum === 2)
                    ldMainWinId.setSource("FunWin.qml", {"funWinBg": "images/tuoshui.png",
                                              "funWinTitle": "单脱水",
                                              "funWinDes": "单独的脱水程序",
                                              "funWinNum": 13});
            }
        }
    }

    Image {
        id: image3
        width: 241
        height: 148
        anchors.left: parent.left
        anchors.leftMargin: 542
        fillMode: Image.PreserveAspectFit
        source: threeBg

        MouseArea {
            anchors.fill: parent
            onClicked: {
                ldMainWinId.isMainWin = false;
                if (ldMainWinId.pageNum === 0)
                    ldMainWinId.setSource("FunWin.qml", {"funWinBg": "images/hunhexi.png",
                                              "funWinTitle": "混合洗",
                                              "funWinDes": "日常不褪色衣物混合",
                                              "funWinNum": 2});
                else if (ldMainWinId.pageNum === 1)
                    ldMainWinId.setSource("FunWin.qml", {"funWinBg": "images/tongzhuang.png",
                                              "funWinTitle": "童装",
                                              "funWinDes": "儿童类衣物",
                                              "funWinNum": 8});
                else if (ldMainWinId.pageNum === 2)
                    ldMainWinId.setSource("FunWin.qml", {"funWinBg": "images/honggan.png",
                                              "funWinTitle": "单烘干",
                                              "funWinDes": "单独的烘干程序",
                                              "funWinNum": 14});
            }
        }
    }

    Image {
        id: image4
        width: 241
        height: 148
        anchors.top: parent.top
        anchors.topMargin: 168
        anchors.left: parent.left
        anchors.leftMargin: 20
        fillMode: Image.PreserveAspectFit
        source: fourBg

        MouseArea {
            anchors.fill: parent
            onClicked: {
                ldMainWinId.isMainWin = false;
                if (ldMainWinId.pageNum === 0)
                    ldMainWinId.setSource("FunWin.qml", {"funWinBg": "images/kuaixi.png",
                                              "funWinTitle": "快洗15'",
                                              "funWinDes": "微脏的少量衣物",
                                              "funWinNum": 3});
                else if (ldMainWinId.pageNum === 1)
                    ldMainWinId.setSource("FunWin.qml", {"funWinBg": "images/neiyi.png",
                                              "funWinTitle": "内衣",
                                              "funWinDes": "内衣类衣物",
                                              "funWinNum": 9});
                else if (ldMainWinId.pageNum === 2)
                    ldMainWinId.setSource("FunWin.qml", {"funWinBg": "images/xihong.png",
                                              "funWinTitle": "洗烘60'",
                                              "funWinDes": "一次性洗烘\n微脏的少件衣物",
                                              "funWinNum": 15});
            }
        }
    }

    Image {
        id: image5
        width: 241
        height: 148
        anchors.top: parent.top
        anchors.topMargin: 168
        anchors.left: parent.left
        anchors.leftMargin: 281
        fillMode: Image.PreserveAspectFit
        source: fivesBg

        MouseArea {
            anchors.fill: parent
            onClicked: {
                ldMainWinId.isMainWin = false;
                if (ldMainWinId.pageNum === 0)
                    ldMainWinId.setSource("FunWin.qml", {"funWinBg": "images/dajian.png",
                                              "funWinTitle": "大件",
                                              "funWinDes": "较厚重或较大的衣物",
                                              "funWinNum": 4});
                else if (ldMainWinId.pageNum === 1)
                    ldMainWinId.setSource("FunWin.qml", {"funWinBg": "images/yurongfu.png",
                                              "funWinTitle": "羽绒服",
                                              "funWinDes": "可机洗的羽绒类衣物",
                                              "funWinNum": 10});
                else if (ldMainWinId.pageNum === 2)
                    ldMainWinId.setSource("FunWin.qml", {"funWinBg": "images/kongqixi.png",
                                              "funWinTitle": "空气洗",
                                              "funWinDes": "暖风清新衣物",
                                              "funWinNum": 16});
            }
        }
    }

    Image {
        id: image6
        width: 241
        height: 148
        anchors.top: parent.top
        anchors.topMargin: 168
        anchors.left: parent.left
        anchors.leftMargin: 542
        fillMode: Image.PreserveAspectFit
        source: sixBg

        MouseArea {
            anchors.fill: parent
            onClicked: {
                ldMainWinId.isMainWin = false;
                if (ldMainWinId.pageNum === 0)
                    ldMainWinId.setSource("FunWin.qml", {"funWinBg": "images/chenshan.png",
                                              "funWinTitle": "衬衫",
                                              "funWinDes": "衬衫类衣物",
                                              "funWinNum": 5});
                else if (ldMainWinId.pageNum === 1)
                    ldMainWinId.setSource("FunWin.qml", {"funWinBg": "images/jieneng.png",
                                              "funWinTitle": "节能",
                                              "funWinDes": "能耗相对较低",
                                              "funWinNum": 11});
                else if (ldMainWinId.pageNum === 2)
                    ldMainWinId.setSource("FunWin.qml", {"funWinBg": "images/jianzijie.png",
                                              "funWinTitle": "简自洁",
                                              "funWinDes": "特设的自清洁程序",
                                              "funWinNum": 17});
            }
        }
    }

    /* Play six control animations simultaneously */
    ParallelAnimation {
        id: paMainPageOneAnimation

        XAnimator {
            target: image1
            easing.type: Easing.OutQuad
            duration: 300
            from: 800
            to: 20
        }

        XAnimator {
            target: image2
            easing.type: Easing.OutQuad
            duration: 400
            from: 1041
            to: 281
        }

        XAnimator {
            target: image3
            easing.type: Easing.OutQuad
            duration: 500
            from: 1282
            to: 542
        }

        XAnimator {
            target: image4
            easing.type: Easing.OutQuad
            duration: 400
            from: 800
            to: 20
        }

        XAnimator {
            target: image5
            easing.type: Easing.OutQuad
            duration: 500
            from: 1041
            to: 281
        }

        XAnimator {
            target: image6
            easing.type: Easing.OutQuad
            duration: 600
            from: 1282
            to: 542
        }

        /* Do not click when playing an animation */
        onStarted: desktopMouseId.enabled = false;
        onStopped: desktopMouseId.enabled = true;
    }

    function startAnimator()
    {
        paMainPageOneAnimation.running = true;
    }
}
