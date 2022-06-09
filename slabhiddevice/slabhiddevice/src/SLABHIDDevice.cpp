////////////////////////////////////////////////////////////////////////////////
// SLAB_HID_Device_C.cpp
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include "HIDDevice.h"
#include "SLABHIDDevice.h"
#include "DeviceList.h"
#include "OsDep.h"

////////////////////////////////////////////////////////////////////////////////
// Static Global Variables
////////////////////////////////////////////////////////////////////////////////

// Each time Construct()/Destruct() are called
// DeviceList will be updated to track the use
// of heap memory (stores object pointers)
//
// The CHIDDeviceList destructor will automatically
// free any remaining devices
static CDeviceList<CHIDDevice> DeviceList;

////////////////////////////////////////////////////////////////////////////////
// constructor is called after the executable is loaded, before main()
// destructor is called before the executable is unloaded, after main()
////////////////////////////////////////////////////////////////////////////////

__attribute__((constructor))
static void Initializer()
{
    hid_init();
}

__attribute__((destructor))
static void Finalizer()
{
    hid_exit();
}

////////////////////////////////////////////////////////////////////////////////
// Exported Functions
////////////////////////////////////////////////////////////////////////////////

DWORD HidDevice_GetNumHidDevices(WORD vid, WORD pid)
{
    return GetNumHidDevices(vid, pid);
}

BYTE HidDevice_GetHidString(DWORD deviceIndex, WORD vid, WORD pid, BYTE hidStringType, char* deviceString, DWORD deviceStringLength)
{
    return GetHidString(deviceIndex, vid, pid, hidStringType, deviceString, deviceStringLength);
}

BYTE HidDevice_GetHidIndexedString(DWORD deviceIndex, WORD vid, WORD pid, DWORD stringIndex, char* deviceString, DWORD deviceStringLength)
{
    return GetHidIndexedString(deviceIndex, vid, pid, stringIndex, deviceString, deviceStringLength);
}

BYTE HidDevice_GetHidAttributes(DWORD deviceIndex, WORD vid, WORD pid, WORD* deviceVid, WORD* devicePid, WORD* deviceReleaseNumber)
{
    return GetHidAttributes(deviceIndex, vid, pid, deviceVid, devicePid, deviceReleaseNumber);
}

void HidDevice_GetHidGuid(void* hidGuid)
{
    // Not Supported
}

BYTE HidDevice_GetHidLibraryVersion(BYTE* major, BYTE* minor, BOOL* release)
{
    return GetHidLibraryVersion(major, minor, release);
}

BYTE HidDevice_Open(HID_DEVICE* device, DWORD deviceIndex, WORD vid, WORD pid, DWORD numInputBuffers)
{
    // Create a new CHIDDevice object on the heap
    CHIDDevice* hid = DeviceList.Construct();

    if (DeviceList.Validate(hid))
    {
        // Return the newly constructed object pointer
        *device = hid;

        return hid->Open(deviceIndex, vid, pid, numInputBuffers);
    }
    else
    {
        return HID_DEVICE_NOT_FOUND;
    }
}

BOOL HidDevice_IsOpened(HID_DEVICE device)
{
    CHIDDevice* hid = (CHIDDevice*) device;

    if (DeviceList.Validate(hid))
    {
        return hid->IsOpened();
    }
    else
    {
        return FALSE;
    }
}

HANDLE HidDevice_GetHandle(HID_DEVICE device)
{
    CHIDDevice* hid = (CHIDDevice*) device;

    if (DeviceList.Validate(hid))
    {
        return hid->GetHandle();
    }
    else
    {
        return NULL;
    }
}

BYTE HidDevice_GetString(HID_DEVICE device, BYTE hidStringType, char* deviceString, DWORD deviceStringLength)
{
    CHIDDevice* hid = (CHIDDevice*) device;

    if (DeviceList.Validate(hid))
    {
        return hid->GetString(hidStringType, deviceString, deviceStringLength);
    }
    else
    {
        return HID_DEVICE_NOT_FOUND;
    }
}

BYTE HidDevice_GetIndexedString(HID_DEVICE device, DWORD stringIndex, char* deviceString, DWORD deviceStringLength)
{
    CHIDDevice* hid = (CHIDDevice*) device;

    if (DeviceList.Validate(hid))
    {
        return hid->GetIndexedString(stringIndex, deviceString, deviceStringLength);
    }
    else
    {
        return HID_DEVICE_NOT_FOUND;
    }
}

BYTE HidDevice_GetAttributes(HID_DEVICE device, WORD* deviceVid, WORD* devicePid, WORD* deviceReleaseNumber)
{
    CHIDDevice* hid = (CHIDDevice*) device;

    if (DeviceList.Validate(hid))
    {
        return hid->GetAttributes(deviceVid, devicePid, deviceReleaseNumber);
    }
    else
    {
        return HID_DEVICE_NOT_FOUND;
    }
}

BYTE HidDevice_SetFeatureReport_Control(HID_DEVICE device, BYTE* buffer, DWORD bufferSize)
{
    CHIDDevice* hid = (CHIDDevice*) device;

    if (DeviceList.Validate(hid))
    {
        return hid->SetFeatureReport_Control(buffer, bufferSize);
    }
    else
    {
        return HID_DEVICE_NOT_FOUND;
    }
}

