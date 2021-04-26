/****************************************************************************
**
** Copyright (C) 2019 Luxoft Sweden AB
** Copyright (C) 2018 Pelagicore AG
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Luxoft Application Manager.
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

#include <QCoreApplication>
#include <QDir>
#include <QUuid>

#include "logging.h"
#include "application.h"
#include "applicationinstaller.h"
#include "applicationinstaller_p.h"
#include "installationtask.h"
#include "deinstallationtask.h"
#include "sudo.h"
#include "utilities.h"
#include "exception.h"
#include "global.h"
#include "qml-utilities.h"
#include "applicationmanager.h"


/*!
    \qmltype ApplicationInstaller
    \inqmlmodule QtApplicationManager.SystemUI
    \ingroup system-ui-singletons
    \brief The package installation/removal/update part of the application-manager.

    The ApplicationInstaller singleton type handles the package installation
    part of the application manager. It provides both a DBus and QML APIs for
    all of its functionality.

    \note The ApplicationInstaller singleton and its corresponding DBus API are only available if you
          specify a base directory for installed application manifests. See \l{Configuration} for details.

    \target TaskStates

    The following table describes all possible states that a background task could be in:

    \table
    \header
        \li Task State
        \li Description
    \row
        \li \c Queued
        \li The task was created and is now queued up for execution.
    \row
        \li \c Executing
        \li The task is being executed.
    \row
        \li \c Finished
        \li The task was executed successfully.
    \row
        \li \c Failed
        \li The task failed to execute successfully.
    \row
        \li \c AwaitingAcknowledge
        \li \e{Installation tasks only!} The task is currently halted, waiting for either
            acknowledgePackageInstallation() or cancelTask() to continue. See startPackageInstallation()
            for more information on the installation workflow.
    \row
        \li \c Installing
        \li \e{Installation tasks only!} The installation was acknowledged via acknowledgePackageInstallation()
            and the final installation phase is now running.
    \row
        \li \c CleaningUp
        \li \e{Installation tasks only!} The installation has finished, and previous installations as
            well as temporary files are being cleaned up.
    \endtable

    The normal workflow for tasks is: \c Queued \unicode{0x2192} \c Executing \unicode{0x2192} \c
    Finished. The task can enter the \c Failed state at any point though - either by being canceled via
    cancelTask() or simply by failing due to an error.

    Installation tasks are a bit more complex due to the acknowledgment: \c Queued \unicode{0x2192}
    \c Executing \unicode{0x2192} \c AwaitingAcknowledge (this state may be skipped if
    acknowledgePackageInstallation() was called already) \unicode{0x2192} \c Installing
    \unicode{0x2192} \c Cleanup \unicode{0x2192} \c Finished. Again, the task can fail at any point.
*/

// THIS IS MISSING AN EXAMPLE!

/*!
    \qmlsignal ApplicationInstaller::taskStateChanged(string taskId, string newState)

    This signal is emitted when the state of the task identified by \a taskId changes. The
    new state is supplied in the parameter \a newState.

    \sa taskState()
*/

/*!
    \qmlsignal ApplicationInstaller::taskStarted(string taskId)

    This signal is emitted when the task identified by \a taskId enters the \c Executing state.

    \sa taskStateChanged()
*/

/*!
    \qmlsignal ApplicationInstaller::taskFinished(string taskId)

    This signal is emitted when the task identified by \a taskId enters the \c Finished state.

    \sa taskStateChanged()
*/

/*!
    \qmlsignal ApplicationInstaller::taskFailed(string taskId)

    This signal is emitted when the task identified by \a taskId enters the \c Failed state.

    \sa taskStateChanged()
*/

/*!
    \qmlsignal ApplicationInstaller::taskRequestingInstallationAcknowledge(string taskId, object application, object packageExtraMetaData, object packageExtraSignedMetaData)

    This signal is emitted when the installation task identified by \a taskId has received enough
    meta-data to be able to emit this signal. The task may be in either \c Executing or \c
    AwaitingAcknowledge state.

    The contents of the package's manifest file are supplied via \a application as a JavaScript object.
    Please see the \l {ApplicationManager Roles}{role names} for the expected object fields.

    In addition, the package's extra meta-data (signed and unsinged) is also supplied via \a
    packageExtraMetaData and \a packageExtraSignedMetaData respectively as JavaScript objects.
    Both these objects are optional and need to be explicitly either populated during an
    application's packaging step or added by an intermediary app-store server.
    By default, both will just be empty.

    Following this signal, either cancelTask() or acknowledgePackageInstallation() has to be called
    for this \a taskId, to either cancel the installation or try to complete it.

    The ApplicationInstaller has two convenience functions to help the System-UI with verifying the
    meta-data: compareVersions() and, in case you are using reverse-DNS notation for application-ids,
    validateDnsName().

    \sa taskStateChanged(), startPackageInstallation()
*/

/*!
    \qmlsignal ApplicationInstaller::taskBlockingUntilInstallationAcknowledge(string taskId)

    This signal is emitted when the installation task identified by \a taskId cannot continue
    due to a missing acknowledgePackageInstallation() call for the task.

    \sa taskStateChanged(), acknowledgePackageInstallation()
*/

/*!
    \qmlsignal ApplicationInstaller::taskProgressChanged(string taskId, qreal progress)

    This signal is emitted whenever the task identified by \a taskId makes progress towards its
    completion. The \a progress is reported as a floating-point number ranging from \c 0.0 to \c 1.0.

    \sa taskStateChanged()
*/

