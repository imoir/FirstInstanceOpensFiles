#ifndef SHAREDMEMORYTHREAD_H
#define SHAREDMEMORYTHREAD_H

#include <QThread>

class QSharedMemory;

class SharedMemoryThread : public QThread
{
    Q_OBJECT
    void run() override;
public:
    SharedMemoryThread(const QString &memoryLockKey, const QString &sharedMemoryKey, const QString &memorySignalKey);

    static QStringList ReadStringListFromShareMemory(QSharedMemory &sharedMemory);
    static bool WriteStringListToShareMemory(QSharedMemory &sharedMemory, const QStringList &list);

signals:
    void fileToOpen(const QString &filename);

private:
    const QString m_memoryLockKey;
    const QString m_sharedMemoryKey;
    const QString m_memorySignalKey;
};

#endif // SHAREDMEMORYTHREAD_H