BYTE HidDevice_GetFeatureReport_Control(HID_DEVICE device, BYTE* buffer, DWORD bufferSize)
{
    CHIDDevice* hid = (CHIDDevice*) device;

    if (DeviceList.Validate(hid))
    {
        return hid->GetFeatureReport_Control(buffer, bufferSize);
    }
    else
    {
        return HID_DEVICE_NOT_FOUND;
    }
}

BYTE HidDevice_SetOutputReport_Interrupt(HID_DEVICE device, BYTE* buffer, DWORD bufferSize)
{
    CHIDDevice* hid = (CHIDDevice*) device;

    if (DeviceList.Validate(hid))
    {
        return hid->SetOutputReport_Interrupt(buffer, bufferSize);
    }
    else
    {
        return HID_DEVICE_NOT_FOUND;
    }
}

BYTE HidDevice_GetInputReport_Interrupt(HID_DEVICE device, BYTE* buffer, DWORD bufferSize, DWORD numReports, DWORD* bytesReturned)
{
    CHIDDevice* hid = (CHIDDevice*) device;

    if (DeviceList.Validate(hid))
    {
        return hid->GetInputReport_Interrupt(buffer, bufferSize, numReports, bytesReturned);
    }
    else
    {
        return HID_DEVICE_NOT_FOUND;
    }
}

BYTE HidDevice_GetInputReport_Interrupt_WithTimeout(HID_DEVICE device, BYTE* buffer, DWORD bufferSize, DWORD numReports, DWORD* bytesReturned, DWORD TimeoutMSec)
{
	CHIDDevice* hid = (CHIDDevice*)device;

	if (DeviceList.Validate(hid))
	{
		return hid->GetInputReport_Interrupt_WithTimeout(buffer, bufferSize, numReports, bytesReturned, TimeoutMSec);
	}
	else
	{
		return HID_DEVICE_NOT_FOUND;
	}
}

BYTE HidDevice_SetOutputReport_Control(HID_DEVICE device, BYTE* buffer, DWORD bufferSize)
{
    return HID_DEVICE_UNSUPPORTED_FUNCTION;
}

BYTE HidDevice_GetInputReport_Control(HID_DEVICE device, BYTE* buffer, DWORD bufferSize)
{
    return HID_DEVICE_UNSUPPORTED_FUNCTION;
}

WORD HidDevice_GetInputReportBufferLength(HID_DEVICE device)
{
    CHIDDevice* hid = (CHIDDevice*) device;

    if (DeviceList.Validate(hid))
    {
        return hid->GetInputReportBufferLength();
    }
    else
    {
        return 0;
    }
}

WORD HidDevice_GetOutputReportBufferLength(HID_DEVICE device)
{
    CHIDDevice* hid = (CHIDDevice*) device;

    if (DeviceList.Validate(hid))
    {
        return hid->GetOutputReportBufferLength();
    }
    else
    {
        return 0;
    }
}

WORD HidDevice_GetFeatureReportBufferLength(HID_DEVICE device)
{
    CHIDDevice* hid = (CHIDDevice*) device;

    if (DeviceList.Validate(hid))
    {
        return hid->GetFeatureReportBufferLength();
    }
    else
    {
        return 0;
    }
}

DWORD HidDevice_GetMaxReportRequest(HID_DEVICE device)
{
    CHIDDevice* hid = (CHIDDevice*) device;

    if (DeviceList.Validate(hid))
    {
        return hid->GetMaxReportRequest();
    }
    else
    {
        return 0;
    }
}

BOOL HidDevice_FlushBuffers(HID_DEVICE device)
{
    CHIDDevice* hid = (CHIDDevice*) device;

    if (DeviceList.Validate(hid))
    {
        return hid->FlushBuffers();
    }
    else
    {
        return FALSE;
    }
}

BOOL HidDevice_CancelIo(HID_DEVICE device)
{
    // Not Supported
    return FALSE;
}

void HidDevice_GetTimeouts(HID_DEVICE device, DWORD* getReportTimeout, DWORD* setReportTimeout)
{
    CHIDDevice* hid = (CHIDDevice*) device;

    if (DeviceList.Validate(hid))
    {
        hid->GetTimeouts(getReportTimeout, setReportTimeout);
    }
    else
    {
        *getReportTimeout = 0;
        *setReportTimeout = 0;
    }
}

void HidDevice_SetTimeouts(HID_DEVICE device, DWORD getReportTimeout, DWORD setReportTimeout)
{
    CHIDDevice* hid = (CHIDDevice*) device;

    if (DeviceList.Validate(hid))
    {
        hid->SetTimeouts(getReportTimeout, setReportTimeout);
    }
}

BYTE HidDevice_Close(HID_DEVICE device)
{
    CHIDDevice* hid = (CHIDDevice*) device;

    if (DeviceList.Validate(hid))
    {
        BYTE status = hid->Close();

        // Deallocate the device object
        DeviceList.Destruct(hid);

        return status;
    }
    else
    {
        return HID_DEVICE_NOT_FOUND;
    }
}
