// Copyright (C) 2021 The Qt Company Ltd.
// Copyright (C) 2019 Luxoft Sweden AB
// Copyright (C) 2018 Pelagicore AG
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.4
import QtApplicationManager.Application 2.0

ApplicationManagerWindow {
    id: root

    ApplicationManagerWindow {
        id: sub
        visible: false
        Component.onCompleted: setWindowProperty("type", "sub");
    }

    Connections {
        target: ApplicationInterface
        function onOpenDocument(documentUrl) {
            switch (documentUrl) {
            case "show-main": root.visible = true; root.setWindowProperty("key1", "val1"); break;
            case "hide-main": root.visible = false; break;
            case "show-sub": sub.visible = true; break;
            case "hide-sub": sub.visible = false; break;
            }
        }
    }

    Component.onCompleted: setWindowProperty("objectName", 42);
}
