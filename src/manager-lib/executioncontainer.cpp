/****************************************************************************
**
** Copyright (C) 2015 Pelagicore AG
** Contact: http://www.pelagicore.com/
**
** This file is part of the Pelagicore Application Manager.
**
** SPDX-License-Identifier: GPL-3.0
**
** $QT_BEGIN_LICENSE:GPL3$
** Commercial License Usage
** Licensees holding valid commercial Pelagicore Application Manager
** licenses may use this file in accordance with the commercial license
** agreement provided with the Software or, alternatively, in accordance
** with the terms contained in a written agreement between you and
** Pelagicore. For licensing terms and conditions, contact us at:
** http://www.pelagicore.com.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3 requirements will be
** met: http://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef Q_OS_WIN32
#  include <signal.h>
#endif

#include "application.h"
#include "executioncontainerfactory.h"
#include "executioncontainer.h"


QProcess::ProcessState ExecutionContainerProcess::state()
{
    return m_state;
}

void ExecutionContainerProcess::kill()
{
#ifndef Q_OS_WIN32
    ::kill(pid(), SIGKILL);
#endif
}

void ExecutionContainerProcess::terminate()
{
#ifndef Q_OS_WIN32
    ::kill(pid(), SIGTERM);
#endif
}

Q_PID ExecutionContainerProcess::pid()
{
    return m_pid;
}

void ExecutionContainerProcess::setFinished(int returnCode, QProcess::ExitStatus status)
{
    m_state = QProcess::ProcessState::NotRunning;
    emit finished(returnCode, status);
}


ExecutionContainer::~ExecutionContainer()
{
}

bool ExecutionContainer::init()
{
    return true;
}