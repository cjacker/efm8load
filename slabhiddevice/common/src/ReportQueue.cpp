#include "ReportQueue.h"

void CReportQueue::Clear()
{
    m_Lock.Lock();

    m_queue.clear();

    m_Lock.Unlock();
}

void CReportQueue::Reserve(DWORD count)
{
    m_Lock.Lock();

    m_queue.reserve(count);

    m_Lock.Unlock();
}

void CReportQueue::Enqueue(const BYTE* data, DWORD size)
{
    m_Lock.Lock();

    for (DWORD i = 0; i < size; i++)
    {
        m_queue.push_back(data[i]);
    }

    m_Lock.Unlock();
}

void CReportQueue::Dequeue(BYTE* data, DWORD size, DWORD* bytesDequeued)
{
    m_Lock.Lock();

    // Figure out how many bytes to dequeue
    DWORD num = (size < m_queue.size()) ? size : static_cast<DWORD>(m_queue.size());

    // Copy the bytes to the user buffer
    for (DWORD i = 0; i < num; i++)
    {
        data[i] = m_queue[i];
    }

    // Remove the bytes from the queue
    m_queue.erase(m_queue.begin(), m_queue.begin() + num);

    // Return the number of bytes dequeued
    *bytesDequeued = num;

    m_Lock.Unlock();
}

DWORD CReportQueue::Size()
{
    m_Lock.Lock();

    DWORD size = static_cast<DWORD>(m_queue.size());

    m_Lock.Unlock();

    return size;
}
