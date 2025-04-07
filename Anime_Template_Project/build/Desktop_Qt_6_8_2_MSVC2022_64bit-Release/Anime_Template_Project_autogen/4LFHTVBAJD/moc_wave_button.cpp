/****************************************************************************
** Meta object code from reading C++ file 'wave_button.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../button_class/wave_button.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'wave_button.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.2. It"
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
struct qt_meta_tag_ZN11Wave_buttonE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN11Wave_buttonE = QtMocHelpers::stringData(
    "Wave_button",
    "angleChanged",
    "",
    "wave_positionChanged",
    "wave_transparencyChanged",
    "right_angleChanged",
    "angle",
    "wave_position",
    "wave_transparency",
    "right_angle"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN11Wave_buttonE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       4,   42, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   38,    2, 0x06,    5 /* Public */,
       3,    0,   39,    2, 0x06,    6 /* Public */,
       4,    0,   40,    2, 0x06,    7 /* Public */,
       5,    0,   41,    2, 0x06,    8 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags, notifyId, revision
       6, QMetaType::Int, 0x00015903, uint(0), 0,
       7, QMetaType::Int, 0x00015903, uint(1), 0,
       8, QMetaType::Int, 0x00015903, uint(2), 0,
       9, QMetaType::Int, 0x00015903, uint(3), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject Wave_button::staticMetaObject = { {
    QMetaObject::SuperData::link<QPushButton::staticMetaObject>(),
    qt_meta_stringdata_ZN11Wave_buttonE.offsetsAndSizes,
    qt_meta_data_ZN11Wave_buttonE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN11Wave_buttonE_t,
        // property 'angle'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'wave_position'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'wave_transparency'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'right_angle'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Wave_button, std::true_type>,
        // method 'angleChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'wave_positionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'wave_transparencyChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'right_angleChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void Wave_button::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<Wave_button *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->angleChanged(); break;
        case 1: _t->wave_positionChanged(); break;
        case 2: _t->wave_transparencyChanged(); break;
        case 3: _t->right_angleChanged(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (Wave_button::*)();
            if (_q_method_type _q_method = &Wave_button::angleChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (Wave_button::*)();
            if (_q_method_type _q_method = &Wave_button::wave_positionChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (Wave_button::*)();
            if (_q_method_type _q_method = &Wave_button::wave_transparencyChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (Wave_button::*)();
            if (_q_method_type _q_method = &Wave_button::right_angleChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = _t->angle(); break;
        case 1: *reinterpret_cast< int*>(_v) = _t->wave_position(); break;
        case 2: *reinterpret_cast< int*>(_v) = _t->wave_transparency(); break;
        case 3: *reinterpret_cast< int*>(_v) = _t->right_angle(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setAngle(*reinterpret_cast< int*>(_v)); break;
        case 1: _t->setWave_position(*reinterpret_cast< int*>(_v)); break;
        case 2: _t->setWave_transparency(*reinterpret_cast< int*>(_v)); break;
        case 3: _t->setRight_angle(*reinterpret_cast< int*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *Wave_button::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Wave_button::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN11Wave_buttonE.stringdata0))
        return static_cast<void*>(this);
    return QPushButton::qt_metacast(_clname);
}

int Wave_button::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QPushButton::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void Wave_button::angleChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void Wave_button::wave_positionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void Wave_button::wave_transparencyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void Wave_button::right_angleChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