QT_BEGIN_NAMESPACE_AM

ApplicationInstaller *ApplicationInstaller::s_instance = nullptr;

ApplicationInstaller::ApplicationInstaller(const QVector<InstallationLocation> &installationLocations,
                                           QDir *manifestDir, QDir *imageMountDir, const QString &hardwareId,
                                           QObject *parent)
    : QObject(parent)
    , d(new ApplicationInstallerPrivate())
{
    d->installationLocations = installationLocations;
    d->manifestDir.reset(manifestDir);
    d->imageMountDir.reset(imageMountDir);
    d->hardwareId = hardwareId;
}

ApplicationInstaller::~ApplicationInstaller()
{
    delete d;
    s_instance = nullptr;
}

ApplicationInstaller *ApplicationInstaller::createInstance(const QVector<InstallationLocation> &installationLocations,
                                                           const QString &manifestDirPath, const QString &imageMountDirPath,
                                                           const QString &hardwareId, QString *error)
{
    if (Q_UNLIKELY(s_instance))
        qFatal("ApplicationInstaller::createInstance() was called a second time.");

    qRegisterMetaType<AsynchronousTask *>();
    qRegisterMetaType<AsynchronousTask::TaskState>();

    QScopedPointer<QDir> manifestDir(new QDir(manifestDirPath));

    if (Q_UNLIKELY(!manifestDir->exists())) {
        if (error)
            *error = qL1S("ApplicationInstaller::createInstance() could not access the manifest directory ") + manifestDir->absolutePath();
        return nullptr;
    }

    QScopedPointer<QDir> imageMountDir;

    if (!imageMountDirPath.isEmpty())
        imageMountDir.reset(new QDir(imageMountDirPath));

    if (Q_UNLIKELY(!imageMountDir.isNull() && !imageMountDir->exists())) {
        if (error)
            *error = qL1S("ApplicationInstaller::createInstance() could not access the image-mount directory ")
                + imageMountDirPath;
        return nullptr;
    }

    qmlRegisterSingletonType<ApplicationInstaller>("QtApplicationManager.SystemUI", 2, 0, "ApplicationInstaller",
                                                   &ApplicationInstaller::instanceForQml);

    return s_instance = new ApplicationInstaller(installationLocations, manifestDir.take(), imageMountDir.take(),
            hardwareId, QCoreApplication::instance());
}

ApplicationInstaller *ApplicationInstaller::instance()
{
    if (!s_instance)
        qFatal("ApplicationInstaller::instance() was called before createInstance().");
    return s_instance;
}

QObject *ApplicationInstaller::instanceForQml(QQmlEngine *, QJSEngine *)
{
    QQmlEngine::setObjectOwnership(instance(), QQmlEngine::CppOwnership);
    return instance();
}

bool ApplicationInstaller::developmentMode() const
{
    return d->developmentMode;
}

void ApplicationInstaller::setDevelopmentMode(bool b)
{
    d->developmentMode = b;
}

bool ApplicationInstaller::allowInstallationOfUnsignedPackages() const
{
    return d->allowInstallationOfUnsignedPackages;
}

void ApplicationInstaller::setAllowInstallationOfUnsignedPackages(bool b)
{
    d->allowInstallationOfUnsignedPackages = b;
}

QString ApplicationInstaller::hardwareId() const
{
    return d->hardwareId;
}

bool ApplicationInstaller::isApplicationUserIdSeparationEnabled() const
{
    return d->userIdSeparation;
}

uint ApplicationInstaller::commonApplicationGroupId() const
{
    return d->commonGroupId;
}

bool ApplicationInstaller::enableApplicationUserIdSeparation(uint minUserId, uint maxUserId, uint commonGroupId)
{
    if (minUserId >= maxUserId || minUserId == uint(-1) || maxUserId == uint(-1))
        return false;
    d->userIdSeparation = true;
    d->minUserId = minUserId;
    d->maxUserId = maxUserId;
    d->commonGroupId = commonGroupId;
    return true;
}

uint ApplicationInstaller::findUnusedUserId() const Q_DECL_NOEXCEPT_EXPR(false)
{
    if (!isApplicationUserIdSeparationEnabled())
        return uint(-1);

    QVector<AbstractApplication *> apps = ApplicationManager::instance()->applications();

    for (uint uid = d->minUserId; uid <= d->maxUserId; ++uid) {
        bool match = false;
        for (AbstractApplication *app : qAsConst(apps)) {
            if (app->nonAliasedInfo()->uid() == uid) {
                match = true;
                break;
            }
        }
        if (!match)
            return uid;
    }
    throw Exception("could not find a free user-id for application separation in the range %1 to %2")
            .arg(d->minUserId).arg(d->maxUserId);
}

const QDir *ApplicationInstaller::manifestDirectory() const
{
    return d->manifestDir.get();
}

const QDir *ApplicationInstaller::applicationImageMountDirectory() const
{
    return d->imageMountDir.get();
}

QList<QByteArray> ApplicationInstaller::caCertificates() const
{
    return d->chainOfTrust;
}

void ApplicationInstaller::setCACertificates(const QList<QByteArray> &chainOfTrust)
{
    d->chainOfTrust = chainOfTrust;
}

