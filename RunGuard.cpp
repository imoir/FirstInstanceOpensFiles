#include "RunGuard.h"

#include <QBuffer>
#include <QCryptographicHash>
#include <QDataStream>

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
    , m_memorySize(memSize)
    , m_memoryLockKey(generateKeyHash(m_key, "_memoryLockKey"))
    , m_sharedMemoryKey(generateKeyHash(m_key, "_sharedMemoryKey"))
    , m_memorySignalKey(generateKeyHash(m_key, "_memorySignalKey"))
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
            QStringList fileList;
            SharedMemoryThread::WriteStringListToShareMemory(m_sharedMemory, fileList);
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
        QStringList fileList = SharedMemoryThread::ReadStringListFromShareMemory(m_sharedMemory);
        fileList.append(filename);
        // If there isn't enough room in shared memory for the new filename, it simply isn't sent
        if (SharedMemoryThread::WriteStringListToShareMemory(m_sharedMemory, fileList)) {
            m_memorySignal.release();
        }
    }
}

void RunGuard::runFileOpenThread() {
    if ( m_sharedMemoryThread == nullptr ) {
        m_sharedMemoryThread = new SharedMemoryThread(m_memoryLockKey, m_sharedMemoryKey, m_memorySignalKey);
        QObject::connect(m_sharedMemoryThread, &SharedMemoryThread::fileToOpen, this, &RunGuard::OpenFileReceivedEvent);
        QObject::connect(m_sharedMemoryThread, SIGNAL(finished()), m_sharedMemoryThread, SLOT(deleteLater()));
        m_sharedMemoryThread->start();
    }
}
