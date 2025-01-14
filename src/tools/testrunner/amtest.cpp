// Copyright (C) 2021 The Qt Company Ltd.
// Copyright (C) 2020 Luxoft Sweden AB
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QPointer>
#include <QDir>
#include <QRegularExpression>
#include <QAbstractEventDispatcher>
#if defined(Q_OS_LINUX)
#  include <QTextStream>
#  include <QProcess>
#endif
#include <private/qtestlog_p.h>
#include "amtest.h"
#include "utilities.h"


QT_BEGIN_NAMESPACE_AM

AmTest::AmTest()
{}

AmTest *AmTest::instance()
{
    static QPointer<AmTest> object = new AmTest;
    if (!object) {
        qWarning("A new appman test object has been created, the behavior may be compromised");
        object = new AmTest;
    }
    return object;
}

int AmTest::timeoutFactor() const
{
    return QT_PREPEND_NAMESPACE_AM(timeoutFactor)();
}

static QtMsgType convertMsgType(AmTest::MsgType type)
{
    QtMsgType ret;

    switch (type) {
    case AmTest::WarningMsg: ret = QtWarningMsg; break;
    case AmTest::CriticalMsg: ret = QtCriticalMsg; break;
    case AmTest::FatalMsg: ret = QtFatalMsg; break;
    case AmTest::InfoMsg: ret = QtInfoMsg; break;
    default: ret = QtDebugMsg;
    }
    return ret;
}

void AmTest::ignoreMessage(MsgType type, const char *msg)
{
    QTestLog::ignoreMessage(convertMsgType(type), msg);
}

void AmTest::ignoreMessage(MsgType type, const QRegularExpression &expression)
{
    QTestLog::ignoreMessage(convertMsgType(type), expression);
}

int AmTest::observeObjectDestroyed(QObject *obj)
{
    static int idx = 0;
    int index = idx++;

    connect(obj, &QObject::destroyed, [this, index] () {
        emit objectDestroyed(index);
    });

    return index;
}

void AmTest::aboutToBlock()
{
    emit QAbstractEventDispatcher::instance()->aboutToBlock();
}

bool AmTest::dirExists(const QString &dir)
{
    return QDir(dir).exists();
}

#if defined(Q_OS_LINUX)
QString AmTest::ps(int pid)
{
    QProcess process;
    process.start(qSL("ps"), QStringList{qSL("--no-headers"), QString::number(pid)});
    QString str;
    if (process.waitForFinished(5000 * timeoutFactor()))
        str = QString::fromLocal8Bit(process.readAllStandardOutput().trimmed());
    return str;
}

QString AmTest::cmdLine(int pid)
{
    QFile file(QString::fromUtf8("/proc/%1/cmdline").arg(pid));
    QString str;
    if (file.open(QFile::ReadOnly)) {
        str = QString::fromLocal8Bit(file.readLine().trimmed());
        int nullPos = str.indexOf(QChar::Null);
        if (nullPos >= 0)
            str.truncate(nullPos);
    }
    return str;
}

QString AmTest::environment(int pid)
{
    QFile file(QString::fromUtf8("/proc/%1/environ").arg(pid));
    return file.open(QFile::ReadOnly) ? QTextStream(&file).readAll() : QString();
}

int AmTest::findChildProcess(int ppid, const QString &substr)
{
    QProcess process;
    process.start(qSL("ps"), QStringList{qSL("--ppid"), QString::number(ppid), qSL("-o"),
                                         qSL("pid,args"), qSL("--no-headers")});
    if (process.waitForFinished(5000 * timeoutFactor())) {
        const QString str = QString::fromLocal8Bit(process.readAllStandardOutput());
        QRegularExpression re(qSL(" *(\\d*) .*") + substr);
        QRegularExpressionMatch match = re.match(str);
        if (match.hasMatch())
            return match.captured(1).toInt();
    }
    return 0;
}
#endif  // Q_OS_LINIX

QT_END_NAMESPACE_AM

#include "moc_amtest.cpp"