// find mounts and loopbacks left-over from a previous instance and kill them
void ApplicationInstaller::cleanupMounts() const
{
    // nothing to do as we don't support mounting app images
    if (d->imageMountDir.isNull())
        return;

    QMultiMap<QString, QString> mountPoints = mountedDirectories();

    const QFileInfoList mounts = d->imageMountDir->entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for (const QFileInfo &fi : mounts) {
        QString path = fi.canonicalFilePath();
        QString device = mountPoints.value(path);

        if (!device.isEmpty()) {
            qCDebug(LogInstaller) << "cleanup: trying to unmount stale application mount" << path;

            if (!SudoClient::instance()->unmount(path)) {
                if (!SudoClient::instance()->unmount(path, true /*force*/))
                    throw Exception("failed to un-mount stale mount %1 on %2: %3")
                        .arg(device, path, SudoClient::instance()->lastError());
            }
            if (device.startsWith(qL1S("/dev/loop"))) {
                if (!SudoClient::instance()->detachLoopback(device)) {
                    qCWarning(LogInstaller) << "failed to detach stale loopback device" << device;
                    // we can still continue here
                }
            }
        }

        if (!SudoClient::instance()->removeRecursive(path))
            throw Exception(Error::IO, SudoClient::instance()->lastError());
    }
}

void ApplicationInstaller::cleanupBrokenInstallations() const Q_DECL_NOEXCEPT_EXPR(false)
{
    // 1. find mounts and loopbacks left-over from a previous instance and kill them
    cleanupMounts();

    // 2. Check that everything in the app-db is available
    //    -> if not, remove from app-db

    ApplicationManager *am = ApplicationManager::instance();

    // key: baseDirPath, value: subDirName/ or fileName
    QMultiMap<QString, QString> validPaths {
        { manifestDirectory()->absolutePath(), QString() }
    };
    for (const InstallationLocation &il : qAsConst(d->installationLocations)) {
        if (!il.isRemovable() || il.isMounted()) {
            validPaths.insert(il.documentPath(), QString());
            validPaths.insert(il.installationPath(), QString());
        }
    }

    const auto allApps = am->applications();
    for (AbstractApplication *app : allApps) {
        const InstallationReport *ir = app->nonAliasedInfo()->installationReport();
        if (ir) {
            const InstallationLocation &il = installationLocationFromId(ir->installationLocationId());

            bool valid = il.isValid();

            if (!valid)
                qCDebug(LogInstaller) << "cleanup: uninstalling" << app->id() << "- installationLocation is invalid";

            if (valid && (!il.isRemovable() || il.isMounted())) {
                QStringList checkDirs;
                QStringList checkFiles;

                checkDirs << manifestDirectory()->absoluteFilePath(app->id());
                checkFiles << manifestDirectory()->absoluteFilePath(app->id()) + qSL("/info.yaml");
                checkFiles << manifestDirectory()->absoluteFilePath(app->id()) + qSL("/installation-report.yaml");
                checkDirs << il.documentPath() + app->id();

                if (il.isRemovable())
                    checkFiles << il.installationPath() + app->id() + qSL(".appimg");
                else
                    checkDirs << il.installationPath() + app->id();

                for (const QString &checkFile : qAsConst(checkFiles)) {
                    QFileInfo fi(checkFile);
                    if (!fi.exists() || !fi.isFile() || !fi.isReadable()) {
                        valid = false;
                        qCDebug(LogInstaller) << "cleanup: uninstalling" << app->id() << "- file missing:" << checkFile;
                        break;
                    }
                }
                for (const QString &checkDir : checkDirs) {
                    QFileInfo fi(checkDir);
                    if (!fi.exists() || !fi.isDir() || !fi.isReadable()) {
                        valid = false;
                        qCDebug(LogInstaller) << "cleanup: uninstalling" << app->id() << "- directory missing:" << checkDir;
                        break;
                    }
                }

                if (valid) {
                    if (il.isRemovable())
                        validPaths.insertMulti(il.installationPath(), app->id() + qSL(".appimg"));
                    else
                        validPaths.insertMulti(il.installationPath(), app->id() + qL1C('/'));
                    validPaths.insertMulti(il.documentPath(), app->id() + qL1C('/'));
                    validPaths.insertMulti(manifestDirectory()->absolutePath(), app->id() + qL1C('/'));
                }
            }
            if (!valid) {
                if (am->startingApplicationRemoval(app->id())) {
                    if (am->finishedApplicationInstall(app->id()))
                        continue;
                }
                throw Exception(Error::Package, "could not remove broken installation of app %1 from database").arg(app->id());
            }
        }
    }

    // 3. Remove everything that is not referenced from the app-db

    for (auto it = validPaths.cbegin(); it != validPaths.cend(); ) {
        const QString currentDir = it.key();

        // collect all values for the unique key currentDir
        QVector<QString> validNames;
        for ( ; it != validPaths.cend() && it.key() == currentDir; ++it)
            validNames << it.value();

        const QFileInfoList &dirEntries = QDir(currentDir).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);

        // check if there is anything in the filesystem that is NOT listed in the validNames
        for (const QFileInfo &fi : dirEntries) {
            QString name = fi.fileName();
            if (fi.isDir())
                name.append(qL1C('/'));

            if ((!fi.isDir() && !fi.isFile()) || !validNames.contains(name)) {
                qCDebug(LogInstaller) << "cleanup: removing unreferenced inode" << name;

                if (!SudoClient::instance()->removeRecursive(fi.absoluteFilePath()))
                    throw Exception(Error::IO, "could not remove broken installation leftover %1 : %2").arg(fi.absoluteFilePath()).arg(SudoClient::instance()->lastError());
            }
        }
    }
}

