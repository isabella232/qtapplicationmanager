TARGET = tst_yaml

include($$PWD/../tests.pri)

QT *= appman_common-private

SOURCES += tst_yaml.cpp

RESOURCES += data/test.yaml