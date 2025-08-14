/****************************************************************************
** Meta object code from reading C++ file 'SchematicWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../SchematicWidget.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SchematicWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.7.3. It"
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

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSSchematicWidgetENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSSchematicWidgetENDCLASS = QtMocHelpers::stringData(
    "SchematicWidget",
    "startOpenConfigureAnalysis",
    "",
    "startRunAnalysis",
    "startPlacingGround",
    "startPlacingResistor",
    "startPlacingCapacitor",
    "startPlacingInductor",
    "startPlacingVoltageSource",
    "startPlacingCurrentSource",
    "startPlacingDiode",
    "startDeleteComponent",
    "startPlacingWire",
    "startOpenNodeLibrary",
    "startPlacingLabel",
    "handleNodeLibraryItemSelection",
    "compType"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSSchematicWidgetENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   98,    2, 0x0a,    1 /* Public */,
       3,    0,   99,    2, 0x0a,    2 /* Public */,
       4,    0,  100,    2, 0x0a,    3 /* Public */,
       5,    0,  101,    2, 0x0a,    4 /* Public */,
       6,    0,  102,    2, 0x0a,    5 /* Public */,
       7,    0,  103,    2, 0x0a,    6 /* Public */,
       8,    0,  104,    2, 0x0a,    7 /* Public */,
       9,    0,  105,    2, 0x0a,    8 /* Public */,
      10,    0,  106,    2, 0x0a,    9 /* Public */,
      11,    0,  107,    2, 0x0a,   10 /* Public */,
      12,    0,  108,    2, 0x0a,   11 /* Public */,
      13,    0,  109,    2, 0x0a,   12 /* Public */,
      14,    0,  110,    2, 0x0a,   13 /* Public */,
      15,    1,  111,    2, 0x08,   14 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   16,

       0        // eod
};

Q_CONSTINIT const QMetaObject SchematicWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSSchematicWidgetENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSSchematicWidgetENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSSchematicWidgetENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<SchematicWidget, std::true_type>,
        // method 'startOpenConfigureAnalysis'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'startRunAnalysis'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'startPlacingGround'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'startPlacingResistor'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'startPlacingCapacitor'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'startPlacingInductor'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'startPlacingVoltageSource'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'startPlacingCurrentSource'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'startPlacingDiode'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'startDeleteComponent'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'startPlacingWire'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'startOpenNodeLibrary'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'startPlacingLabel'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleNodeLibraryItemSelection'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void SchematicWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SchematicWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->startOpenConfigureAnalysis(); break;
        case 1: _t->startRunAnalysis(); break;
        case 2: _t->startPlacingGround(); break;
        case 3: _t->startPlacingResistor(); break;
        case 4: _t->startPlacingCapacitor(); break;
        case 5: _t->startPlacingInductor(); break;
        case 6: _t->startPlacingVoltageSource(); break;
        case 7: _t->startPlacingCurrentSource(); break;
        case 8: _t->startPlacingDiode(); break;
        case 9: _t->startDeleteComponent(); break;
        case 10: _t->startPlacingWire(); break;
        case 11: _t->startOpenNodeLibrary(); break;
        case 12: _t->startPlacingLabel(); break;
        case 13: _t->handleNodeLibraryItemSelection((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *SchematicWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SchematicWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSSchematicWidgetENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int SchematicWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 14;
    }
    return _id;
}
QT_WARNING_POP