QVector<InstallationLocation> ApplicationInstaller::installationLocations() const
{
    return d->installationLocations;
}


const InstallationLocation &ApplicationInstaller::defaultInstallationLocation() const
{
    static bool once = false;
    static int defaultIndex = -1;

    if (!once) {
        for (int i = 0; i < d->installationLocations.size(); ++i) {
            if (d->installationLocations.at(i).isDefault()) {
                defaultIndex = i;
                break;
            }
        }
        once = true;
    }
    return (defaultIndex < 0) ? d->invalidInstallationLocation : d->installationLocations.at(defaultIndex);
}

const InstallationLocation &ApplicationInstaller::installationLocationFromId(const QString &installationLocationId) const
{
    for (const InstallationLocation &il : d->installationLocations) {
        if (il.id() == installationLocationId)
            return il;
    }
    return d->invalidInstallationLocation;
}

const InstallationLocation &ApplicationInstaller::installationLocationFromApplication(const QString &id) const
{
    if (AbstractApplication *a = ApplicationManager::instance()->fromId(id)) {
        if (const InstallationReport *report = a->nonAliasedInfo()->installationReport())
            return installationLocationFromId(report->installationLocationId());
    }
    return d->invalidInstallationLocation;
}


/*!
   \qmlmethod list<string> ApplicationInstaller::installationLocationIds()

   Retuns a list of all known installation location ids. Calling getInstallationLocation() on one of
   the returned identifiers will yield specific information about the individual installation locations.
*/
QStringList ApplicationInstaller::installationLocationIds() const
{
    QStringList ids;
    ids.reserve(d->installationLocations.size());
    for (const InstallationLocation &il : d->installationLocations)
        ids << il.id();
    return ids;
}

/*!
   \qmlmethod string ApplicationInstaller::installationLocationIdFromApplication(string id)

   Returns the installation location id for the application identified by \a id. Returns
   an empty string in case the application is not installed.

   \sa installationLocationIds()
 */
QString ApplicationInstaller::installationLocationIdFromApplication(const QString &id) const
{
    const InstallationLocation &il = installationLocationFromApplication(id);
    return il.isValid() ? il.id() : QString();
}

/*!
    \qmlmethod object ApplicationInstaller::getInstallationLocation(string installationLocationId)

    Returns an object describing the installation location identified by \a installationLocationId in detail.

    The returned object has the following members:

    \table
    \header
        \li \c Name
        \li \c Type
        \li Description
    \row
        \li \c id
        \li \c string
        \li The installation location id that is used as the handle for all other ApplicationInstaller
            function calls. The \c id consists of the \c type and \c index field, concatenated by
            a single dash (for example, \c internal-0).
    \row
        \li \c type
        \li \c string
        \li The type of device this installation location is connected to. Valid values are \c
            internal (for any kind of built-in storage, e.g. flash), \c removable (for any kind of
            storage that is removable by the user, e.g. an SD card) and \c invalid.
    \row
        \li \c index
        \li \c int
        \li In case there is more than one installation location for the same type of device, this
            \c zero-based index is used for disambiguation. For example, two SD card slots will
            result in the ids \c removable-0 and \c removable-1. Otherwise, the index is always \c 0.
    \row
        \li \c isDefault
        \li \c bool

        \li Exactly one installation location is the default location which must be mounted and
            accessible at all times. This can be used by an UI application to get a sensible
            default for the installation location that it needs to pass to startPackageInstallation().
    \row
        \li \c isRemovable
        \li \c bool
        \li Indicates whether this installation location is on a removable media (for example, an SD
            card).
    \row
        \li \c isMounted
        \li \c bool
        \li The current mount status of this installation location: should always be \c true for
            non-removable media.
    \row
        \li \c installationPath
        \li \c string
        \li The absolute file-system path to the base directory under which applications are installed.
    \row
        \li \c installationDeviceSize
        \li \c int
        \li The size of the device holding \c installationPath in bytes. This field is only present if
            \c isMounted is \c true.
    \row
        \li \c installationDeviceFree
        \li \c int
        \li The amount of bytes available on the device holding \c installationPath. This field is only
            present if \c isMounted is \c true.
    \row
        \li \c documentPath
        \li \c string
        \li The absolute file-system path to the base directory under which per-user document
            directories are created.
    \row
        \li \c documentDeviceSize
        \li \c int
        \li The size of the device holding \c documentPath in bytes. This field is only present if
            \c isMounted is \c true.
    \row
        \li \c documentDeviceFree
        \li \c int
        \li The amount of bytes available on the device holding \c documentPath. This field is only
            present if \c isMounted is \c true.
    \endtable

    Returns an empty object in case the \a installationLocationId is not valid.
*/
QVariantMap ApplicationInstaller::getInstallationLocation(const QString &installationLocationId) const
{
    const InstallationLocation &il = installationLocationFromId(installationLocationId);
    return il.isValid() ? il.toVariantMap() : QVariantMap();
}

/*!
   \qmlmethod int ApplicationInstaller::installedApplicationSize(string id)

   Returns the size in bytes that the application identified by \a id is occupying on the storage
   device.

   Returns \c -1 in case the application \a id is not valid, or the application is not installed.
*/
qint64 ApplicationInstaller::installedApplicationSize(const QString &id) const
{
    if (AbstractApplication *a = ApplicationManager::instance()->fromId(id)) {
        if (const InstallationReport *report = a->nonAliasedInfo()->installationReport())
            return static_cast<qint64>(report->diskSpaceUsed());
    }
    return -1;
}

