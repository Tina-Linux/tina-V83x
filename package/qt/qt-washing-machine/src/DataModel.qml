import QtQuick 2.10
import "DefaultData.js" as DataFunctions

ListModel {
    ListElement {
        number: "15"
    }

    /* Update list data */
    function updateModel(index) {
        clear();
        if (swBottomId.currentIndex === 0) {
            switch (index) {
            case 0:
                if (bottomBarNum === 3 || bottomBarNum === 6
                        || bottomBarNum === 12 || bottomBarNum === 17) {
                    append(DataFunctions.getBottomListModel(1));
                } else if(bottomBarNum === 13 || bottomBarNum === 14) {
                    append(DataFunctions.getBottomListModel(2));
                } else {
                    append(DataFunctions.getBottomListModel(0));
                }
                break;
            case 1:
                if (bottomBarNum === 3 || bottomBarNum === 6) {
                    append(DataFunctions.getBottomListModel(3));
                } else if(bottomBarNum === 12) {
                    append(DataFunctions.getBottomListModel(2));
                } else if(bottomBarNum === 13 || bottomBarNum === 14) {
                    append(DataFunctions.getBottomListModel(5));
                } else if(bottomBarNum === 17) {
                    append(DataFunctions.getBottomListModel(4));
                } else {
                    append(DataFunctions.getBottomListModel(1));
                }
                break;
            case 2:
                if (bottomBarNum === 3 || bottomBarNum === 6) {
                    append(DataFunctions.getBottomListModel(4));
                } else if(bottomBarNum === 13 || bottomBarNum === 14) {
                    append(DataFunctions.getBottomListModel(1));
                } else if(bottomBarNum === 12) {
                    append(DataFunctions.getBottomListModel(3));
                } else {
                    append(DataFunctions.getBottomListModel(2));
                }
                break;
            }
        } else {
            switch (index) {
            case 0:
                if (bottomBarNum === 12) {
                    append(DataFunctions.getBottomListModel(2));
                } else {
                    append(DataFunctions.getBottomListModel(3));
                }
                break;
            case 1:
                if (bottomBarNum === 12) {
                    append(DataFunctions.getBottomListModel(3));
                } else {
                    append(DataFunctions.getBottomListModel(4));
                }
                break;
            case 2:
                append(DataFunctions.getBottomListModel(5));
                break;
            }
        }
    }
}
