/****************************************************************************
** Meta object code from reading C++ file 'CoreManager.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../include/CoreManager.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CoreManager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN11CoreManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto CoreManager::qt_create_metaobjectdata<qt_meta_tag_ZN11CoreManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "CoreManager",
        "logOutput",
        "",
        "log",
        "errorOutput",
        "error",
        "statusChanged",
        "CoreStatus",
        "status",
        "processFinished",
        "exitCode",
        "QProcess::ExitStatus",
        "exitStatus",
        "onReadyReadStandardOutput",
        "onReadyReadStandardError",
        "onProcessStateChanged",
        "QProcess::ProcessState",
        "state",
        "onProcessFinished"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'logOutput'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'errorOutput'
        QtMocHelpers::SignalData<void(const QString &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 },
        }}),
        // Signal 'statusChanged'
        QtMocHelpers::SignalData<void(CoreStatus)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 7, 8 },
        }}),
        // Signal 'processFinished'
        QtMocHelpers::SignalData<void(int, QProcess::ExitStatus)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 10 }, { 0x80000000 | 11, 12 },
        }}),
        // Slot 'onReadyReadStandardOutput'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onReadyReadStandardError'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onProcessStateChanged'
        QtMocHelpers::SlotData<void(QProcess::ProcessState)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 16, 17 },
        }}),
        // Slot 'onProcessFinished'
        QtMocHelpers::SlotData<void(int, QProcess::ExitStatus)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 10 }, { 0x80000000 | 11, 12 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<CoreManager, qt_meta_tag_ZN11CoreManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject CoreManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11CoreManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11CoreManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11CoreManagerE_t>.metaTypes,
    nullptr
} };

void CoreManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CoreManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->logOutput((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->errorOutput((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->statusChanged((*reinterpret_cast<std::add_pointer_t<CoreStatus>>(_a[1]))); break;
        case 3: _t->processFinished((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QProcess::ExitStatus>>(_a[2]))); break;
        case 4: _t->onReadyReadStandardOutput(); break;
        case 5: _t->onReadyReadStandardError(); break;
        case 6: _t->onProcessStateChanged((*reinterpret_cast<std::add_pointer_t<QProcess::ProcessState>>(_a[1]))); break;
        case 7: _t->onProcessFinished((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QProcess::ExitStatus>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (CoreManager::*)(const QString & )>(_a, &CoreManager::logOutput, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (CoreManager::*)(const QString & )>(_a, &CoreManager::errorOutput, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (CoreManager::*)(CoreStatus )>(_a, &CoreManager::statusChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (CoreManager::*)(int , QProcess::ExitStatus )>(_a, &CoreManager::processFinished, 3))
            return;
    }
}

const QMetaObject *CoreManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CoreManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11CoreManagerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CoreManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void CoreManager::logOutput(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void CoreManager::errorOutput(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void CoreManager::statusChanged(CoreStatus _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void CoreManager::processFinished(int _t1, QProcess::ExitStatus _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}
QT_WARNING_POP