/*!
   \qmlmethod var ApplicationInstaller::installedApplicationExtraMetaData(string id)

   Returns a map of all extra metadata in the package header of the application identified by \a id.

   Returns an empty map in case the application \a id is not valid, or the application is not installed.
*/
QVariantMap ApplicationInstaller::installedApplicationExtraMetaData(const QString &id) const
{
    if (AbstractApplication *a = ApplicationManager::instance()->fromId(id)) {
        if (const InstallationReport *report = a->nonAliasedInfo()->installationReport())
            return report->extraMetaData();
    }
    return QVariantMap();
}

/*!
   \qmlmethod var ApplicationInstaller::installedApplicationExtraSignedMetaData(string id)

   Returns a map of all signed extra metadata in the package header of the application identified
   by \a id.

   Returns an empty map in case the application \a id is not valid, or the application is not installed.
*/
QVariantMap ApplicationInstaller::installedApplicationExtraSignedMetaData(const QString &id) const
{
    if (AbstractApplication *a = ApplicationManager::instance()->fromId(id)) {
        if (const InstallationReport *report = a->nonAliasedInfo()->installationReport())
            return report->extraSignedMetaData();
    }
    return QVariantMap();
}

/*! \internal
  Type safe convenience function, since DBus does not like QUrl
*/
QString ApplicationInstaller::startPackageInstallation(const QString &installationLocationId, const QUrl &sourceUrl)
{
    AM_TRACE(LogInstaller, installationLocationId, sourceUrl);

    const InstallationLocation &il = installationLocationFromId(installationLocationId);

    return enqueueTask(new InstallationTask(il, sourceUrl));
}

/*!
    \qmlmethod string ApplicationInstaller::startPackageInstallation(string installationLocationId, string sourceUrl)

    Downloads an application package from \a sourceUrl and installs it to the installation location
    described by \a installationLocationId.

    The actual download and installation will happen asynchronously in the background. The
    ApplicationInstaller emits the signals \l taskStarted, \l taskProgressChanged, \l
    taskRequestingInstallationAcknowledge, \l taskFinished, \l taskFailed, and \l taskStateChanged
    for the returned taskId when applicable.

    \note Simply calling this function is not enough to complete a package installation: The
    taskRequestingInstallationAcknowledge() signal needs to be connected to a slot where the
    supplied application meta-data can be validated (either programmatically or by asking the user).
    If the validation is successful, the installation can be completed by calling
    acknowledgePackageInstallation() or, if the validation was unsuccessful, the installation should
    be canceled by calling cancelTask().
    Failing to do one or the other will leave an unfinished "zombie" installation.

    Returns a unique \c taskId. This can also be an empty string, if the task could not be
    created (in this case, no signals will be emitted).
*/
QString ApplicationInstaller::startPackageInstallation(const QString &installationLocationId, const QString &sourceUrl)
{
    QUrl url(sourceUrl);
    if (url.scheme().isEmpty())
        url = QUrl::fromLocalFile(sourceUrl);
    return startPackageInstallation(installationLocationId, url);
}

/*!
    \qmlmethod void ApplicationInstaller::acknowledgePackageInstallation(string taskId)

    Calling this function enables the installer to complete the installation task identified by \a
    taskId. Normally, this function is called after receiving the taskRequestingInstallationAcknowledge()
    signal, and the user and/or the program logic decided to proceed with the installation.

    \sa startPackageInstallation()
 */
void ApplicationInstaller::acknowledgePackageInstallation(const QString &taskId)
{
    AM_TRACE(LogInstaller, taskId)

    const auto allTasks = d->allTasks();

    for (AsynchronousTask *task : allTasks) {
        if (qobject_cast<InstallationTask *>(task) && (task->id() == taskId)) {
            static_cast<InstallationTask *>(task)->acknowledge();
            break;
        }
    }
}

/*!
    \qmlmethod string ApplicationInstaller::removePackage(string id, bool keepDocuments, bool force)

    Uninstalls the application identified by \a id. Normally, the documents directory of the
    application is deleted on removal, but this can be prevented by setting \a keepDocuments to \c true.

    The actual removal will happen asynchronously in the background. The ApplicationInstaller will
    emit the signals \l taskStarted, \l taskProgressChanged, \l taskFinished, \l taskFailed and \l
    taskStateChanged for the returned \c taskId when applicable.

    Normally, \a force should only be set to \c true if a previous call to removePackage() failed.
    This may be necessary if the installation process was interrupted, or if an SD card got lost
    or has file-system issues.

    Returns a unique \c taskId. This can also be an empty string, if the task could not be created
    (in this case, no signals will be emitted).
*/
QString ApplicationInstaller::removePackage(const QString &id, bool keepDocuments, bool force)
{
    AM_TRACE(LogInstaller, id, keepDocuments)

    if (AbstractApplication *a = ApplicationManager::instance()->fromId(id)) {
        if (const InstallationReport *report = a->nonAliasedInfo()->installationReport()) {
            const InstallationLocation &il = installationLocationFromId(report->installationLocationId());

            if (il.isValid() && (il.id() == report->installationLocationId()))
                return enqueueTask(new DeinstallationTask(id, il, force, keepDocuments));
        }
    }
    return QString();
}


