#ifndef REPORT_QUEUE_H
#define REPORT_QUEUE_H

#include "OsDep.h"
#include <vector>

class CReportQueue
{
public:
    void Clear();
    void Reserve(DWORD count);
    void Enqueue(const BYTE* data, DWORD size);
    void Dequeue(BYTE* data, DWORD size, DWORD* bytesDequeued);
    DWORD Size();

protected:
    std::vector<BYTE>     m_queue;
    CCriticalSectionLock  m_Lock;
};

#endif // REPORT_QUEUE_H
