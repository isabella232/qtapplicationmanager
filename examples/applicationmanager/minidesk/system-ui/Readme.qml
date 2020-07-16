/****************************************************************************
**
** Copyright (C) 2019 Luxoft Sweden AB
** Copyright (C) 2018 Pelagicore AG
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Application Manager.
**
** $QT_BEGIN_LICENSE:BSD-QTAS$
** Commercial License Usage
** Licensees holding valid commercial Qt Automotive Suite licenses may use
** this file in accordance with the commercial license agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and The Qt Company.  For
** licensing terms and conditions see https://www.qt.io/terms-conditions.
** For further information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
** SPDX-License-Identifier: BSD-3-Clause
**
****************************************************************************/

import QtQuick 2.4

Item {
    anchors.fill: parent

    Column {
        width: paragraph.implicitWidth
        height: heading.implicitHeight + paragraph.implicitHeight + 80
        anchors.centerIn: parent
        spacing: 10

        Text {
            id: heading
            color: "cornflowerblue"
            font { pixelSize: 20; weight: Font.Bold }
            text: "Minimal Desktop System UI in pure QML"
        }

        Text {
            id: paragraph
            color: "grey"
            font.pixelSize: 16
            text: "The following features are supported:\n" +
                  "\u2022 Start applications by clicking on an icon in the top left\n" +
                  "\u2022 Stop an application by clicking on the icon in the top left again\n" +
                  "\u2022 Close application windows by clicking on the top left window decoration rectangle\n" +
                  "\u2022 Bring applications forward by clicking on the decoration\n" +
                  "\u2022 Drag windows by pressing on the window decoration and moving them\n" +
                  "\u2022 The System UI sends a 'propA' change when an app starts\n" +
                  "\u2022 The System UI and App2 react to window property changes with a debug message\n" +
                  "\u2022 Stop or restart App1 animations with a click\n" +
                  "\u2022 App1 sends rotation angle as a window property to System UI on stop\n" +
                  "\u2022 App1 shows a pop-up on the System UI while it is paused\n" +
                  "\u2022 App2 makes use of an IPC extension when it starts\n" +
                  "\u2022 App2 logs the document URL that started it\n" +
                  "\u2022 App2 triggers a notification in the System UI when the bulb icon is clicked\n" +
                  "\u2022 Show Wayland client windows that originate from processes outside of appman"
        }
    }
}
