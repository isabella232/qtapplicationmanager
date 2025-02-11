// Copyright (C) 2021 The Qt Company Ltd.
// Copyright (C) 2019 Luxoft Sweden AB
// Copyright (C) 2018 Pelagicore AG
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "startupinterface.h"

StartupInterface::~StartupInterface() { }


/*! \class StartupInterface
    \inmodule QtApplicationManager
    \brief An interface that allows to implement custom startup activities.

    This interface provides hooks that are called during the startup of application manager
    processes. Hence, implementers of the interface can run their custom code at certain points
    during the startup phase.

    A plugin has to implemet the pure virtual functions of the StartupInterface. The interface is
    the same for the System UI (appman), as well as for QML applications (appman-launcher-qml). The
    plugins that should be loaded have to be specified in the (am-config.yaml) configuration file.
    The following snippet shows how the application manager can be configured to load and execute
    the \c libappmanplugin.so in both the System UI and applications and additionally the
    \c libextplugin.so in the System UI only:

    \badcode
    # System UI
    plugins:
      startup: [ "path/to/libappmanplugin.so", "path/to/libextplugin.so" ]

    # Applications
    runtimes:
      qml:
        plugins:
          startup: "path/to/libappmanplugin.so"
    \endcode

    The functions are called in the following order:
    \list
    \li \l{StartupInterface::initialize}{initialize}
    \li afterRuntimeRegistration (optional)
    \li beforeQmlEngineLoad
    \li afterQmlEngineLoad
    \li beforeWindowShow
    \li afterWindowShow (optional)
    \endlist
*/

/*! \fn void StartupInterface::initialize(const QVariantMap &systemProperties)

    This method is called right after the \l{system properties}{\a systemProperties} have been parsed.
*/

/*! \fn void StartupInterface::afterRuntimeRegistration()

    This method is called, right after the runtime has been registered.
    \note It will only be called in the System UI process.
*/

/*! \fn void StartupInterface::beforeQmlEngineLoad(QQmlEngine *engine)

    This method is called, before the QML \a engine is loaded.

    \note All \c QtApplicationManager* QML namespaces are locked for new registrations via
          qmlProtectModule() after this call.
*/

/*! \fn void StartupInterface::afterQmlEngineLoad(QQmlEngine *engine)

    This method is called, after the QML \a engine has been loaded.
*/

/*! \fn void StartupInterface::beforeWindowShow(QWindow *window)

    This method is called, after the main window has been instantiated, but before it is shown. The
    \a window parameter holds a pointer to the main window.
    \note The \a window is only valid, if the root object of your QML tree is a visible item (e.g.
    a \l{Window} or \l{Item} derived class). It will be a \c nullptr, if it is a QtObject for
    instance.
*/

/*! \fn void StartupInterface::afterWindowShow(QWindow *window)

    This method is called, right after the main window is shown. The \a window parameter holds a
    pointer to the just shown main window.
    \note This function will only be called, if the root object of your QML tree is a visible item
    (e.g. a \l{Window} or \l{Item} derived class). It will not be called, if it is a QtObject for
    instance.
*/
