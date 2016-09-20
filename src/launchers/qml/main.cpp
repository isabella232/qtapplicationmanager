/****************************************************************************
**
** Copyright (C) 2016 Pelagicore AG
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Pelagicore Application Manager.
**
** $QT_BEGIN_LICENSE:LGPL-QTAS$
** Commercial License Usage
** Licensees holding valid commercial Qt Automotive Suite licenses may use
** this file in accordance with the commercial license agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and The Qt Company.  For
** licensing terms and conditions see https://www.qt.io/terms-conditions.
** For further information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
** SPDX-License-Identifier: LGPL-3.0
**
****************************************************************************/


#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlIncubationController>

#include <QSocketNotifier>
#include <QFile>
#include <QDir>
#include <QtEndian>
#include <private/qcrashhandler_p.h>
#include <QRegularExpression>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusArgument>
#include <QLoggingCategory>

#include <qplatformdefs.h>

#if !defined(AM_HEADLESS)
#  include <QGuiApplication>
#  include <QQuickItem>
#  include <QQuickView>
#  include <QQuickWindow>

#  include "applicationmanagerwindow.h"
#else
#  include <QCoreApplication>
#endif

#include "qmlapplicationinterface.h"
#include "notification.h"
#include "qtyaml.h"
#include "global.h"
#include "utilities.h"

AM_BEGIN_NAMESPACE

// maybe make this configurable for specific workloads?
class HeadlessIncubationController : public QObject, public QQmlIncubationController
{
public:
    HeadlessIncubationController(QObject *parent)
        : QObject(parent)
    {
        startTimer(50);
    }

protected:
    void timerEvent(QTimerEvent *) override
    {
        incubateFor(25);
    }
};



// copied straight from Qt 5.1.0 qmlscene/main.cpp for now - needs to be revised
static void loadDummyDataFiles(QQmlEngine &engine, const QString& directory)
{
    QDir dir(directory + qSL("/dummydata"), qSL("*.qml"));
    QStringList list = dir.entryList();
    for (int i = 0; i < list.size(); ++i) {
        QString qml = list.at(i);
        QQmlComponent comp(&engine, dir.filePath(qml));
        QObject *dummyData = comp.create();

        if (comp.isError()) {
            QList<QQmlError> errors = comp.errors();
            foreach (const QQmlError &error, errors)
                qWarning() << error;
        }

        if (dummyData) {
            qml.truncate(qml.length()-4);
            engine.rootContext()->setContextProperty(qml, dummyData);
            dummyData->setParent(&engine);
        }
    }
}

class Controller : public QObject
{
    Q_OBJECT

public:
    Controller();

public slots:
    void startApplication(const QString &baseDir, const QString &qmlFile, const QString &document, const QVariantMap &runtimeParameters);

private:
    QQmlApplicationEngine m_engine;
    QQmlComponent *m_component;
    QmlApplicationInterface *m_applicationInterface = 0;
    QVariantMap m_configuration;
    bool m_launched = false;
#if !defined(AM_HEADLESS)
    QQuickWindow *m_window = 0;
#endif
};

AM_END_NAMESPACE

AM_USE_NAMESPACE

int main(int argc, char *argv[])
{
    qInstallMessageHandler(colorLogToStderr);
    QLoggingCategory::setFilterRules(QString::fromUtf8(qgetenv("AM_LOGGING_RULES")));

#if defined(AM_HEADLESS)
    QCoreApplication a(argc, argv);
#else
#  if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    // this is needed for WebEngine
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
#  endif

    QGuiApplication a(argc, argv);

    qmlRegisterType<ApplicationManagerWindow>("QtApplicationManager", 1, 0, "ApplicationManagerWindow");
#endif

    qmlRegisterType<QmlNotification>("QtApplicationManager", 1, 0, "Notification");
    qmlRegisterType<QmlApplicationInterfaceExtension>("QtApplicationManager", 1, 0, "ApplicationInterfaceExtension");

    QByteArray dbusAddress = qgetenv("AM_DBUS_PEER_ADDRESS");
    if (dbusAddress.isEmpty()) {
        qCCritical(LogQmlRuntime) << "ERROR: $AM_DBUS_PEER_ADDRESS is empty";
        return 2;
    }
    QDBusConnection dbusConnection = QDBusConnection::connectToPeer(QString::fromUtf8(dbusAddress), qSL("am"));

    if (!dbusConnection.isConnected()) {
        qCCritical(LogQmlRuntime) << "ERROR: could not connect to the application manager's peer D-Bus at" << dbusAddress;
        return 3;
    }

    Controller controller;
    return a.exec();
}

Controller::Controller()
    : m_component(new QQmlComponent(&m_engine))
{
    connect(&m_engine, &QObject::destroyed, &QCoreApplication::quit);
    connect(&m_engine, &QQmlEngine::quit, &QCoreApplication::quit);

    auto docs = QtYaml::variantDocumentsFromYaml(qgetenv("AM_RUNTIME_CONFIGURATION"));
    if (docs.size() == 1)
        m_configuration = docs.first().toMap();

    setCrashActionConfiguration(m_configuration.value("crashAction").toMap());

    QVariantMap config;
    auto additionalConfigurations = QtYaml::variantDocumentsFromYaml(qgetenv("AM_RUNTIME_ADDITIONAL_CONFIGURATION"));
    if (additionalConfigurations.size() == 1)
        config = additionalConfigurations.first().toMap();

    //qCDebug(LogQmlRuntime, 1) << " qml-runtime started with pid ==" << QCoreApplication::applicationPid () << ", waiting for qmlFile on stdin...";

    m_applicationInterface = new QmlApplicationInterface(config, qSL("am"), this);
    connect(m_applicationInterface, &QmlApplicationInterface::startApplication,
            this, &Controller::startApplication);
    if (!m_applicationInterface->initialize()) {
        qCritical("ERROR: could not connect to the application manager's interface on the peer D-Bus");
        qApp->exit(4);
    }
}

