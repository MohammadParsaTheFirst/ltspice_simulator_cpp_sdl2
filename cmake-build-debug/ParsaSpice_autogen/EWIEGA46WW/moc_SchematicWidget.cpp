/****************************************************************************
** Meta object code from reading C++ file 'SchematicWidget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
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
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.1. It"
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
struct qt_meta_tag_ZN15SchematicWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto SchematicWidget::qt_create_metaobjectdata<qt_meta_tag_ZN15SchematicWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SchematicWidget",
        "startOpenConfigureAnalysis",
        "",
        "startRunAnalysis",
        "startPlacingGround",
        "startPlacingResistor",
        "startPlacingCapacitor",
        "startPlacingInductor",
        "startPlacingVoltageSource",
        "startPlacingACVoltageSource",
        "startPlacingCurrentSource",
        "startPlacingDiode",
        "startDeleteComponent",
        "startPlacingWire",
        "startOpenNodeLibrary",
        "startPlacingLabel",
        "startCreateSubcircuit",
        "startPlacingSubcircuit",
        "startOpeningSubcircuitLibrary",
        "handleNodeLibraryItemSelection",
        "compType"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'startOpenConfigureAnalysis'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'startRunAnalysis'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'startPlacingGround'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'startPlacingResistor'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'startPlacingCapacitor'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'startPlacingInductor'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'startPlacingVoltageSource'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'startPlacingACVoltageSource'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'startPlacingCurrentSource'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'startPlacingDiode'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'startDeleteComponent'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'startPlacingWire'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'startOpenNodeLibrary'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'startPlacingLabel'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'startCreateSubcircuit'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'startPlacingSubcircuit'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'startOpeningSubcircuitLibrary'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'handleNodeLibraryItemSelection'
        QtMocHelpers::SlotData<void(const QString &)>(19, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 20 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SchematicWidget, qt_meta_tag_ZN15SchematicWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SchematicWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15SchematicWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15SchematicWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15SchematicWidgetE_t>.metaTypes,
    nullptr
} };

void SchematicWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SchematicWidget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->startOpenConfigureAnalysis(); break;
        case 1: _t->startRunAnalysis(); break;
        case 2: _t->startPlacingGround(); break;
        case 3: _t->startPlacingResistor(); break;
        case 4: _t->startPlacingCapacitor(); break;
        case 5: _t->startPlacingInductor(); break;
        case 6: _t->startPlacingVoltageSource(); break;
        case 7: _t->startPlacingACVoltageSource(); break;
        case 8: _t->startPlacingCurrentSource(); break;
        case 9: _t->startPlacingDiode(); break;
        case 10: _t->startDeleteComponent(); break;
        case 11: _t->startPlacingWire(); break;
        case 12: _t->startOpenNodeLibrary(); break;
        case 13: _t->startPlacingLabel(); break;
        case 14: _t->startCreateSubcircuit(); break;
        case 15: _t->startPlacingSubcircuit(); break;
        case 16: _t->startOpeningSubcircuitLibrary(); break;
        case 17: _t->handleNodeLibraryItemSelection((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
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
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15SchematicWidgetE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int SchematicWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 18)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 18;
    }
    return _id;
}
QT_WARNING_POP
