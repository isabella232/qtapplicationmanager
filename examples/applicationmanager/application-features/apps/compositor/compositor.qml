// Copyright (C) 2021 The Qt Company Ltd.
// Copyright (C) 2019 Luxoft Sweden AB
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick 2.11
import QtApplicationManager.Application 2.0
import QtWayland.Compositor 1.3
import QtWayland.Compositor.XdgShell
import QtWayland.Compositor.WlShell

ApplicationManagerWindow {
    id: root
    color: "lightgrey"

    property ListModel shellSurfaces: ListModel {}

    Text {
        anchors.fill: parent
        anchors.margins: 8
        font.pointSize: 14
        wrapMode: Text.Wrap
        textFormat: Text.RichText
        text: "This Wayland<sup>*</sup> client window implements a Wayland compositor (nested compositor). " +
              "To display Wayland clients here, set:<br><br><b>WAYLAND_DISPLAY=qtam-wayland-nested</b>" +
              "<br><br>For instance:<br>WAYLAND_DISPLAY=qtam-wayland-nested qml client.qml -platform wayland" +
              "<br><br><small>* in multi-process mode</small>"
    }

    WaylandCompositor {
        socketName: "qtam-wayland-nested"

        WaylandOutput {
            window: root
            sizeFollowsWindow: true
        }

        WlShell {
            onWlShellSurfaceCreated: (shellSurface) => shellSurfaces.append({shellSurface: shellSurface});
        }

        XdgShell {
            onToplevelCreated: (toplevel, xdgSurface) => shellSurfaces.append({shellSurface: xdgSurface});
        }
    }

    Repeater {
        model: shellSurfaces
        ShellSurfaceItem {
            shellSurface: modelData
            onSurfaceDestroyed: shellSurfaces.remove(index)
        }
    }

    Component.onCompleted: console.info("Start a client application in the nested compositor for instance with:\n  " +
                                        "WAYLAND_DISPLAY=qtam-wayland-nested QT_WAYLAND_DISABLE_WINDOWDECORATION=1 " +
                                        "QT_WAYLAND_SHELL_INTEGRATION=xdg-shell qml client.qml -platform wayland");
}