/*!
    \qmlmethod enumeration ApplicationInstaller::taskState(string taskId)

    Returns the current state of the installation task identified by \a taskId.
    \l {TaskStates}{See here} for a list of valid task states.

    Returns \c ApplicationInstaller.Invalid if the \a taskId is invalid.
*/
AsynchronousTask::TaskState ApplicationInstaller::taskState(const QString &taskId) const
{
    const auto allTasks = d->allTasks();

    for (const AsynchronousTask *task : allTasks) {
        if (task && (task->id() == taskId))
            return task->state();
    }
    return AsynchronousTask::Invalid;
}

/*!
    \qmlmethod string ApplicationInstaller::taskApplicationId(string taskId)

    Returns the application id associated with the task identified by \a taskId. The task may not
    have a valid application id at all times though and in this case the function will return an
    empty string (this will be the case for installations before the taskRequestingInstallationAcknowledge
    signal has been emitted).

    Returns an empty string if the \a taskId is invalid.
*/
QString ApplicationInstaller::taskApplicationId(const QString &taskId) const
{
    const auto allTasks = d->allTasks();

    for (const AsynchronousTask *task : allTasks) {
        if (task && (task->id() == taskId))
            return task->applicationId();
    }
    return QString();
}

/*!
    \qmlmethod list<string> ApplicationInstaller::activeTaskIds()

    Retuns a list of all currently active (as in not yet finished or failed) installation task ids.
*/
QStringList ApplicationInstaller::activeTaskIds() const
{
    const auto allTasks = d->allTasks();

    QStringList result;
    for (const AsynchronousTask *task : allTasks)
        result << task->id();
    return result;
}

/*!
    \qmlmethod bool ApplicationInstaller::cancelTask(string taskId)

    Tries to cancel the installation task identified by \a taskId.

    Returns \c true if the task was canceled, \c false otherwise.
*/
bool ApplicationInstaller::cancelTask(const QString &taskId)
{
    AM_TRACE(LogInstaller, taskId)

    // incoming tasks can be forcefully cancelled right away
    for (AsynchronousTask *task : qAsConst(d->incomingTaskList)) {
        if (task->id() == taskId) {
            task->forceCancel();
            task->deleteLater();

            handleFailure(task);

            d->incomingTaskList.removeOne(task);
            triggerExecuteNextTask();
            return true;
        }
    }

    // the active task and async tasks might be in a state where cancellation is not possible,
    // so we have to ask them nicely
    if (d->activeTask && d->activeTask->id() == taskId)
        return d->activeTask->cancel();

    for (AsynchronousTask *task : qAsConst(d->installationTaskList)) {
        if (task->id() == taskId)
            return task->cancel();
    }
    return false;
}

/*!
    \qmlmethod int ApplicationInstaller::compareVersions(string version1, string version2)

    Convenience method for app-store implementations or taskRequestingInstallationAcknowledge()
    callbacks for comparing version numbers, as the actual version comparison algorithm is not
    trivial.

    Returns \c -1, \c 0 or \c 1 if \a version1 is smaller than, equal to, or greater than \a
    version2 (similar to how \c strcmp() works).
*/
int ApplicationInstaller::compareVersions(const QString &version1, const QString &version2)
{
    int pos1 = 0;
    int pos2 = 0;
    int len1 = version1.length();
    int len2 = version2.length();

    forever {
        if (pos1 == len1 && pos2 == len2)
            return 0;
       else if (pos1 >= len1)
            return -1;
        else if (pos2 >= len2)
            return +1;

        QString part1 = version1.mid(pos1++, 1);
        QString part2 = version2.mid(pos2++, 1);

        bool isDigit1 = part1.at(0).isDigit();
        bool isDigit2 = part2.at(0).isDigit();

        if (!isDigit1 || !isDigit2) {
            int cmp = part1.compare(part2);
            if (cmp)
                return (cmp > 0) ? 1 : -1;
        } else {
            while ((pos1 < len1) && version1.at(pos1).isDigit())
                part1.append(version1.at(pos1++));
            while ((pos2 < len2) && version2.at(pos2).isDigit())
                part2.append(version2.at(pos2++));

            int num1 = part1.toInt();
            int num2 = part2.toInt();

            if (num1 != num2)
                return (num1 > num2) ? 1 : -1;
        }
    }
}

/*!
    \qmlmethod int ApplicationInstaller::validateDnsName(string name, int minimalPartCount)

    Convenience method for app-store implementations or taskRequestingInstallationAcknowledge()
    callbacks for checking if the given \a name is a valid DNS (or reverse-DNS) name according to
    RFC 1035/1123. If the optional parameter \a minimalPartCount is specified, this function will
    also check if \a name contains at least this amount of parts/sub-domains.

    Returns \c true if the name is a valid DNS name or \c false otherwise.
*/
bool ApplicationInstaller::validateDnsName(const QString &name, int minimalPartCount)
{
    try {
        // check if we have enough parts: e.g. "tld.company.app" would have 3 parts
        QStringList parts = name.split('.');
        if (parts.size() < minimalPartCount) {
            throw Exception(Error::Parse, "the minimum amount of parts (subdomains) is %1 (found %2)")
                .arg(minimalPartCount).arg(parts.size());
        }

        // standard RFC compliance tests (RFC 1035/1123)

        auto partCheck = [](const QString &part) {
            int len = part.length();

            if (len < 1 || len > 63)
                throw Exception(Error::Parse, "domain parts must consist of at least 1 and at most 63 characters (found %2 characters)").arg(len);

            for (int pos = 0; pos < len; ++pos) {
                ushort ch = part.at(pos).unicode();
                bool isFirst = (pos == 0);
                bool isLast  = (pos == (len - 1));
                bool isDash  = (ch == '-');
                bool isDigit = (ch >= '0' && ch <= '9');
                bool isLower = (ch >= 'a' && ch <= 'z');

                if ((isFirst || isLast || !isDash) && !isDigit && !isLower)
                    throw Exception(Error::Parse, "domain parts must consist of only the characters '0-9', 'a-z', and '-' (which cannot be the first or last character)");
            }
        };

        for (const QString &part : parts)
            partCheck(part);

        return true;
    } catch (const Exception &e) {
        qCDebug(LogInstaller).noquote() << "validateDnsName failed:" << e.errorString();
        return false;
    }
}


