#ifndef SERIALIZERBASE_H
#define SERIALIZERBASE_H

#include <QObject>

// taken from https://stackoverflow.com/questions/13835197/serializing-my-custom-class-in-qt

class SerializerBase : public QObject
{
    Q_OBJECT
public:
    explicit SerializerBase(QObject *parent = nullptr) : QObject(parent) {;}
};

QDataStream &operator<<(QDataStream &ds, const SerializerBase &obj);
QDataStream &operator>>(QDataStream &ds, SerializerBase &obj) ;

#endif // SERIALIZERBASE_H
