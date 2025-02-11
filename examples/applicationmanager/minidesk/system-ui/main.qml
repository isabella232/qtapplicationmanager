// Copyright (C) 2021 The Qt Company Ltd.
// Copyright (C) 2019 Luxoft Sweden AB
// Copyright (C) 2018 Pelagicore AG
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick 2.11
import QtQuick.Window 2.11
import QtApplicationManager.SystemUI 2.0

Window {
    title: "Minidesk - QtApplicationManager Example"
    width: 1024
    height: 640
    color: "whitesmoke"

    Readme {}

    Text {
        anchors.bottom: parent.bottom
        text: (ApplicationManager.singleProcess ? "Single" : "Multi") + "-Process Mode"
    }

    // Application launcher panel
    Column {
        Repeater {
            model: ApplicationManager

            Image {
                source: icon
                opacity: isRunning ? 0.3 : 1.0

                MouseArea {
                    anchors.fill: parent
                    onClicked: isRunning ? application.stop() : application.start("documentUrl");
                }
            }
        }
    }

    // System UI chrome for applications
    Repeater {
        model: ListModel { id: topLevelWindowsModel }

        delegate: Image {
            source: "chrome-bg.png"
            z: model.index

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Decoration: " + (model.window.application ? model.window.application.names["en"]
                                                                 : 'External Application')
            }

            MouseArea {
                anchors.fill: parent
                drag.target: parent
                onPressed: topLevelWindowsModel.move(model.index, topLevelWindowsModel.count - 1, 1);
            }

            Rectangle {
                width: 25; height: 25
                color: "chocolate"

                MouseArea {
                    anchors.fill: parent
                    onClicked: model.window.close();
                }
            }

            WindowItem {
                anchors.fill: parent
                anchors.margins: 3
                anchors.topMargin: 25
                window: model.window

                Connections {
                    target: window
                    function onContentStateChanged() {
                        if (window.contentState === WindowObject.NoSurface)
                            topLevelWindowsModel.remove(model.index, 1);
                    }
                }
            }

            Component.onCompleted: {
                x = 300 + model.index * 50;
                y =  10 + model.index * 30;
            }
        }
    }

    // System UI for a pop-up
    WindowItem {
        id: popUpContainer
        z: 9998
        width: 200; height: 60
        anchors.centerIn: parent

        Connections {
            target: popUpContainer.window
            function onContentStateChanged() {
                if (popUpContainer.window.contentState === WindowObject.NoSurface) {
                    popUpContainer.window = null;
                }
            }
        }
    }

    // System UI for a notification
    Text {
        z: 9999
        font.pixelSize: 46
        anchors.centerIn: parent
        text: NotificationManager.count > 0 ? NotificationManager.get(0).summary : ""
    }

    // Handler for WindowManager signals
    Connections {
        target: WindowManager
        function onWindowAdded(window) {
            if (window.windowProperty("type") === "pop-up") {
                popUpContainer.window = window;
            } else {
                topLevelWindowsModel.append({"window": window});
                window.setWindowProperty("propA", 42);
            }
        }

        function onWindowPropertyChanged(window, name, value) {
            console.log("SystemUI: OnWindowPropertyChanged [" + window + "] - " + name + ": " + value);
        }
    }
}
