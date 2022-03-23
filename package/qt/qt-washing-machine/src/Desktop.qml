import QtQuick 2.10
import QtQuick.Controls 2.3

Image {
    id: desktopBgId
    x: 0
    y: 0
    width: 800
    height: 480
    source: "images/background.png"

    HeadBar {
        x: 0
        y: 0
        width: 800
        height: 127
    }

    /* One Item loads three home pages */
    Item {
        id: itDesktop
        x: desktopX
        y: 127
        width: 2400
        height: 316

        MainPage {
            id: pageOneId
            width: 800
            height: 316
        }

        MainPage {
            id: pageTowId
            x: 800
            width: 800
            height: 316

            oneBg: "images/fun_huoxingmei.png";
            towBg: "images/fun_yangmao.png";
            threeBg: "images/fun_tongzhuang.png";
            fourBg: "images/fun_neiyi.png";
            fivesBg: "images/fun_yurongfu.png";
            sixBg: "images/fun_jieneng.png";
        }

        MainPage {
            id: pageThreeId
            x: 1600
            width: 800
            height: 316

            oneBg: "images/fun_piaoxi.png";
            towBg: "images/fun_tuoshui.png";
            threeBg: "images/fun_honggan.png";
            fourBg: "images/fun_xihong.png";
            fivesBg: "images/fun_kongqixi.png";
            sixBg: "images/fun_jianzijie.png";
        }

        /* Settings can be dragged */
        MouseArea {
            id: desktopMouseId
            anchors.fill: parent
            propagateComposedEvents: true
            drag.target: itDesktop
            drag.axis: Drag.XAxis
            drag.maximumX: 40
            drag.minimumX: -1640
            property int lastX: 0

            onPressed: {
                lastX = mouse.x;
            }

            onReleased: {
                /* If the distance is too small, it will not change pages. */
                if (Math.abs(mouse.x - lastX) < 5)
                    return;

                if (mouse.x - lastX < 0) {
                    pageNum++;
                    pageNum = pageNum > 2 ? 2 : pageNum;
                } else {
                    pageNum--;
                    pageNum = pageNum < 0 ? 0 : pageNum;
                }

                /* Play the animation after releasing the mouse */
                lastX = mouse.x;
                animator.from = itDesktop.x;
                animator.to = pageNum * (-800);
                animator.running = true;
            }
        }

        XAnimator {
            id: animator
            target: itDesktop
            easing.type: Easing.OutQuad
            duration: 400

            onStarted: {
                desktopMouseId.enabled = false
            }

            onStopped: {
                desktopX = pageNum * (-800);
                desktopMouseId.enabled = true
            }
        }
    }

    /* Sliding indicator */
    PageIndicator {
        id: piDesktop
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter

        count: 3
        currentIndex: pageNum

        delegate: Rectangle {
            implicitWidth: 10
            implicitHeight: 10
            radius: width / 2
            color: "#aa8f00"

            opacity: index === piDesktop.currentIndex ? 0.95 : pressed ? 0.7 : 0.45

            Behavior on opacity {
                OpacityAnimator {
                    duration: 200
                }
            }
        }
    }

    function startAnimator()
    {
        if (pageNum === 0)
            pageOneId.startAnimator();
        else if(pageNum ===1 )
            pageTowId.startAnimator();
        else if(pageNum === 2)
            pageThreeId.startAnimator();
    }
}
