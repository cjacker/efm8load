////////////////////////////////////////////////////////////////////////////////
// HidDevice.h
////////////////////////////////////////////////////////////////////////////////

#ifndef HID_DEVICE_H
#define HID_DEVICE_H

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include "Types.h"
#include "hidapi.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

// Return codes
#define HID_DEVICE_SUCCESS					0x00
#define HID_DEVICE_NOT_FOUND					0x01
#define HID_DEVICE_NOT_OPENED					0x02
#define HID_DEVICE_ALREADY_OPENED				0x03
#define HID_DEVICE_TRANSFER_TIMEOUT				0x04
#define HID_DEVICE_TRANSFER_FAILED				0x05
#define HID_DEVICE_CANNOT_GET_HID_INFO                          0x06
#define HID_DEVICE_HANDLE_ERROR					0x07
#define HID_DEVICE_INVALID_BUFFER_SIZE                          0x08
#define HID_DEVICE_SYSTEM_CODE					0x09
#define HID_DEVICE_UNKNOWN_ERROR				0xFF

// String types
#define HID_VID_STRING						0x01
#define HID_PID_STRING						0x02
#define HID_PATH_STRING						0x03
#define HID_SERIAL_STRING					0x04
#define HID_MANUFACTURER_STRING					0x05
#define HID_PRODUCT_STRING					0x06

// String lengths
#define MAX_VID_LENGTH						5
#define MAX_PID_LENGTH						5
#define MAX_PATH_LENGTH						512
#define MAX_SERIAL_STRING_LENGTH				256
#define MAX_MANUFACTURER_STRING_LENGTH                          256
#define MAX_PRODUCT_STRING_LENGTH				256
#define MAX_INDEXED_STRING_LENGTH                               256
#define MAX_STRING_LENGTH					512

////////////////////////////////////////////////////////////////////////////////
// Forward Class Declarations
////////////////////////////////////////////////////////////////////////////////

//class CReadThreadParam;
class CReportQueue;

////////////////////////////////////////////////////////////////////////////////
// Global Functions
////////////////////////////////////////////////////////////////////////////////

DWORD GetNumHidDevices(WORD vid, WORD pid);
BYTE GetHidString(DWORD deviceIndex, WORD vid, WORD pid, BYTE hidStringType, char* deviceString, DWORD deviceStringLength);
BYTE GetHidIndexedString(DWORD deviceIndex, WORD vid, WORD pid, DWORD stringIndex, char* deviceString, DWORD deviceStringLength);
BYTE GetHidAttributes(DWORD deviceIndex, WORD vid, WORD pid, WORD* deviceVid, WORD* devicePid, WORD* deviceReleaseNumber);
BYTE GetHidLibraryVersion(BYTE* major, BYTE* minor, BOOL* release);

////////////////////////////////////////////////////////////////////////////////
// HID Device Class Definition
////////////////////////////////////////////////////////////////////////////////

class CHIDDevice
{
// Constructor/Destructor
public:
	CHIDDevice();
	virtual ~CHIDDevice();
	
// Public Methods
public:
	BYTE Open(DWORD deviceIndex, WORD vid, WORD pid, DWORD numInputBuffers = 512);
	BYTE Close();
	BOOL IsOpened();
	HANDLE GetHandle();
	
	BYTE GetString(BYTE hidStringType, char* deviceString, DWORD deviceStringLength);
        BYTE GetIndexedString(DWORD stringIndex, char* deviceString, DWORD deviceStringLength);
	BYTE GetAttributes(WORD* deviceVid, WORD* devicePid, WORD* deviceReleaseNumber);
	
	BYTE SetFeatureReport_Control(BYTE* buffer, DWORD bufferSize);
	BYTE GetFeatureReport_Control(BYTE* buffer, DWORD bufferSize);
	BYTE SetOutputReport_Interrupt(BYTE* buffer, DWORD bufferSize);
	BYTE GetInputReport_Interrupt(BYTE* buffer, DWORD bufferSize, DWORD numReports, DWORD* bytesReturned);
	BYTE GetInputReport_Interrupt_WithTimeout(BYTE* buffer, DWORD bufferSize, DWORD numReports, DWORD* bytesReturned, DWORD TimeoutMSec);
	
	// Return the input report size (input via interrupt pipe)
    WORD GetInputReportBufferLength() const { return m_inputReportBufferLength; }
	// Return the output report size (output via interrupt pipe)
    WORD GetOutputReportBufferLength() const { return m_outputReportBufferLength; }
	// Return the feature report size (input/output via control pipe)
    WORD GetFeatureReportBufferLength() const { return m_featureReportBufferLength; }
    // Return the maximum number of input reports that can be received (input via interrupt pipe)
	DWORD GetMaxReportRequest() const { return m_maxReportRequest; }
	BYTE FlushBuffers();
	
	void GetTimeouts(DWORD* getReportTimeout, DWORD* setReportTimeout);
	void SetTimeouts(DWORD getReportTimeout, DWORD setReportTimeout);

// Protected Methods
protected:
	void ResetDeviceData();
	void CacheDeviceStrings(hid_device_info* info);
	void CacheReportLengths(hid_device_info* info);
	bool QueueInterruptReadData(DWORD TimeoutMSec);
	
// Protected Members
protected:
	hid_device*			m_handle;
	BOOL				m_deviceOpened;
	CReportQueue*                   m_interruptReadQueue;
	
	DWORD				m_getReportTimeout;
	DWORD				m_setReportTimeout;
	
	WORD				m_inputReportBufferLength;
	WORD				m_outputReportBufferLength;
	WORD				m_featureReportBufferLength;
	DWORD				m_maxReportRequest;
	
	char				m_pathStr[MAX_PATH_LENGTH];
	char				m_serialStr[MAX_SERIAL_STRING_LENGTH];
	char				m_manufacturerStr[MAX_MANUFACTURER_STRING_LENGTH];
	char				m_productStr[MAX_PRODUCT_STRING_LENGTH];
	WORD				m_vid;
	WORD				m_pid;
	WORD				m_releaseNumber;
};

#endif // HID_DEVICE_H
