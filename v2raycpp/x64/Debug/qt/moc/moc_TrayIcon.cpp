/****************************************************************************
** Meta object code from reading C++ file 'TrayIcon.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../include/TrayIcon.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TrayIcon.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN8TrayIconE_t {};
} // unnamed namespace

template <> constexpr inline auto TrayIcon::qt_create_metaobjectdata<qt_meta_tag_ZN8TrayIconE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "TrayIcon",
        "startProxyClicked",
        "",
        "stopProxyClicked",
        "enableSystemProxyClicked",
        "disableSystemProxyClicked",
        "showWindowClicked",
        "settingsClicked",
        "exitClicked",
        "trayIconDoubleClicked",
        "onTrayIconActivated",
        "QSystemTrayIcon::ActivationReason",
        "reason",
        "onStartProxy",
        "onStopProxy",
        "onEnableSystemProxy",
        "onDisableSystemProxy",
        "onShowWindow",
        "onSettings",
        "onExit"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'startProxyClicked'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'stopProxyClicked'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'enableSystemProxyClicked'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'disableSystemProxyClicked'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'showWindowClicked'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'settingsClicked'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'exitClicked'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'trayIconDoubleClicked'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onTrayIconActivated'
        QtMocHelpers::SlotData<void(QSystemTrayIcon::ActivationReason)>(10, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'onStartProxy'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onStopProxy'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onEnableSystemProxy'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDisableSystemProxy'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onShowWindow'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSettings'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onExit'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<TrayIcon, qt_meta_tag_ZN8TrayIconE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject TrayIcon::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8TrayIconE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8TrayIconE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN8TrayIconE_t>.metaTypes,
    nullptr
} };

void TrayIcon::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<TrayIcon *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->startProxyClicked(); break;
        case 1: _t->stopProxyClicked(); break;
        case 2: _t->enableSystemProxyClicked(); break;
        case 3: _t->disableSystemProxyClicked(); break;
        case 4: _t->showWindowClicked(); break;
        case 5: _t->settingsClicked(); break;
        case 6: _t->exitClicked(); break;
        case 7: _t->trayIconDoubleClicked(); break;
        case 8: _t->onTrayIconActivated((*reinterpret_cast<std::add_pointer_t<QSystemTrayIcon::ActivationReason>>(_a[1]))); break;
        case 9: _t->onStartProxy(); break;
        case 10: _t->onStopProxy(); break;
        case 11: _t->onEnableSystemProxy(); break;
        case 12: _t->onDisableSystemProxy(); break;
        case 13: _t->onShowWindow(); break;
        case 14: _t->onSettings(); break;
        case 15: _t->onExit(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (TrayIcon::*)()>(_a, &TrayIcon::startProxyClicked, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrayIcon::*)()>(_a, &TrayIcon::stopProxyClicked, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrayIcon::*)()>(_a, &TrayIcon::enableSystemProxyClicked, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrayIcon::*)()>(_a, &TrayIcon::disableSystemProxyClicked, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrayIcon::*)()>(_a, &TrayIcon::showWindowClicked, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrayIcon::*)()>(_a, &TrayIcon::settingsClicked, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrayIcon::*)()>(_a, &TrayIcon::exitClicked, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (TrayIcon::*)()>(_a, &TrayIcon::trayIconDoubleClicked, 7))
            return;
    }
}

const QMetaObject *TrayIcon::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TrayIcon::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8TrayIconE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int TrayIcon::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void TrayIcon::startProxyClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void TrayIcon::stopProxyClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void TrayIcon::enableSystemProxyClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void TrayIcon::disableSystemProxyClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void TrayIcon::showWindowClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void TrayIcon::settingsClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void TrayIcon::exitClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void TrayIcon::trayIconDoubleClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}
QT_WARNING_POP
