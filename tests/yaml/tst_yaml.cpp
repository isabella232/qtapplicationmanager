/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Copyright (C) 2019 Luxoft Sweden AB
** Copyright (C) 2018 Pelagicore AG
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Luxoft Application Manager.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT-QTAS$
** Commercial License Usage
** Licensees holding valid commercial Qt Automotive Suite licenses may use
** this file in accordance with the commercial license agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and The Qt Company.  For
** licensing terms and conditions see https://www.qt.io/terms-conditions.
** For further information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore>
#include <QtTest>

#include "qtyaml.h"
#include "exception.h"
#include "global.h"

QT_USE_NAMESPACE_AM

class tst_Yaml : public QObject
{
    Q_OBJECT

public:
    tst_Yaml();

private slots:
    void parser();
    void documentParser();
};


tst_Yaml::tst_Yaml()
{ }

void tst_Yaml::parser()
{
    QVector<QPair<const char *, QVariant>> tests = {
        { "dec", QVariant::fromValue<int>(10) },
        { "hex", QVariant::fromValue<int>(16) },
        { "bin", QVariant::fromValue<int>(2) },
        { "oct", QVariant::fromValue<int>(8) },
        { "float1", QVariant::fromValue<double>(10.1) },
        { "float2", QVariant::fromValue<double>(.1) },
        { "float3", QVariant::fromValue<double>(.1) },
        { "number-separators", QVariant::fromValue<int>(1234567) },
        { "bool-true", QVariant::fromValue<bool>(true) },
        { "bool-yes", QVariant::fromValue<bool>(true) },
        { "bool-false", QVariant::fromValue<bool>(false) },
        { "bool-no", QVariant::fromValue<bool>(false) },
        { "null-literal", QVariant() },
        { "null-tilde", QVariant() },
        { "string-unquoted", QVariant::fromValue<QString>(qSL("unquoted")) },
        { "string-singlequoted", QVariant::fromValue<QString>(qSL("singlequoted")) },
        { "string-doublequoted", QVariant::fromValue<QString>(qSL("doublequoted")) },
        { "list-int", QVariantList { 1, 2, 3 } },
        { "list-mixed", QVariantList { 1, qSL("two"), QVariantList { true, QVariant { } } } },
        { "map1", QVariantMap { { "a", 1 }, { "b", "two" }, { "c", QVariantList { 1, 2, 3 } } } }
    };

    try {
        QFile f(":/data/test.yaml");
        QVERIFY2(f.open(QFile::ReadOnly), qPrintable(f.errorString()));
        QByteArray ba = f.readAll();
        QVERIFY(!ba.isEmpty());
        YamlParser p(ba);
        auto header = p.parseHeader();

        QCOMPARE(header.first, "testfile");
        QCOMPARE(header.second, 42);

        QVERIFY(p.nextDocument());

        YamlParser::Fields fields;
        for (const auto &pair : tests) {
            YamlParser::FieldType type = YamlParser::Scalar;
            if (pair.second.type() == QVariant::List)
                type = YamlParser::List;
            else if (pair.second.type() == QVariant::Map)
                type = YamlParser::Map;
            QVariant value = pair.second;

            fields.emplace_back(pair.first, true, type, [type, value](YamlParser *p) {
                switch (type) {
                case YamlParser::Scalar: {
                    QVERIFY(p->isScalar());
                    QVariant v = p->parseScalar();
                    QCOMPARE(v.type(), value.type());
                    QVERIFY(v == value);
                    break;
                }
                case YamlParser::List: {
                    QVERIFY(p->isList());
                    QVariantList vl = p->parseList();
                    QVERIFY(vl == value.toList());
                    break;
                }
                case YamlParser::Map: {
                    QVERIFY(p->isMap());
                    QVariantMap vm = p->parseMap();
                    QVERIFY(vm == value.toMap());
                    break;
                }
                }
            });
        }
        fields.emplace_back("extended", true, YamlParser::Map, [](YamlParser *p) {
            YamlParser::Fields extFields = {
                { "ext-string", true, YamlParser::Scalar, [](YamlParser *p) {
                      QVERIFY(p->isScalar());
                      QVariant v = p->parseScalar();
                      QCOMPARE(v.type(), QVariant::String);
                      QCOMPARE(v.toString(), qSL("ext string"));
                  } }
            };
            p->parseFields(extFields);
        });

        fields.emplace_back("stringlist-string", true, YamlParser::Scalar | YamlParser::List, [](YamlParser *p) {
            QCOMPARE(p->parseStringOrStringList(), QStringList { "string" });
        });
        fields.emplace_back("stringlist-list1", true, YamlParser::Scalar | YamlParser::List, [](YamlParser *p) {
            QCOMPARE(p->parseStringOrStringList(), QStringList { "string" });
        });
        fields.emplace_back("stringlist-list2", true, YamlParser::Scalar | YamlParser::List, [](YamlParser *p) {
            QCOMPARE(p->parseStringOrStringList(), QStringList({ "string1", "string2" }));
        });

        fields.emplace_back("list-of-maps", true, YamlParser::List, [](YamlParser *p) {
            int index = 0;
            p->parseList([&index](YamlParser *p) {
                ++index;
                YamlParser::Fields lomFields = {
                    { "index", true, YamlParser::Scalar, [&index](YamlParser *p) {
                          QCOMPARE(p->parseScalar().toInt(), index);
                      } },
                    { "name", true, YamlParser::Scalar, [&index](YamlParser *p) {
                          QCOMPARE(p->parseScalar().toString(), QString::number(index));
                      } }
                };
                p->parseFields(lomFields);
            });
            QCOMPARE(index, 2);
        });

        p.parseFields(fields);

        QVERIFY(!p.nextDocument());

    } catch (const Exception &e) {
        QVERIFY2(false, e.what());
    }
}

