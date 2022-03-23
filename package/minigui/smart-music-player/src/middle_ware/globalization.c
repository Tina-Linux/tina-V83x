/*
 * globalization.c
 *
 *  Created on: 2019/8/16
 *      Author: anruliu
 */

#include "globalization.h"
#include "resource.h"

char* getDateText(int wday) {
    switch (wday) {
    case 0:
        return (languageType == Language_CN) ? "星期日" : "Sunday";
    case 1:
        return (languageType == Language_CN) ? "星期一" : "Monday";
    case 2:
        return (languageType == Language_CN) ? "星期二" : "Tuesday";
    case 3:
        return (languageType == Language_CN) ? "星期三" : "Wednesday";
    case 4:
        return (languageType == Language_CN) ? "星期四" : "Thursday";
    case 5:
        return (languageType == Language_CN) ? "星期五" : "Friday";
    case 6:
        return (languageType == Language_CN) ? "星期六" : "Saturday";
    case 7:
        return (languageType == Language_CN) ? "月" : "/";
    case 8:
        return (languageType == Language_CN) ? "日" : " ";
    case 9:
        return (languageType == Language_CN) ? "秒" : "S";
    default:
        return "";
    }
    return "";
}

char* getDeskButtonText(int flag, int index) {
    if (flag == 0) {
        switch (index) {
        case 0:
            return (languageType == Language_CN) ? "音乐" : "Music";
        case 1:
            return (languageType == Language_CN) ? "本地" : "Local";
        case 2:
            return (languageType == Language_CN) ? "蓝牙" : "Bht";
        case 3:
            return (languageType == Language_CN) ? "外音" : "Aux";
        case 4:
            return (languageType == Language_CN) ? "闹钟" : "Alarm";
        case 5:
            return (languageType == Language_CN) ? "设置" : "Setting";
        default:
            return "";
        }
    } else {
        switch (index) {
        case 0:
            return (languageType == Language_CN) ? "回家" : "Go Home";
        case 1:
            return (languageType == Language_CN) ? "工作" : "Working";
        case 2:
            return (languageType == Language_CN) ? "用餐" : "Dining";
        case 3:
            return (languageType == Language_CN) ? "睡觉" : "Sleeping";
        default:
            return "";
        }
    }
    return "";
}

char* getSceneText(int index) {
    if (index == 0) {
        return (languageType == Language_CN) ? "情景" : "Scene";
    } else {
        return (languageType == Language_CN) ?
                "智能生活，一点就好" : "Smart life, just a little";
    }
    return "";
}

char* getHeadbarDesText(int index) {
    switch (index) {
    case 0:
        return (languageType == Language_CN) ? "音乐播放" : "Music Play";
    case 1:
        return (languageType == Language_CN) ? "音乐列表" : "Music List";
    case 2:
        return (languageType == Language_CN) ? "蓝牙音乐" : "Bht Music";
    case 3:
        return (languageType == Language_CN) ? "外部音频" : "Music Aux";
    case 4:
        return (languageType == Language_CN) ? "闹钟设置" : "Alarm Setting";
    case 5:
        return (languageType == Language_CN) ? "系统设置" : "System Setting";
    case 6:
        return (languageType == Language_CN) ? "系统密码设置" : "PWD Setting";
    case 7:
        return (languageType == Language_CN) ? "管理员设置" : "Admini Setting";
    default:
        return "";
    }
    return "";
}

