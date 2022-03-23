import QtQuick 2.10

Rectangle {
    width: 800
    height: 480
    color: "#00000000"

    Rectangle {
        x: 440
        width: 360
        height: 480
        color: "#e3e3e3"
    }

    SideButton {
        x: 452
        y: 8
        sourceNormal: "images/tongsuo_n.png";
        sourceActive: "images/tongsuo_p.png";
        title: "童锁"
    }

    SideButton {
        x: 572
        y: 8
        sourceNormal: "images/tongdeng_n.png";
        sourceActive: "images/tongdeng_p.png";
        title: "筒灯"
    }

    SideButton {
        x: 692
        y: 8
        sourceNormal: "images/wifi_n.png";
        sourceActive: "images/wifi_p.png";
        title: "wifi"
    }

    SideButton {
        x: 452
        y: 128
        sourceNormal: "images/shuiguanjia_n.png";
        sourceActive: "images/shuiguanjia_p.png";
        title: "水管家"
    }

    SideButton {
        x: 572
        y: 128
        sourceNormal: "images/jingyin_n.png";
        sourceActive: "images/jingyin_p.png";
        title: "音量"
    }

    SideButton {
        x: 692
        y: 128
        sourceNormal: "images/light_n.png";
        sourceActive: "images/light_p.png";
        title: "亮度"
    }

    SideButton {
        x: 452
        y: 248
        sourceNormal: "images/language_n.png";
        sourceActive: "images/language_p.png";
        title: "语言"
    }

    SideButton {
        x: 572
        y: 248
        sourceNormal: "images/video_n.png";
        sourceActive: "images/video_p.png";
        title: "视频"
    }

    SideButton {
        x: 692
        y: 248
        sourceNormal: "images/czzy_n.png";
        sourceActive: "images/czzy_p.png";
        title: "操作指引"
    }

    SideButton {
        x: 452
        y: 368
        sourceNormal: "images/reset_n.png";
        sourceActive: "images/reset_p.png";
        title: "恢复默认"
    }

    SideButton {
        x: 572
        y: 368
        sourceNormal: "images/ywbs_n.png";
        sourceActive: "images/ywbs_p.png";
        title: "衣物标识"
    }

    SideButton {
        x: 692
        y: 368
        sourceNormal: "images/xyjq_n.png";
        sourceActive: "images/xyjq_p.png";
        title: "洗衣技巧"
    }
}
