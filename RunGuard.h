#ifndef RUNGUARD_H
#define RUNGUARD_H

/*************************
 * Adapted from Dmitry Sazonov's (https://stackoverflow.com/users/1035613/dmitry-sazonov) contribution to
 * https://stackoverflow.com/a/28172162
 */

#include <QObject>
#include <QSharedMemory>
#include <QSystemSemaphore>

class SharedMemoryThread;

class RunGuard : public QObject
{
    Q_OBJECT

public:
    RunGuard(const QString& key, int memorySize = 1024);
    ~RunGuard();

    bool isAnotherRunning();
    bool tryToRun();
    void release();
    void sendFileToOpen(const QString &filename);
    void runFileOpenThread();

signals:
    void OpenFileReceivedEvent(QString filename);

private:
    const QString m_key;
    const QString m_memoryLockKey;
    const QString m_sharedMemoryKey;
    const QString m_memorySignalKey;
    const int m_memorySize;

    QSharedMemory m_sharedMemory;
    QSystemSemaphore m_memoryLock;
    QSystemSemaphore m_memorySignal;

    SharedMemoryThread *m_sharedMemoryThread;

    Q_DISABLE_COPY(RunGuard)
};

#endif // RUNGUARD_H