// this is a simple helper class - we need this to be able to run the filesystem (un)mounting
// in a separate thread to avoid blocking the UI and D-Bus
class ActivationHelper : public QObject // clazy:exclude=missing-qobject-macro
{
public:
    enum Mode { Activate, Deactivate, IsActivated };

    static bool run(Mode mode, const QString &id)
    {
        if (!SudoClient::instance())
            return false;

        const QDir *imageMountDir = ApplicationInstaller::instance()->applicationImageMountDirectory();

        if (id.isEmpty() || !imageMountDir || !imageMountDir->exists())
            return false;

        const InstallationLocation &il = ApplicationInstaller::instance()->installationLocationFromApplication(id);
        if (!il.isValid() || !il.isRemovable())
            return false;

        QString imageName = il.installationPath() + id + qSL(".appimg");
        if (!QFile::exists(imageName))
            return false;

        QString mountPoint = imageMountDir->absoluteFilePath(id);
        QString mountedDevice;
        auto currentMounts = mountedDirectories();
        bool isMounted = currentMounts.contains(mountPoint);

        switch (mode) {

        case Activate:
            if (isMounted)
                return false;
            mountPoint.clear(); // not mounted yet
            break;
        case Deactivate:
            if (!isMounted)
                return false;
            mountedDevice = currentMounts.value(mountPoint);
            break;
        case IsActivated:
            return isMounted;
        }

        ActivationHelper *a = new ActivationHelper(id, imageName, *imageMountDir, mountPoint, mountedDevice);
        QThread *t = new QThread();
        a->moveToThread(t);
        connect(t, &QThread::started, a, [t, a, mode]() {
            a->m_status = (mode == Activate) ? a->mount()
                                             : a->unmount();
            t->quit();
        });
        connect(t, &QThread::finished, ApplicationInstaller::instance(), [id, t, a, mode]() {
            if (mode == Activate) {
                emit ApplicationInstaller::instance()->packageActivated(id, a->m_status);
                if (!a->m_status)
                    qCCritical(LogSystem) << "ERROR: failed to activate package" << id << ":" << a->m_errorString;
            } else {
                emit ApplicationInstaller::instance()->packageDeactivated(id, a->m_status);
                if (!a->m_status)
                    qCCritical(LogSystem) << "ERROR: failed to de-activate package" << id << ":" << a->m_errorString;
            }
            delete a;
            t->deleteLater();
        });
        t->start();
        return true;
    }

    ActivationHelper(const QString &id, const QString &imageName, const QDir &imageMountDir,
                     const QString &mountPoint, const QString &mountedDevice)
        : m_applicationId(id)
        , m_imageName(imageName)
        , m_imageMountDir(imageMountDir)
        , m_mountPoint(mountPoint)
        , m_mountedDevice(mountedDevice)
    { }

    bool mount()
    {
        SudoClient *root = SudoClient::instance();

        // m_mountPoint is only set, if the image is/was actually mounted
        QString mountDir = m_imageMountDir.filePath(m_applicationId);

        try {
            QFileInfo fi(mountDir);
            if (!fi.isDir() && !QDir(fi.absolutePath()).mkpath(fi.fileName()))
                throw Exception("could not create mountpoint directory %1").arg(mountDir);

            m_mountedDevice = root->attachLoopback(m_imageName, true /*ro*/);
            if (m_mountedDevice.isEmpty())
                throw Exception("could not create a new loopback device: %1").arg(root->lastError());

            if (!root->mount(m_mountedDevice, mountDir, true /*ro*/))
                throw Exception("could not mount application image %1 to %2: %3").arg(mountDir, m_mountedDevice, root->lastError());
            m_mountPoint = mountDir;

            // better be safe than sorry - make sure this is the exact same version we installed
            // (we cannot check every single file, but we at least make sure that info.yaml matches)
            QFile manifest1(ApplicationInstaller::instance()->manifestDirectory()->absoluteFilePath(m_applicationId + qSL("/info.yaml")));
            QFile manifest2(QDir(mountDir).absoluteFilePath(qSL("info.yaml")));

            if ((manifest1.size() != manifest2.size())
                    || (manifest1.size() > (16 * 1024))
                    || !manifest1.open(QFile::ReadOnly)
                    || !manifest2.open(QFile::ReadOnly)
                    || (manifest1.readAll() != manifest2.readAll())) {
                throw Exception("the info.yaml files in the manifest directory and within the application image do not match");
            }
            return true;
        } catch (const Exception &e) {
            unmount();
            m_errorCode = e.errorCode();
            m_errorString = e.errorString();
            return false;
        }
    }


