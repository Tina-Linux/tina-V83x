function getBottomButtonValue(barNum, swipeIndex) {
    if (swipeIndex === 0) {
        switch (barNum) {
        case 1:
            return ["33", "分", "洗涤", "1", "次", "漂洗", "5", "分", "脱水"];
        case 2:
            return ["14", "分", "洗涤", "1", "次", "漂洗", "8", "分", "脱水"];
        case 3:
            return ["2", "次", "漂洗", "低", "", "水位", "冷水", "度", "温度"];
        case 4:
            return ["37", "分", "洗涤", "1", "次", "漂洗", "8", "分", "脱水"];
        case 5:
            return ["8", "分", "洗涤", "1", "次", "漂洗", "8", "分", "脱水"];
        case 6:
            return ["1", "次", "漂洗", "低", "", "水位", "60", "度", "温度"];
        case 7:
        case 8:
            return ["15", "分", "洗涤", "1", "次", "漂洗", "8", "分", "脱水"];
        case 9:
            return ["19", "分", "洗涤", "1", "次", "漂洗", "8", "分", "脱水"];
        case 10:
            return ["22", "分", "洗涤", "1", "次", "漂洗", "8", "分", "脱水"];
        case 11:
            return ["20", "分", "洗涤", "1", "次", "漂洗", "8", "分", "脱水"];
        case 12:
            return ["1", "次", "漂洗", "8", "分", "脱水", "低", "", "水位"];
        case 13:
            return ["12", "分", "脱水", "免脱水", "转", "转速", "1", "次", "漂洗"];
        case 14:
            return ["10", "分", "脱水", "免脱水", "转", "转速", "1", "次", "漂洗"];
        case 15:
            return ["15", "分", "洗涤", "1", "次", "漂洗", "5", "分", "脱水"];
        case 17:
            return ["1", "次", "漂洗", "95", "度", "温度", "5", "分", "脱水"];
        default:
            return ["15", "分", "洗涤", "2", "次", "漂洗", "10", "分", "脱水"];
        }
    } else if (swipeIndex === 1) {
        switch (barNum) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 13:
        case 14:
        case 17:
            return ["低", "", "水位", "冷水", "度", "温度", "免脱水", "转", "转速"];
        case 5:
            return ["低", "", "水位", "20", "度", "温度", "免脱水", "转", "转速"];
        case 6:
            return ["低", "", "水位", "60", "度", "温度", "免脱水", "转", "转速"];
        case 12:
            return ["8", "分", "脱水", "低", "", "水位", "免脱水", "转", "转速"];
        case 15:
            return ["低", "", "水位", "冷水", "度", "温度", "1600", "转", "转速"];
        default:
            return ["低", "", "水位", "冷水", "度", "温度", "免脱水", "转", "转速"];
        }
    }

    return;
}

function getBottomListModel(index) {
    switch (index) {
    case 0:
        /* xidi */
        return [{"number": "8"}, {"number": "11"}, {"number": "14"},
                {"number": "15"}, {"number": "18"}, {"number": "19"},
                {"number": "20"}, {"number": "22"}, {"number": "23"},
                {"number": "28"}, {"number": "33"}, {"number": "37"},
                {"number": "42"}, {"number": "52"}, {"number": "57"},
                {"number": "62"}];
    case 1:
        /* piaoxi */
        return [{"number": "1"}, {"number": "2"}, {"number": "3"},
                {"number": "4"}, {"number": "自动"}];
    case 2:
        /* tuoshui */
        return [{"number": "5"}, {"number": "6"}, {"number": "7"},
                {"number": "8"}, {"number": "9"}, {"number": "10"},
                {"number": "11"}, {"number": "12"}];
    case 3:
        /* shuiwei */
        return [{"number": "低"}, {"number": "中"}, {"number": "高"}];
    case 4:
        /* wendu */
        return [{"number": "冷水"}, {"number": "20"}, {"number": "30"},
                {"number": "40"}, {"number": "60"}, {"number": "95"}];
    case 5:
        /* zhuanshu */
        return [{"number": "免脱水"}, {"number": "400"}, {"number": "600"},
                {"number": "800"}, {"number": "1000"}, {"number": "1200"},
                {"number": "1400"}, {"number": "1600"}];
    default:
        /* xidi */
        return [{"number": "15"}, {"number": "18"}, {"number": "23"},
                {"number": "28"}, {"number": "33"}, {"number": "37"},
                {"number": "42"}, {"number": "52"}, {"number": "57"},
                {"number": "62"}];
    }
}
