#ifndef SHAREDMEMORYTHREAD_H
#define SHAREDMEMORYTHREAD_H

#include <QThread>

class QSharedMemory;
class InstanceControl;

class SharedMemoryThread : public QThread
{
    Q_OBJECT
    void run() override;
public:
    SharedMemoryThread(const QString &memoryLockKey, const QString &sharedMemoryKey, const QString &memorySignalKey);

    static InstanceControl ReadInstanceControlFromShareMemory(QSharedMemory &sharedMemory);
    static bool WriteInstanceControlToShareMemory(QSharedMemory &sharedMemory, const InstanceControl &instanceControl);

signals:
    void fileToOpen(const QString &filename);

private:
    const QString m_memoryLockKey;
    const QString m_sharedMemoryKey;
    const QString m_memorySignalKey;
};

#endif // SHAREDMEMORYTHREAD_H
