#include "SharedMemoryThread.h"

#include <QBuffer>
#include <QDataStream>
#include <QSharedMemory>
#include <QSystemSemaphore>

#include "InstanceControl.h"
#include "SystemSemaphoreReleaser.h"
#include "Windows.h"

SharedMemoryThread::SharedMemoryThread(const QString &memoryLockKey, const QString &sharedMemoryKey, const QString &memorySignalKey)
    : QThread(nullptr), m_memoryLockKey(memoryLockKey), m_sharedMemoryKey(sharedMemoryKey), m_memorySignalKey(memorySignalKey)
{ }

void SharedMemoryThread::run() {
    QSystemSemaphore memoryLock(m_memoryLockKey);
    QSystemSemaphore memorySignal(m_memorySignalKey);
    QSharedMemory sharedMemory(m_sharedMemoryKey);
    sharedMemory.attach();
    while ( !QThread::currentThread()->isInterruptionRequested() )  {
        memorySignal.acquire();
        InstanceControl instanceControl;

        if ( !QThread::currentThread()->isInterruptionRequested() ) {
            SystemSemaphoreReleaser releaser(memoryLock);
            instanceControl = SharedMemoryThread::ReadInstanceControlFromShareMemory(sharedMemory);
            InstanceControl emptyInstanceControl;
            emptyInstanceControl.setWindowsHandle(instanceControl.getWindowsHandle());
            SharedMemoryThread::WriteInstanceControlToShareMemory(sharedMemory, emptyInstanceControl);
        }

        if ( !QThread::currentThread()->isInterruptionRequested() ) {
            QStringList fileList = instanceControl.getFilenameList();
            for ( const auto& filename : fileList ) {
                emit fileToOpen(filename);
            }
        }
    }
}

/*
 * DANGER DANGER!! Assumes shared memory is locked for access. You will pay dearly if it isn't
 */
InstanceControl SharedMemoryThread::ReadInstanceControlFromShareMemory(QSharedMemory &sharedMemory) {
    InstanceControl instanceControl;
    if ( sharedMemory.isAttached() ) {
        QBuffer buffer;
        QDataStream in(&buffer);
        buffer.setData((char*)sharedMemory.constData(), sharedMemory.size());
        buffer.open(QBuffer::ReadOnly);
        in >> instanceControl;
    }
    return instanceControl;
}

/*
 * DANGER DANGER!! Assumes shared memory is locked for access. You will pay dearly if it isn't
 * return false if there was not enough room shared memory to fit the string list
 */
bool SharedMemoryThread::WriteInstanceControlToShareMemory(QSharedMemory &sharedMemory, const InstanceControl &instanceControl) {
    if ( sharedMemory.isAttached() ) {
        QBuffer buffer;
        buffer.open(QBuffer::ReadWrite);
        QDataStream out(&buffer);
        out << instanceControl;
        if ( buffer.size() < sharedMemory.size() ) {
            char *to = (char*)sharedMemory.data();
            const char *from = buffer.data().data();
            memcpy(to, from, buffer.size());
            return true;
        }
    }
    return false;
}