void tst_Yaml::documentParser()
{
    try {
        QFile f(":/data/test.yaml");
        QVERIFY2(f.open(QFile::ReadOnly), qPrintable(f.errorString()));
        QByteArray ba = f.readAll();
        QVERIFY(!ba.isEmpty());
        QVector<QVariant> docs = YamlParser::parseAllDocuments(ba);
        QCOMPARE(docs.size(), 2);
        QCOMPARE(docs.at(0).toMap().size(), 2);

        QVariantMap header = {
            { "formatVersion", 42 }, { "formatType", "testfile" }
        };

        QVariantMap main = {
            { "dec", 10 },
            { "hex", 16 },
            { "bin", 2 },
            { "oct", 8 },
            { "float1", 10.1 },
            { "float2", .1 },
            { "float3", .1 },
            { "number-separators", 1234567 },
            { "bool-true", true },
            { "bool-yes", true },
            { "bool-false", false },
            { "bool-no", false },
            { "null-literal", QVariant() },
            { "null-tilde", QVariant() },
            { "string-unquoted", qSL("unquoted") },
            { "string-singlequoted", qSL("singlequoted") },
            { "string-doublequoted", qSL("doublequoted") },
            { "list-int", QVariantList { 1, 2, 3 } },
            { "list-mixed", QVariantList { 1, qSL("two"), QVariantList { true, QVariant { } } } },
            { "map1", QVariantMap { { "a", 1 }, { "b", "two" }, { "c", QVariantList { 1, 2, 3 } } } },


            { "extended", QVariantMap { { "ext-string", "ext string" } } },

            { "stringlist-string", "string" },
            { "stringlist-list1", QVariantList { "string" } },
            { "stringlist-list2", QVariantList { "string1", "string2" } },

            { "list-of-maps", QVariantList { QVariantMap { { "index", 1 }, { "name", "1" } },
                                             QVariantMap { { "index", 2 }, { "name", "2" } } } }
        };

        QCOMPARE(header, docs.at(0).toMap());
        QCOMPARE(main, docs.at(1).toMap());

    } catch (const Exception &e) {
        QVERIFY2(false, e.what());
    }
}

QTEST_MAIN(tst_Yaml)

#include "tst_yaml.moc"