void Controller::startApplication(const QString &baseDir, const QString &qmlFile, const QString &document, const QVariantMap &runtimeParameters)
{
    if (m_launched)
        return;
    m_launched = true;

    qCDebug(LogQmlRuntime) << "loading" << qmlFile << "- document:" << document << "- parameters:"
                           << runtimeParameters << "- baseDir:" << baseDir;

    if (!QDir::setCurrent(baseDir)) {
        qCCritical(LogQmlRuntime) << "could not set the current directory to" << baseDir;
        QCoreApplication::exit(2);
        return;
    }

    if (!QFile::exists(qmlFile)) {
        qCCritical(LogQmlRuntime) << "could not load" << qmlFile << ": file does not exist";
        QCoreApplication::exit(2);
        return;
    }

    bool loadDummyData = runtimeParameters.value(qSL("loadDummyData")).toBool()
            || m_configuration.value(qSL("loadDummydata")).toBool();

    if (loadDummyData) {
        qCDebug(LogQmlRuntime) << "loading dummy-data";
        loadDummyDataFiles(m_engine, QFileInfo(qmlFile).path());
    }

    QStringList importPaths = m_configuration.value(qSL("importPaths")).toStringList();
    const QString prefix = QString::fromLocal8Bit(qgetenv("AM_BASE_DIR") + "/");
    for (QString &path : importPaths) {
        if (QFileInfo(path).isRelative()) {
            path.prepend(prefix);
        } else {
            qCWarning(LogQmlRuntime) << "Absolute import path in config file might lead to problems inside containers:"
                                     << path;
        }
    }

    auto vl = qdbus_cast<QVariantList>(runtimeParameters.value(qSL("importPaths")));
    for (const QVariant &v : qAsConst(vl)) {
        const QString path = v.toString();
        if (QFileInfo(path).isRelative())
            importPaths.append(QDir().absoluteFilePath(path));
        else
            qCWarning(LogQmlRuntime) << "Omitting absolute import path in info file for safety reasons:" << path;
    }

    if (!importPaths.isEmpty())
        qCDebug(LogQmlRuntime) << "setting QML2_IMPORT_PATH to" << importPaths;

    m_engine.setImportPathList(m_engine.importPathList() + importPaths);
    //qWarning() << m_engine.importPathList();

    m_engine.rootContext()->setContextProperty(qSL("ApplicationInterface"), m_applicationInterface);

    // This is a bit of a hack to make ApplicationManagerWindow known as a sub-class
    // of QWindow. Without this, assigning an ApplicationManagerWindow to a QWindow*
    // property will fail with [unknown property type]. First seen when trying to
    // write a nested Wayland-compositor where assigning to WaylandOutput.window failed.
    {
        static const char registerWindowQml[] = "import QtQuick 2.0\nimport QtQuick.Window 2.2\nQtObject { }\n";
        QQmlComponent registerWindowComp(&m_engine);
        registerWindowComp.setData(QByteArray::fromRawData(registerWindowQml, sizeof(registerWindowQml) - 1), QUrl());
        QScopedPointer<QObject> dummy(registerWindowComp.create());
        registerWindowComp.completeCreate();
    }

    QUrl qmlFileUrl = QUrl::fromLocalFile(qmlFile);
    m_engine.load(qmlFileUrl);

    QObject *topLevel = m_engine.rootObjects().at(0);

    if (!topLevel) {
        qCCritical(LogSystem) << "could not load" << qmlFile << ": no root object";
        QCoreApplication::exit(3);
        return;
    }

#if !defined(AM_HEADLESS)
    m_window = qobject_cast<QQuickWindow *>(topLevel);
    if (!m_window) {
        QQuickItem *contentItem = qobject_cast<QQuickItem *>(topLevel);
        if (contentItem) {
            QQuickView* view = new QQuickView(&m_engine, 0);
            m_window = view;
            view->setContent(qmlFileUrl, 0, topLevel);
        } else {
            qCCritical(LogSystem) << "could not load" << qmlFile << ": root object is not a QQuickItem.";
            QCoreApplication::exit(4);
            return;
        }
    } else {
        if (!m_engine.incubationController())
            m_engine.setIncubationController(m_window->incubationController());
    }

    Q_ASSERT(m_window);
    QObject::connect(&m_engine, &QQmlEngine::quit, m_window, &QObject::deleteLater); // not sure if this is needed .. or even the best thing to do ... see connects above, they seem to work better

    if (m_configuration.contains(qSL("backgroundColor"))) {
        QSurfaceFormat surfaceFormat = m_window->format();
        surfaceFormat.setAlphaBufferSize(8);
        m_window->setFormat(surfaceFormat);
        m_window->setClearBeforeRendering(true);
        m_window->setColor(QColor(m_configuration.value(qSL("backgroundColor")).toString()));
    }
    m_window->show();

#else
    m_engine.setIncubationController(new HeadlessIncubationController(&m_engine));
#endif
    qCDebug(LogQmlRuntime) << "component loading and creating complete.";

    if (!document.isEmpty())
        m_applicationInterface->openDocument(document);
}

#include "main.moc"
