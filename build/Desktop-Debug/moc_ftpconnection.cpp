/****************************************************************************
** Meta object code from reading C++ file 'ftpconnection.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.15)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../ftpconnection.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ftpconnection.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.15. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_FtpConnection_t {
    QByteArrayData data[12];
    char stringdata0[144];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FtpConnection_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FtpConnection_t qt_meta_stringdata_FtpConnection = {
    {
QT_MOC_LITERAL(0, 0, 13), // "FtpConnection"
QT_MOC_LITERAL(1, 14, 12), // "disconnected"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 10), // "logMessage"
QT_MOC_LITERAL(4, 39, 7), // "message"
QT_MOC_LITERAL(5, 47, 14), // "processCommand"
QT_MOC_LITERAL(6, 62, 15), // "onDataConnected"
QT_MOC_LITERAL(7, 78, 15), // "onDataReadyRead"
QT_MOC_LITERAL(8, 94, 18), // "onDataDisconnected"
QT_MOC_LITERAL(9, 113, 9), // "onTimeout"
QT_MOC_LITERAL(10, 123, 14), // "onBytesWritten"
QT_MOC_LITERAL(11, 138, 5) // "bytes"

    },
    "FtpConnection\0disconnected\0\0logMessage\0"
    "message\0processCommand\0onDataConnected\0"
    "onDataReadyRead\0onDataDisconnected\0"
    "onTimeout\0onBytesWritten\0bytes"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FtpConnection[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   54,    2, 0x06 /* Public */,
       3,    1,   55,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   58,    2, 0x08 /* Private */,
       6,    0,   59,    2, 0x08 /* Private */,
       7,    0,   60,    2, 0x08 /* Private */,
       8,    0,   61,    2, 0x08 /* Private */,
       9,    0,   62,    2, 0x08 /* Private */,
      10,    1,   63,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::LongLong,   11,

       0        // eod
};

void FtpConnection::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FtpConnection *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->disconnected(); break;
        case 1: _t->logMessage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->processCommand(); break;
        case 3: _t->onDataConnected(); break;
        case 4: _t->onDataReadyRead(); break;
        case 5: _t->onDataDisconnected(); break;
        case 6: _t->onTimeout(); break;
        case 7: _t->onBytesWritten((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (FtpConnection::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FtpConnection::disconnected)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (FtpConnection::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FtpConnection::logMessage)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject FtpConnection::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_FtpConnection.data,
    qt_meta_data_FtpConnection,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *FtpConnection::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FtpConnection::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_FtpConnection.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int FtpConnection::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void FtpConnection::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void FtpConnection::logMessage(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
