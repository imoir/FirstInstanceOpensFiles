#ifndef SYSTEMSEMAPHORERELEASER_H
#define SYSTEMSEMAPHORERELEASER_H

#include <QSystemSemaphore>

class SystemSemaphoreReleaser
{
public:
    SystemSemaphoreReleaser(QSystemSemaphore &semaphore) : m_semaphore(semaphore) { m_semaphore.acquire(); }
    ~SystemSemaphoreReleaser() { m_semaphore.release(); }
private:
    QSystemSemaphore &m_semaphore;
};

#endif // SYSTEMSEMAPHORERELEASER_H
