#include "RunGuard.h"

#include <QBuffer>
#include <QCryptographicHash>
#include <QDataStream>

#include "InstanceControl.h"
#include "SharedMemoryThread.h"
#include "SystemSemaphoreReleaser.h"
#include "Windows.h"

namespace {
    QString generateKeyHash(const QString& key, const QString& salt) {
        QByteArray data;

        data.append(key.toUtf8());
        data.append(salt.toUtf8());
        data = QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex();

        return data;
    }
}

RunGuard::RunGuard(const QString& key, int memSize)
    : m_key(key)
    , m_memoryLockKey(generateKeyHash(m_key, "_memoryLockKey"))
    , m_sharedMemoryKey(generateKeyHash(m_key, "_sharedMemoryKey"))
    , m_memorySignalKey(generateKeyHash(m_key, "_memorySignalKey"))
    , m_memorySize(memSize)
    , m_sharedMemory(m_sharedMemoryKey)
    , m_memoryLock(m_memoryLockKey, 1)
    , m_memorySignal(m_memorySignalKey, 0)
    , m_sharedMemoryThread(nullptr)
{
    SystemSemaphoreReleaser releaser(m_memoryLock);
    {
        QSharedMemory fix(m_sharedMemoryKey);    // Fix for *nix: http://habrahabr.ru/post/173281/
        fix.attach();
    }
}

RunGuard::~RunGuard() {
    release();
    if ( m_sharedMemoryThread != nullptr ) {
        m_sharedMemoryThread->requestInterruption();
        m_memorySignal.release();
        m_sharedMemoryThread->wait();
        delete m_sharedMemoryThread;
    }
}

bool RunGuard::isAnotherRunning() {
    if ( m_sharedMemory.isAttached() ) {
        return false;
    }

    SystemSemaphoreReleaser releaser(m_memoryLock);
    const bool isRunning = m_sharedMemory.attach();
    if ( isRunning ) {
        m_sharedMemory.detach();
    }

    return isRunning;
}

bool RunGuard::tryToRun() {
    if ( isAnotherRunning() ) {   // Extra check
        return false;
    }

    bool result = false;
    {
        SystemSemaphoreReleaser releaser(m_memoryLock);
        result = m_sharedMemory.create(m_memorySize);
        if ( result ) {
            InstanceControl instanceControl;
            SharedMemoryThread::WriteInstanceControlToShareMemory(m_sharedMemory, instanceControl);
        }
    }
    if ( !result ) {
        release();
        return false;
    }
    return true;
}

void RunGuard::release() {
    SystemSemaphoreReleaser releaser(m_memoryLock);
    if ( m_sharedMemory.isAttached() ) {
        m_sharedMemory.detach();
    }
}

void RunGuard::sendFileToOpen(const QString &filename) {
    SystemSemaphoreReleaser releaser(m_memoryLock);
    if ( m_sharedMemory.attach() ) {
        InstanceControl instanceControl = SharedMemoryThread::ReadInstanceControlFromShareMemory(m_sharedMemory);
        QStringList fileList = instanceControl.getFilenameList();
        fileList.append(filename);
        instanceControl.setFilenameList(fileList);
        // If there isn't enough room in shared memory for the new filename, it simply isn't sent
        if (SharedMemoryThread::WriteInstanceControlToShareMemory(m_sharedMemory, instanceControl)) {
#if defined(Q_OS_WIN)
            if (instanceControl.getWindowsHandle() != 0)
            {
                SetForegroundWindow((HWND)instanceControl.getWindowsHandle());
            }
#endif // Q_OS_WIN
            m_memorySignal.release();
        }
    }
}

void RunGuard::runFileOpenThread(WId winId) {
    if ( m_sharedMemoryThread == nullptr ) {
        {
            SystemSemaphoreReleaser releaser(m_memoryLock);
            if ( m_sharedMemory.isAttached() ) {
                InstanceControl instanceControl = SharedMemoryThread::ReadInstanceControlFromShareMemory(m_sharedMemory);
                instanceControl.setWindowsHandle(winId);
                SharedMemoryThread::WriteInstanceControlToShareMemory(m_sharedMemory, instanceControl);
            }
        }
        m_sharedMemoryThread = new SharedMemoryThread(m_memoryLockKey, m_sharedMemoryKey, m_memorySignalKey);
        QObject::connect(m_sharedMemoryThread, &SharedMemoryThread::fileToOpen, this, &RunGuard::OpenFileReceivedEvent);
        m_sharedMemoryThread->start();
    }
}
