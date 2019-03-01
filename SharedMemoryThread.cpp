#include "SharedMemoryThread.h"

#include <QBuffer>
#include <QDataStream>
#include <QSharedMemory>
#include <QSystemSemaphore>

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
        QStringList fileList;

        if ( !QThread::currentThread()->isInterruptionRequested() ) {
            SystemSemaphoreReleaser releaser(memoryLock);
            fileList = SharedMemoryThread::ReadStringListFromShareMemory(sharedMemory);
            QStringList emptyFileList;
            SharedMemoryThread::WriteStringListToShareMemory(sharedMemory, emptyFileList);
        }

        if ( !QThread::currentThread()->isInterruptionRequested() ) {
            for ( const auto& filename : fileList ) {
                emit fileToOpen(filename);
            }
        }
    }
}

/*
 * DANGER DANGER!! Assumes shared memory is locked for access. You will pay dearly if it isn't
 */
QStringList SharedMemoryThread::ReadStringListFromShareMemory(QSharedMemory &sharedMemory) {
    QStringList fileList;
    if ( sharedMemory.isAttached() ) {
        QBuffer buffer;
        QDataStream in(&buffer);
        buffer.setData((char*)sharedMemory.constData(), sharedMemory.size());
        buffer.open(QBuffer::ReadOnly);
        in >> fileList;
    }
    return fileList;
}

/*
 * DANGER DANGER!! Assumes shared memory is locked for access. You will pay dearly if it isn't
 * return false if there was not enough room shared memory to fit the string list
 */
bool SharedMemoryThread::WriteStringListToShareMemory(QSharedMemory &sharedMemory, const QStringList &list) {
    if ( sharedMemory.isAttached() ) {
        QBuffer buffer;
        buffer.open(QBuffer::ReadWrite);
        QDataStream out(&buffer);
        out << list;
        if ( buffer.size() < sharedMemory.size() ) {
            char *to = (char*)sharedMemory.data();
            const char *from = buffer.data().data();
            memcpy(to, from, buffer.size());
            return true;
        }
    }
    return false;
}
