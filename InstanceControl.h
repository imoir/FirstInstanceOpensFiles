#ifndef INSTANCECONTROL_H
#define INSTANCECONTROL_H

#include "SerializerBase.h"

#include <QStringList>

class InstanceControl : public SerializerBase
{
    Q_OBJECT
    Q_PROPERTY(uint64_t m_windowsHandle READ getWindowsHandle WRITE setWindowsHandle)
    Q_PROPERTY(QStringList m_filenameList READ getFilenameList WRITE setFilenameList)
public:
    InstanceControl();
    uint64_t getWindowsHandle() const { return m_windowsHandle; }
    void setWindowsHandle(uint64_t newId) { m_windowsHandle = newId; }
    QStringList getFilenameList() const { return m_filenameList; }
    void setFilenameList(const QStringList &newFilenameList) { m_filenameList = newFilenameList; }

private:
    uint64_t m_windowsHandle;
    QStringList m_filenameList;
};

#endif // INSTANCECONTROL_H
