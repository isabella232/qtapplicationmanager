// Copyright (C) 2021 The Qt Company Ltd.
// Copyright (C) 2019 Luxoft Sweden AB
// Copyright (C) 2018 Pelagicore AG
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick 2.4
import QtApplicationManager.Application 2.0

ApplicationManagerWindow {
    id: root
    color: "green"

    Text {
        anchors.fill: parent
        text: "This application consumes a lot of memory."
        font.pixelSize: 15
        color: "white"
    }

    Rectangle {
        id: rectangle
        width: Math.min(parent.width, parent.height) / 2
        height: width
        anchors.centerIn: parent
        color: "grey"
        Timer {
            interval: 10
            repeat: true
            running: true
            onTriggered: {
                rectangle.rotation += 1;
                root.contentItem.grabToImage(function(result) {
                    foo.push(result);
                });
            }
            property var foo: []
        }
    }
}
