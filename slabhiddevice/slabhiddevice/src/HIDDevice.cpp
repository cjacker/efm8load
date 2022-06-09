////////////////////////////////////////////////////////////////////////////////
// HidDevice.cpp
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include "HIDDevice.h"
#include "HIDDeviceVersion.h"
#include "ReportQueue.h"
#include "SLABHIDDevice.h"
#include "OsDep.h"
#include <string.h>
#include <wchar.h>
#include <cstdio>
#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Static Function Prototypes
////////////////////////////////////////////////////////////////////////////////

// Return a vid, pid, path, or serial string for the specified io object
static BYTE GetDeviceInfoString(hid_device_info* info, BYTE hidStringType, char* deviceString, DWORD deviceStringLength);

////////////////////////////////////////////////////////////////////////////////
// Static Functions
////////////////////////////////////////////////////////////////////////////////

// Return a vid, pid, path, or serial string for the specified hid device info structure

static BYTE GetDeviceInfoString(hid_device_info* info, BYTE hidStringType, char* deviceString, DWORD deviceStringLength)
{
    BYTE status = HID_DEVICE_INVALID_BUFFER_SIZE;

    // Check valid pointer
    if (deviceString)
    {
        switch (hidStringType)
        {
        case HID_VID_STRING:
            if (deviceStringLength >= MAX_VID_LENGTH)
            {
                sprintf(deviceString, "%04x", info->vendor_id);
                status = HID_DEVICE_SUCCESS;
            }
            break;

        case HID_PID_STRING:
            if (deviceStringLength >= MAX_PID_LENGTH)
            {
                sprintf(deviceString, "%04x", info->product_id);
                status = HID_DEVICE_SUCCESS;
            }
            break;

        case HID_PATH_STRING:
            if (deviceStringLength >= MAX_PATH_LENGTH)
            {
                size_t len = MIN(strlen(info->path), MAX_PATH_LENGTH - 1);

                memset(deviceString, 0x00, MAX_PATH_LENGTH);
                memcpy(deviceString, info->path, len);
                status = HID_DEVICE_SUCCESS;
            }
            break;

        case HID_SERIAL_STRING:
            if (deviceStringLength >= MAX_SERIAL_STRING_LENGTH)
            {
                if (info->serial_number != NULL)
                {                
                    size_t len = MIN(wcslen(info->serial_number), MAX_SERIAL_STRING_LENGTH - 1);

                    memset(deviceString, 0x00, MAX_SERIAL_STRING_LENGTH);

                    for (size_t i = 0; i < len; i++)
                    {
                        deviceString[i] = info->serial_number[i];
                    }

                    status = HID_DEVICE_SUCCESS;
                }
                else
                {
                    status = HID_DEVICE_CANNOT_GET_HID_INFO;
                }
            }
            break;

        case HID_MANUFACTURER_STRING:
            if (deviceStringLength >= MAX_MANUFACTURER_STRING_LENGTH)
            {
                if (info->manufacturer_string != NULL)
                {
                    size_t len = MIN(wcslen(info->manufacturer_string), MAX_MANUFACTURER_STRING_LENGTH - 1);

                    memset(deviceString, 0x00, MAX_MANUFACTURER_STRING_LENGTH);

                    for (size_t i = 0; i < len; i++)
                    {
                        deviceString[i] = info->manufacturer_string[i];
                    }

                    status = HID_DEVICE_SUCCESS;
                }
                else
                {
                    status = HID_DEVICE_CANNOT_GET_HID_INFO;
                }
            }
            break;

        case HID_PRODUCT_STRING:
            if (deviceStringLength >= MAX_PRODUCT_STRING_LENGTH)
            {
                if (info->product_string != NULL)
                {
                    size_t len = MIN(wcslen(info->product_string), MAX_PRODUCT_STRING_LENGTH - 1);

                    memset(deviceString, 0x00, MAX_PRODUCT_STRING_LENGTH);

                    for (size_t i = 0; i < len; i++)
                    {
                        deviceString[i] = info->product_string[i];
                    }

                    status = HID_DEVICE_SUCCESS;
                }
                else
                {
                    status = HID_DEVICE_CANNOT_GET_HID_INFO;
                }
            }
            break;

        default:
            status = HID_DEVICE_CANNOT_GET_HID_INFO;
        }
    }
    else
    {
        status = HID_DEVICE_INVALID_BUFFER_SIZE;
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
// Global Functions
////////////////////////////////////////////////////////////////////////////////

// Return the number of connected HID devices with matching VID/PID
// Set vid or pid to 0 to ignore vid or pid

DWORD GetNumHidDevices(WORD vid, WORD pid)
{
    DWORD numDevices = 0;
    hid_device_info* infos;

    infos = hid_enumerate(vid, pid);

    if (infos)
    {
        hid_device_info* cur = infos;

        while (cur)
        {
            numDevices++;
            cur = cur->next;
        }

        hid_free_enumeration(infos);
    }

    return numDevices;
}

// Return the vid, pid, path, serial, manufacturer, or product string for the specified device

BYTE GetHidString(DWORD deviceIndex, WORD vid, WORD pid, BYTE hidStringType, char* deviceString, DWORD deviceStringLength)
{
    BYTE status = HID_DEVICE_NOT_FOUND;

    hid_device_info* infos;

    infos = hid_enumerate(vid, pid);

    if (infos)
    {
        hid_device_info* cur = infos;
        DWORD curIndex = 0;

        while (cur)
        {
            if (curIndex == deviceIndex)
            {
                status = GetDeviceInfoString(cur, hidStringType, deviceString, deviceStringLength);
                break;
            }

            cur = cur->next;
            curIndex++;
        }

        hid_free_enumeration(infos);
    }

    return status;
}

// Return an indexed USB string for the specified device

BYTE GetHidIndexedString(DWORD deviceIndex, WORD vid, WORD pid, DWORD stringIndex, char* deviceString, DWORD deviceStringLength)
{
    BYTE status = HID_DEVICE_NOT_FOUND;

    hid_device_info* infos;

    infos = hid_enumerate(vid, pid);

    if (infos)
    {
        hid_device_info* cur = infos;
        DWORD curIndex = 0;

        while (cur)
        {
            if (curIndex == deviceIndex)
            {
                hid_device* device = hid_open_path(cur->path);

                if (device)
                {
                    DWORD wDeviceStringLength = deviceStringLength;
                    wchar_t* wDeviceString = new wchar_t[wDeviceStringLength];

                    memset(deviceString, 0x00, deviceStringLength);

                    if (hid_get_indexed_string(device, stringIndex, wDeviceString, wDeviceStringLength) != -1)
                    {
                        for (DWORD i = 0; i < deviceStringLength; i++)
                        {
                            deviceString[i] = wDeviceString[i];
                        }

                        status = HID_DEVICE_SUCCESS;
                    }
                    else
                    {
                        status = HID_DEVICE_CANNOT_GET_HID_INFO;
                    }

                    hid_close(device);

                    delete [] wDeviceString;
                }

                break;
            }

            cur = cur->next;
            curIndex++;
        }

        hid_free_enumeration(infos);
    }

    return status;
}

// Returns the vid, pid, and release/version number for the specified device

BYTE GetHidAttributes(DWORD deviceIndex, WORD vid, WORD pid, WORD* deviceVid, WORD* devicePid, WORD* deviceReleaseNumber)
{
    BYTE status = HID_DEVICE_NOT_FOUND;

    hid_device_info* infos;

    infos = hid_enumerate(vid, pid);

    if (infos)
    {
        hid_device_info* cur = infos;
        DWORD curIndex = 0;

        while (cur)
        {
            if (curIndex == deviceIndex)
            {
                *deviceVid = cur->vendor_id;
                *devicePid = cur->product_id;
                *deviceReleaseNumber = cur->release_number;
                status = HID_DEVICE_SUCCESS;
                break;
            }

            cur = cur->next;
            curIndex++;
        }

        hid_free_enumeration(infos);
    }

    return status;
}

// Return the library version information

BYTE GetHidLibraryVersion(BYTE* major, BYTE* minor, BOOL* release)
{
    BYTE status = HID_DEVICE_INVALID_BUFFER_SIZE;

    // Check pointers are not NULL
    if (major && minor && release)
    {
        *major = HID_DEVICE_LIBRARY_MAJOR_VERSION;
        *minor = HID_DEVICE_LIBRARY_MINOR_VERSION;
        *release = HID_DEVICE_LIBRARY_BUILD_RELEASE;

        status = HID_DEVICE_SUCCESS;
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
// CHIDDevice Class - Protected Methods
////////////////////////////////////////////////////////////////////////////////

// Reset class members
// Called in CHIDDevice(), Open() (failed), and Close()

void CHIDDevice::ResetDeviceData()
{
    // Initialize timeouts to wait for 1000 ms
    m_getReportTimeout = 1000;
    m_setReportTimeout = 1000;

    m_handle = NULL;
    m_deviceOpened = FALSE;
    m_inputReportBufferLength = 0;
    m_outputReportBufferLength = 0;
    m_featureReportBufferLength = 0;
    m_maxReportRequest = 512;

    memset(m_pathStr, 0x00, MAX_PATH_LENGTH);
    memset(m_serialStr, 0x00, MAX_SERIAL_STRING_LENGTH);
    memset(m_manufacturerStr, 0x00, MAX_MANUFACTURER_STRING_LENGTH);
    memset(m_productStr, 0x00, MAX_PRODUCT_STRING_LENGTH);

    m_vid = 0;
    m_pid = 0;
    m_releaseNumber = 0;

    // Empty the read queue
    m_interruptReadQueue->Clear();
}

// Read and cache vid, pid, path, and serial string from the IO registry
// just before the device is actually opened

void CHIDDevice::CacheDeviceStrings(hid_device_info* info)
{
    GetDeviceInfoString(info, HID_PATH_STRING, m_pathStr, MAX_PATH_LENGTH);
    GetDeviceInfoString(info, HID_SERIAL_STRING, m_serialStr, MAX_SERIAL_STRING_LENGTH);
    GetDeviceInfoString(info, HID_MANUFACTURER_STRING, m_manufacturerStr, MAX_MANUFACTURER_STRING_LENGTH);
    GetDeviceInfoString(info, HID_PRODUCT_STRING, m_productStr, MAX_PRODUCT_STRING_LENGTH);

    m_vid = info->vendor_id;
    m_pid = info->product_id;
    m_releaseNumber = info->release_number;
}

// Read and cache the input, output, and feature report sizes from the IO registry
// just before the device is actually opened

void CHIDDevice::CacheReportLengths(hid_device_info* info)
{
    m_inputReportBufferLength = 64;
    m_outputReportBufferLength = 64;
    m_featureReportBufferLength = 64;
}

// Get 1 report (with timeout) and get out

bool CHIDDevice::QueueInterruptReadData(DWORD TimeoutMSec)
{
    DWORD bufferLength  = GetInputReportBufferLength();
    BYTE* buffer        = new BYTE[bufferLength];

    memset(buffer, 0x00, bufferLength);
        
    int bytes_read = hid_read_timed(m_handle, buffer, bufferLength, TimeoutMSec);

    bool Success;
    if (bytes_read == 0)
    {
        Success = true; // Read timed out
    }
    else if (bytes_read > 0)
    {
        // Copy data to internal queue
        m_interruptReadQueue->Enqueue(buffer, bufferLength);
        Success = true;
    }
    else
    {
        Success = false;
    }

    delete [] buffer;

    return Success;
}

////////////////////////////////////////////////////////////////////////////////
// CHIDDevice Class - Public Methods
////////////////////////////////////////////////////////////////////////////////

// Construct a CHIDDevice object
// - Create a new queue (for forward declared class)
// - Reset device

CHIDDevice::CHIDDevice()
{
    // Allocate the read queue
    // The class is forward declared so that the
    // user doesn't have to include ReportQueue.h
    m_interruptReadQueue = new CReportQueue();

    ResetDeviceData();
}

// Destruct a CHIDDevice object
// - Stop interrupt read thread
// - Close the device
// - Deallocate the queue

CHIDDevice::~CHIDDevice()
{
    // Make sure that the device is closed
    Close();

    // Deallocate the read queue
    delete m_interruptReadQueue;
}

// Open a device based on its device index and vid/pid
// Specify the number of input buffers (used to determine interrupt read queue size)
// - Create an HID device iterator based on vid, pid, usage page, and usage
// - Iterate until the specified device index is reached
// - Cache vid, pid, path, and serial strings
// - Cache input, output, and feature report sizes
// - Open the device
// - Launch the interrupt read thread

BYTE CHIDDevice::Open(DWORD deviceIndex, WORD vid, WORD pid, DWORD numInputBuffers)
{
    BYTE status = HID_DEVICE_NOT_FOUND;

    hid_device_info* infos = hid_enumerate(vid, pid);
    DWORD curIndex = 0;

    if (infos)
    {
        hid_device_info* cur = infos;

        while (cur)
        {
            if (curIndex == deviceIndex)
            {
                // Store the vid, pid, path, and serial strings
                CacheDeviceStrings(cur);
                CacheReportLengths(cur);

                // Maximum number of input interrupt reports to queue
                m_maxReportRequest = numInputBuffers;

                // Open the device handle
                m_handle = hid_open_path(cur->path);

                if (m_handle)
                {
                    m_deviceOpened = TRUE;

                    // Reserve space in the queue for max number of input interrupt reports
                    m_interruptReadQueue->Reserve(m_maxReportRequest * m_inputReportBufferLength);

                    status = HID_DEVICE_SUCCESS;
                }
                else
                {
                    // Reset on error
                    ResetDeviceData();
                }

                break;
            }

            cur = cur->next;
            curIndex++;
        }

        hid_free_enumeration(infos);
    }

    return status;
}

// Close the device
// - Terminate the interrupt read thread
// - Close the device handle

BYTE CHIDDevice::Close()
{
    BYTE status = HID_DEVICE_UNKNOWN_ERROR;

    // Only attempt to close the device if it is already opened
    if (m_deviceOpened)
    {
        // Make sure that the handle is valid
        if (m_handle != NULL)
        {
            // Close the handle
            hid_close(m_handle);
            status = HID_DEVICE_SUCCESS;
        }
        else
        {
            status = HID_DEVICE_HANDLE_ERROR;
        }

        // Reset internal members
        ResetDeviceData();
    }
    else
    {
        status = HID_DEVICE_NOT_OPENED;
    }

    return status;
}

// Return TRUE if the device is currently opened

BOOL CHIDDevice::IsOpened()
{
    return m_deviceOpened;
}

// Return the device handle (IOHIDDeviceInterface**)

HANDLE CHIDDevice::GetHandle()
{
    return m_handle;
}

// Return the specifiedy device string (vid, pid, path, serial, manufacturer, or product string)

BYTE CHIDDevice::GetString(BYTE hidStringType, char* deviceString, DWORD deviceStringLength)
{
    BYTE status = HID_DEVICE_INVALID_BUFFER_SIZE;

    if (m_deviceOpened)
    {
        if (deviceString)
        {
            switch (hidStringType)
            {
            case HID_VID_STRING:
                if (deviceStringLength >= MAX_VID_LENGTH)
                {
                    sprintf(deviceString, "%04x", m_vid);
                    status = HID_DEVICE_SUCCESS;
                }
                break;

            case HID_PID_STRING:
                if (deviceStringLength >= MAX_PID_LENGTH)
                {
                    sprintf(deviceString, "%04x", m_pid);
                    status = HID_DEVICE_SUCCESS;
                }
                break;

            case HID_PATH_STRING:
                if (deviceStringLength >= MAX_PATH_LENGTH)
                {
                    memcpy(deviceString, m_pathStr, MAX_PATH_LENGTH);
                    status = HID_DEVICE_SUCCESS;
                }
                break;

            case HID_SERIAL_STRING:
                if (deviceStringLength >= MAX_SERIAL_STRING_LENGTH)
                {
                    memcpy(deviceString, m_serialStr, MAX_SERIAL_STRING_LENGTH);
                    status = HID_DEVICE_SUCCESS;
                }
                break;

            case HID_MANUFACTURER_STRING:
                if (deviceStringLength >= MAX_MANUFACTURER_STRING_LENGTH)
                {
                    memcpy(deviceString, m_manufacturerStr, MAX_MANUFACTURER_STRING_LENGTH);
                    status = HID_DEVICE_SUCCESS;
                }
                break;

            case HID_PRODUCT_STRING:
                if (deviceStringLength >= MAX_PRODUCT_STRING_LENGTH)
                {
                    memcpy(deviceString, m_productStr, MAX_PRODUCT_STRING_LENGTH);
                    status = HID_DEVICE_SUCCESS;
                }
                break;

            default:
                status = HID_DEVICE_CANNOT_GET_HID_INFO;
            }
        }
    }
    else
    {
        status = HID_DEVICE_NOT_OPENED;
    }

    return status;
}

BYTE CHIDDevice::GetIndexedString(DWORD stringIndex, char* deviceString, DWORD deviceStringLength)
{
    BYTE status = HID_DEVICE_NOT_OPENED;
    wchar_t stringBuf[MAX_INDEXED_STRING_LENGTH];

    // Since we are acting on the object here, only get the strings if we
    // have a device opened, otherwise nothing is targeted
    if (m_deviceOpened)
    {
        if (deviceStringLength < MAX_INDEXED_STRING_LENGTH)
        {
            status = HID_DEVICE_INVALID_BUFFER_SIZE;
        }
        else
        {
            // Reset the supplied string
            memset(deviceString, 0, deviceStringLength);

            if (hid_get_indexed_string(m_handle, stringIndex, stringBuf, MAX_INDEXED_STRING_LENGTH) != -1)
            {
                // Convert the string from unicode to ansi
                for (DWORD i = 0; i < MAX_INDEXED_STRING_LENGTH; i++)
                {
                    deviceString[i] = stringBuf[i];
                }

                // Return success
                status = HID_DEVICE_SUCCESS;
            }
            else
            {
                status = HID_DEVICE_CANNOT_GET_HID_INFO;
            }
        }
    }

    return status;
}

BYTE CHIDDevice::GetAttributes(WORD* deviceVid, WORD* devicePid, WORD* deviceReleaseNumber)
{
    BYTE status = HID_DEVICE_INVALID_BUFFER_SIZE;

    // Make sure that the device is opened
    if (m_deviceOpened)
    {
        // Check pointers
        if (deviceVid && devicePid && deviceReleaseNumber)
        {
            *deviceVid = m_vid;
            *devicePid = m_pid;
            *deviceReleaseNumber = m_releaseNumber;

            status = HID_DEVICE_SUCCESS;
        }
    }
    else
    {
        status = HID_DEVICE_NOT_OPENED;
    }

    return status;
}

// Output feature report using the control pipe

BYTE CHIDDevice::SetFeatureReport_Control(BYTE* buffer, DWORD bufferSize)
{
    BYTE status = HID_DEVICE_UNKNOWN_ERROR;
    int result;

    // Make sure that the pointer is valid
    if (buffer && bufferSize > 1)
    {
        // Make sure the device is opened
        if (m_deviceOpened)
        {
            // Set the requested feature report
            result = hid_send_feature_report(m_handle, buffer, bufferSize);

            if (result != -1)
            {
                status = HID_DEVICE_SUCCESS;
            }
            else
            {
                status = HID_DEVICE_TRANSFER_FAILED;
            }
        }
        else
        {
            status = HID_DEVICE_NOT_OPENED;
        }
    }
    else
    {
        status = HID_DEVICE_INVALID_BUFFER_SIZE;
    }

    return status;
}

// Receive input feature report using the control pipe

BYTE CHIDDevice::GetFeatureReport_Control(BYTE* buffer, DWORD bufferSize)
{
    BYTE status = HID_DEVICE_UNKNOWN_ERROR;
    int result;

    // Make sure that the pointer is valid
    if (buffer && bufferSize > 1)
    {
        // Make sure the device is opened
        if (m_deviceOpened)
        {
            memset(buffer + 1, 0x00, bufferSize - 1);

            // Get the requested feature report
            result = hid_get_feature_report(m_handle, buffer, bufferSize);

            if (result != -1)
            {
                status = HID_DEVICE_SUCCESS;
            }
            else
            {
                status = HID_DEVICE_TRANSFER_FAILED;
            }
        }
        else
        {
            status = HID_DEVICE_NOT_OPENED;
        }
    }
    else
    {
        status = HID_DEVICE_INVALID_BUFFER_SIZE;
    }

    return status;
}

// Ouput report using interrupt pipe

BYTE CHIDDevice::SetOutputReport_Interrupt(BYTE* buffer, DWORD bufferSize)
{
    BYTE status = HID_DEVICE_UNKNOWN_ERROR;
    int result;

    if (!buffer || !bufferSize)
    {
        return HID_DEVICE_INVALID_BUFFER_SIZE;
    }
    if (bufferSize > m_outputReportBufferLength)
    {
        // Allow reports for sigle-report devices be 1 byte longer.
        // The first byte will be stripped before sending.
        if( buffer[0] != INVAL_REPORT_ID || bufferSize > m_outputReportBufferLength + 1)
        {
            return HID_DEVICE_INVALID_BUFFER_SIZE;
        }
    }
    if (!m_deviceOpened)
    {
        status = HID_DEVICE_NOT_OPENED;
    }

    // Set the requested output report (using interrupt pipe)
    result = hid_write(m_handle, buffer, bufferSize);

    if (result != -1)
    {
        status = HID_DEVICE_SUCCESS;
    }
    else
    {
        status = HID_DEVICE_TRANSFER_FAILED;
    }

    return status;
}

// Receive input report using interrupt pipe

BYTE CHIDDevice::GetInputReport_Interrupt(BYTE* buffer, DWORD bufferSize, DWORD numReports, DWORD* bytesReturned)
{
    return GetInputReport_Interrupt_WithTimeout(buffer, bufferSize, numReports, bytesReturned, m_getReportTimeout);
}

BYTE CHIDDevice::GetInputReport_Interrupt_WithTimeout(BYTE* buffer, DWORD bufferSize, DWORD numReports, DWORD* bytesReturned, DWORD getReportTimeout)
{
    BYTE status = HID_DEVICE_UNKNOWN_ERROR;
    DWORD bytesRequested = m_inputReportBufferLength * numReports;

    // Make sure that the parameters are valid
    if (buffer &&
            bufferSize >= bytesRequested &&
            numReports <= m_maxReportRequest &&
            bytesReturned)
    {
        // Clear out the user buffer
        memset(buffer, 0x00, bufferSize);

        *bytesReturned = 0;

        // Check to see that the device is opened
        if (m_deviceOpened)
        {
            // Get the start time to implement read timeouts
            DWORD startTime        = GetTickCount();
            DWORD elapsTime        = 0;
            do
            {
            	// Check if there is enough data in the queue to satisfy the request
            	if (m_interruptReadQueue->Size() >= bytesRequested)
            	{
            		break;
            	}

            	// Read data from the interrupt pipe to m_interruptReadQueue
            	if (!QueueInterruptReadData(getReportTimeout - elapsTime))
            	{
            		return HID_DEVICE_TRANSFER_FAILED;
            	}
            	// GetTickCount rolls over every few days, but it's not a problem - the unsigned
            	// subtraction will still produce the correct value. Unless it rolls over twice,
            	// which can't happen because timeouts themselves are not large enough.
            	elapsTime = GetTickCount() - startTime;

            } while (getReportTimeout > elapsTime);

            // Calculate number of bytes to return to the user
            DWORD bytesRead = 0;
 
            // Copy data to the user buffer
            // Get the number of bytes copied
            m_interruptReadQueue->Dequeue(buffer, bytesRequested, &bytesRead);

            // Return the number of bytes read
            *bytesReturned = bytesRead;

            if (bytesRead == bytesRequested)
            {
                // The loop ended because enough data was received
                status = HID_DEVICE_SUCCESS;
            }
            else
            {
                // The loop ended because enough time passed.
                // Still the caller will get whatever data was received.
                status = HID_DEVICE_TRANSFER_TIMEOUT;
            }
        }
        else
        {
            status = HID_DEVICE_NOT_OPENED;
        }
    }
    else
    {
        status = HID_DEVICE_INVALID_BUFFER_SIZE;
    }

    return status;
}

// Flush the input report receive path (via interrupt pipe)

BYTE CHIDDevice::FlushBuffers()
{
    BYTE status = HID_DEVICE_UNKNOWN_ERROR;

    // Clear the interrupt read queue
    m_interruptReadQueue->Clear();

    return status;
}

// Return the timeouts used for GetInputReport_Interrupt()
// and SetOutputReport_Interrupt()

void CHIDDevice::GetTimeouts(DWORD* getReportTimeout, DWORD* setReportTimeout)
{
    if (getReportTimeout && setReportTimeout)
    {
        *getReportTimeout = m_getReportTimeout;
        *setReportTimeout = m_setReportTimeout;
    }
}

// Set the timeouts used for GetInputReport_Interrupt()
// and SetOutputReport_Interrupt()
// If getReportTimeout is INFINITE, then GetInputReport_Interrupt()
// will wait indefinitely until the specified number of bytes are received
// If setReportTimeout is INFINITE, then SetInputReport_Interrupt() will wait
// 0xffffffff ms before timing out

void CHIDDevice::SetTimeouts(DWORD getReportTimeout, DWORD setReportTimeout)
{
    m_getReportTimeout = getReportTimeout;
    m_setReportTimeout = setReportTimeout;
}
