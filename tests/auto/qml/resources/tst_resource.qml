/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Copyright (C) 2019 Luxoft Sweden AB
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtApplicationManager module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.11
import QtTest 1.0
import QtApplicationManager.SystemUI 2.0
import widgets 1.0

TestCase {
    id: testCase
    when: windowShown
    name: "ResourceTest"

    RedRect {}
    MagentaRect {}

    SignalSpy {
        id: runStateChangedSpy
        target: ApplicationManager
        signalName: "applicationRunStateChanged"
    }

    SignalSpy {
        id: windowPropertyChangedSpy
        target: WindowManager
        signalName: "windowPropertyChanged"
    }

    function test_basic_data() {
        return [ { tag: "app1" },
                 { tag: "app2" } ];
    }

    function test_basic(data) {
        wait(1200);    // wait for quicklaunch

        var app = ApplicationManager.application(data.tag);
        windowPropertyChangedSpy.clear();

        app.start();
        while (app.runState !== ApplicationObject.Running)
            runStateChangedSpy.wait(3000);

        if (data.tag === "app2") {
            windowPropertyChangedSpy.wait(2000);
            compare(windowPropertyChangedSpy.count, 1);
            compare(windowPropertyChangedSpy.signalArguments[0][0].windowProperty("meaning"), 42);
        }

        app.stop();
        while (app.runState !== ApplicationObject.NotRunning)
            runStateChangedSpy.wait(3000);
    }
}
