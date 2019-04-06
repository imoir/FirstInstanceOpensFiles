#ifndef INSTANCECONTROL_H
#define INSTANCECONTROL_H

#include "SerializerBase.h"

#include <QStringList>

class InstanceControl : public SerializerBase
{
    Q_OBJECT
    Q_PROPERTY(quint64 m_windowsHandle READ getWindowsHandle WRITE setWindowsHandle)
    Q_PROPERTY(QStringList m_filenameList READ getFilenameList WRITE setFilenameList)
public:
    InstanceControl() : m_windowsHandle(0) { }
    InstanceControl(const InstanceControl &ic) : m_windowsHandle(ic.getWindowsHandle()), m_filenameList(ic.getFilenameList())  { }
    quint64 getWindowsHandle() const { return m_windowsHandle; }
    void setWindowsHandle(quint64 newHandle) { m_windowsHandle = newHandle; }
    QStringList getFilenameList() const { return m_filenameList; }
    void setFilenameList(const QStringList &newFilenameList) { m_filenameList = newFilenameList; }
    InstanceControl& operator=(InstanceControl other) { m_windowsHandle = other.m_windowsHandle; m_filenameList = other.m_filenameList; return *this; }

private:
    quint64 m_windowsHandle;
    QStringList m_filenameList;
};

#endif // INSTANCECONTROL_H