    bool unmount()
    {
        SudoClient *root = SudoClient::instance();

        try {
            if (!m_mountPoint.isEmpty() && !root->unmount(m_mountPoint))
                throw Exception("could not unmount the application image at %1: %2").arg(m_mountPoint, root->lastError());

            if (!m_mountedDevice.isEmpty() && !root->detachLoopback(m_mountedDevice))
                throw Exception("could not remove loopback device %1: %2").arg(m_mountedDevice, root->lastError());

            // m_mountPoint is only set, if the image is/was actually mounted
            QString mountDir = m_imageMountDir.filePath(m_applicationId);
            if (QFileInfo(mountDir).isDir() && !m_imageMountDir.rmdir(m_applicationId))
                throw Exception("could not remove mount-point directory %1").arg(mountDir);

            return true;
        } catch (const Exception &e) {
            m_errorCode = e.errorCode();
            m_errorString = e.errorString();
            return false;
        }
    }

private:
    QString m_applicationId;
    QString m_imageName;
    QDir m_imageMountDir;
    QString m_mountPoint;
    QString m_mountedDevice;
    bool m_status = false;
    Error m_errorCode = Error::None;
    QString m_errorString;
};

bool ApplicationInstaller::doesPackageNeedActivation(const QString &id)
{
    const InstallationLocation &il = installationLocationFromApplication(id);
    return il.isValid() && il.isRemovable();
}

bool ApplicationInstaller::isPackageActivated(const QString &id)
{
    return ActivationHelper::run(ActivationHelper::IsActivated, id);
}

bool ApplicationInstaller::activatePackage(const QString &id)
{
    return ActivationHelper::run(ActivationHelper::Activate, id);
}

bool ApplicationInstaller::deactivatePackage(const QString &id)
{
    return ActivationHelper::run(ActivationHelper::Deactivate, id);
}


QString ApplicationInstaller::enqueueTask(AsynchronousTask *task)
{
    d->incomingTaskList.append(task);
    triggerExecuteNextTask();
    return task->id();
}

void ApplicationInstaller::triggerExecuteNextTask()
{
    if (!QMetaObject::invokeMethod(this, "executeNextTask", Qt::QueuedConnection))
        qCCritical(LogSystem) << "ERROR: failed to invoke method checkQueue";
}

void ApplicationInstaller::executeNextTask()
{
    if (d->activeTask || d->incomingTaskList.isEmpty())
        return;

    AsynchronousTask *task = d->incomingTaskList.takeFirst();

    if (task->hasFailed()) {
        task->setState(AsynchronousTask::Failed);

        handleFailure(task);

        task->deleteLater();
        triggerExecuteNextTask();
        return;
    }

    connect(task, &AsynchronousTask::started, this, [this, task]() {
        emit taskStarted(task->id());
    });

    connect(task, &AsynchronousTask::stateChanged, this, [this, task](AsynchronousTask::TaskState newState) {
        emit taskStateChanged(task->id(), newState);
    });

    connect(task, &AsynchronousTask::progress, this, [this, task](qreal p) {
        emit taskProgressChanged(task->id(), p);
        QMetaObject::invokeMethod(ApplicationManager::instance(),
                                  "progressingApplicationInstall",
                                  Qt::DirectConnection,
                                  Q_ARG(QString, task->applicationId()),
                                  Q_ARG(double, p));
    });

    connect(task, &AsynchronousTask::finished, this, [this, task]() {
        task->setState(task->hasFailed() ? AsynchronousTask::Failed : AsynchronousTask::Finished);

        if (task->hasFailed()) {
            handleFailure(task);
        } else {
            qCDebug(LogInstaller) << "emit finished" << task->id();
            emit taskFinished(task->id());
        }

        if (d->activeTask == task)
            d->activeTask = nullptr;
        d->installationTaskList.removeOne(task);

        delete task;
        triggerExecuteNextTask();
    });

    if (qobject_cast<InstallationTask *>(task)) {
        connect(static_cast<InstallationTask *>(task), &InstallationTask::finishedPackageExtraction, this, [this, task]() {
            qCDebug(LogInstaller) << "emit blockingUntilInstallationAcknowledge" << task->id();
            emit taskBlockingUntilInstallationAcknowledge(task->id());

            // we can now start the next download in parallel - the InstallationTask will take care
            // of serializing the final installation steps on its own as soon as it gets the
            // required acknowledge (or cancel).
            if (d->activeTask == task)
                d->activeTask = nullptr;
            d->installationTaskList.append(task);
            triggerExecuteNextTask();
        });
    }


    d->activeTask = task;
    task->setState(AsynchronousTask::Executing);
    task->start();
}

void ApplicationInstaller::handleFailure(AsynchronousTask *task)
{
    qCDebug(LogInstaller) << "emit failed" << task->id() << task->errorCode() << task->errorString();
    emit taskFailed(task->id(), int(task->errorCode()), task->errorString());
}


bool removeRecursiveHelper(const QString &path)
{
    if (ApplicationInstaller::instance()->isApplicationUserIdSeparationEnabled() && SudoClient::instance())
        return SudoClient::instance()->removeRecursive(path);
    else
        return recursiveOperation(path, safeRemove);
}

QT_END_NAMESPACE_AM
