#ifndef SERIALIZERBASE_H
#define SERIALIZERBASE_H

#include <QObject>

/*************************
 * Adapted from Kamil Klimek's (https://stackoverflow.com/users/223880/kamil-klimek) contribution to
 * https://stackoverflow.com/a/13835444
 */

class SerializerBase : public QObject
{
    Q_OBJECT
public:
    explicit SerializerBase(QObject *parent = nullptr) : QObject(parent) {;}
};

QDataStream &operator<<(QDataStream &ds, const SerializerBase &obj);
QDataStream &operator>>(QDataStream &ds, SerializerBase &obj) ;

#endif // SERIALIZERBASE_H