char* getSettingDesText(int index) {
    switch (index) {
    case 0:
        return (languageType == Language_CN) ?
                "请输入管理员密码" : "Please enter admini pwd";
    case 1:
        return (languageType == Language_CN) ? "语言" : "Language";
    case 2:
        return (languageType == Language_CN) ? "自动关屏" : "Auto Off";
    case 3:
        return (languageType == Language_CN) ? "默认音量" : "Initial Vol";
    case 4:
        return (languageType == Language_CN) ? "播放时间" : "Play Time";
    case 5:
        return (languageType == Language_CN) ? "保存音量" : "Save Vol";
    case 6:
        return (languageType == Language_CN) ? "自动播放" : "Auto Play";
    case 7:
        return (languageType == Language_CN) ? "欢迎词播放" : "Welcome Play";
    case 8:
        return (languageType == Language_CN) ? "时间设置" : "Time Setting";
    case 9:
        return (languageType == Language_CN) ? "蓝牙名称" : "Bluetooth Name";
    case 10:
        return (languageType == Language_CN) ? "恢复出厂设置" : "Restore";
    case 11:
        return (languageType == Language_CN) ? "年份" : "Years";
    case 12:
        return (languageType == Language_CN) ? "月份" : "Month";
    case 13:
        return (languageType == Language_CN) ? "日期" : "Date";
    case 14:
        return (languageType == Language_CN) ? "小时" : "Hour";
    case 15:
        return (languageType == Language_CN) ? "分钟" : "Minute";
    case 16:
        return (languageType == Language_CN) ? "提示" : "Hint";
    case 17:
        return (languageType == Language_CN) ? "恢复为出厂设置?" : "Reset settings?";
    case 18:
        return (languageType == Language_CN) ? "中文" : "CHS";
    case 19:
        return (languageType == Language_CN) ? "英文" : "EN";
    default:
        return "";
    }
    return "";
}

char* getButtonDesText(int index) {
    switch (index) {
    case 0:
        return (languageType == Language_CN) ? "删除" : "Delete";
    case 1:
        return (languageType == Language_CN) ? "保存" : "Save";
    case 2:
        return (languageType == Language_CN) ? "确定" : "Confirm";
    case 3:
        return (languageType == Language_CN) ? "取消" : "Cancel";
    default:
        return "";
    }
    return "";
}

char* getAlarmText(int index) {
    switch (index) {
    case 0:
        return (languageType == Language_CN) ? "时间" : "Time";
    case 1:
        return (languageType == Language_CN) ? "铃声" : "Ring";
    default:
        return "";
    }
    return "";
}

char* getHintText(int index) {
    switch (index) {
    case 0:
        return (languageType == Language_CN) ? "正在扫描歌曲..." : "Scanning songs...";
    default:
        return "";
    }
    return "";
}

char* getMusicText(int index) {
    switch (index) {
    case 0:
        return (languageType == Language_CN) ? "爱你一万年" : "爱你一万年";
    case 1:
        return (languageType == Language_CN) ? "你的酒馆被我打了" : "你的酒馆被我打了";
    case 2:
        return (languageType == Language_CN) ? "我拿什么拯救你" : "我拿什么拯救你";
    case 3:
        return (languageType == Language_CN) ? "你的样子" : "你的样子";
    case 4:
        return (languageType == Language_CN) ? "时间煮雨" : "时间煮雨";
    case 5:
        return (languageType == Language_CN) ? "活着" : "活着";
    case 6:
        return (languageType == Language_CN) ? "算你狠" : "算你狠";
    case 7:
        return (languageType == Language_CN) ? "稻草人" : "稻草人";
    case 8:
        return (languageType == Language_CN) ? "勇敢勇敢" : "勇敢勇敢";
    case 9:
        return (languageType == Language_CN) ? "风的季节" : "风的季节";
    case 10:
        return (languageType == Language_CN) ? "当你老了" : "当你老了";
    case 11:
        return (languageType == Language_CN) ? "大雨还在下" : "大雨还在下";
    case 12:
        return (languageType == Language_CN) ? "蓝莲花" : "蓝莲花";
    case 13:
        return (languageType == Language_CN) ? "消愁" : "消愁";
    case 14:
        return (languageType == Language_CN) ? "演员" : "演员";
    case 15:
        return (languageType == Language_CN) ? "成都" : "成都";
    default:
        return "";
    }
    return "";
}
