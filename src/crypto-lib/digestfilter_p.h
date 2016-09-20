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

#pragma once

#include "digestfilter.h"

#if defined(AM_USE_LIBCRYPTO)
typedef struct env_md_ctx_st EVP_MD_CTX;
typedef struct hmac_ctx_st HMAC_CTX;

#elif defined(Q_OS_WIN)
#  if defined(_WIN64)
typedef unsigned __int64 ULONG_PTR;
#  else
typedef unsigned long ULONG_PTR;
#  endif
typedef ULONG_PTR HCRYPTPROV;
typedef ULONG_PTR HCRYPTHASH;
typedef ULONG_PTR HCRYPTKEY;

#elif defined(Q_OS_OSX)
#include <CommonCrypto/CommonHMAC.h>

#else
#  error "No DigestFilter backend available for this platform"
#endif

AM_BEGIN_NAMESPACE

class DigestFilterPrivate
{
public:
    DigestFilter *q;

    bool isHmac = false;
    bool initialized = false;
    DigestFilter::Type type = DigestFilter::Sha1;
    QByteArray key;
    QString errorString;

    bool init();
    void cleanup();
    qint64 process(const char *data, unsigned int size);
    qint64 finish(char *result, unsigned int *size);

private:
#if defined(AM_USE_LIBCRYPTO)
    // OpenSSL
    EVP_MD_CTX *mdCtx = nullptr;
    HMAC_CTX *hmacCtx = nullptr;

#elif defined(Q_OS_WIN)
    // Win: WinCrypt
    HCRYPTPROV cryptProvider = 0;
    HCRYPTHASH cryptHash = 0;
    HCRYPTKEY cryptKey = 0;

#elif defined(Q_OS_OSX)
    // OSX: SecurityFramework
    // The CC API is a bit weird regarding the different hashing context types. We use a union here,
    // since only one of these pointers is used per instance - 'freePtr' is just syntactic sugar
    // to make it clear that we are freeing whatever may be allocated.
    union {
        CC_SHA1_CTX *sha1Ctx;
        CC_SHA256_CTX *sha256Ctx;
        CC_SHA512_CTX *sha512Ctx;
        CCHmacContext *hmacCtx;
        void *freePtr = nullptr;
    };
#endif
};

AM_END_NAMESPACE
