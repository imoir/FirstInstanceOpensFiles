#include "SerializerBase.h"

#include <QMetaProperty>
#include <QVariant>

QDataStream &operator<<(QDataStream &ds, const SerializerBase &obj) {
    for(int i=0; i<obj.metaObject()->propertyCount(); ++i) {
        if(obj.metaObject()->property(i).isStored(&obj)) {
            ds << obj.metaObject()->property(i).read(&obj);
        }
    }
    return ds;
}

QDataStream &operator>>(QDataStream &ds, SerializerBase &obj) {
    QVariant var;
    for(int i=0; i<obj.metaObject()->propertyCount(); ++i) {
        if(obj.metaObject()->property(i).isStored(&obj)) {
            ds >> var;
            obj.metaObject()->property(i).write(&obj, var);
        }
    }
    return ds;
}
