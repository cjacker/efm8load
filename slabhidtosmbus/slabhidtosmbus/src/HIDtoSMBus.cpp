// HIDtoSMBus.cpp : Defines the entry point for the DLL application.
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SLABCP2112.h"
#include "HIDtoSMBusVersion.h"
#include "HIDtoSMBus.h"
#include "UsbReports.h"
#include "DeviceList.h"

#include "silabs_utils.h"

/////////////////////////////////////////////////////////////////////////////
// Definitions
/////////////////////////////////////////////////////////////////////////////

// The SLABHIDDevice read timeout in ms for getting input interrupt reports
#define HID_QUEUE_READ_TIMEOUT				0

// The amount of time in ms to wait before throttling response polling
#define RESPONSE_THROTTLE_DELAY				100

// The sleep delay in ms while throttling
#define RESPONSE_SLEEP_THROTTLE				1

#define FEATURE_REPORT_LENGTH				64
#define INTERRUPT_REPORT_LENGTH				64

/////////////////////////////////////////////////////////////////////////////
// Global Variables
/////////////////////////////////////////////////////////////////////////////

static CDeviceList<CHIDtoSMBus> DeviceList;

/////////////////////////////////////////////////////////////////////////////
// Static Prototypes
/////////////////////////////////////////////////////////////////////////////

static HID_SMBUS_STATUS SetCustomShortString(CHIDtoSMBus* device, BYTE rID, char* ascii, BYTE strlen);
static HID_SMBUS_STATUS GetCustomShortString(CHIDtoSMBus* device, BYTE rID, char* ascii, BYTE* strlen);

/////////////////////////////////////////////////////////////////////////////
// Interrupt Report Prototypes
/////////////////////////////////////////////////////////////////////////////

static BYTE QueueInterruptReports(CHIDtoSMBus* device);

static HID_SMBUS_STATUS SetReadRequest(CHIDtoSMBus* device, BYTE slaveAddress, WORD numBytesToRead);
static HID_SMBUS_STATUS SetAddressReadRequest(CHIDtoSMBus* device, BYTE slaveAddress, WORD numBytesToRead, BYTE targetAddressSize, BYTE targetAddress[16]);
static HID_SMBUS_STATUS SetForceReadResponse(CHIDtoSMBus* device, WORD numBytesToRead);
static HID_SMBUS_STATUS GetReadResponse(_In_ CHIDtoSMBus* device, HID_SMBUS_S0* status, BYTE* buffer, BYTE bufferSize, BYTE* numBytesRead);
static HID_SMBUS_STATUS SetWriteRequest(CHIDtoSMBus* device, BYTE slaveAddress, BYTE* buffer, BYTE numBytesToWrite);
static HID_SMBUS_STATUS SetTransferStatusRequest(CHIDtoSMBus* device);
static HID_SMBUS_STATUS GetTransferStatusResponse(CHIDtoSMBus* device, HID_SMBUS_S0* status, HID_SMBUS_S1* detailedStatus, WORD* numRetries, WORD* bytesRead);
static HID_SMBUS_STATUS SetCancelTransfer(CHIDtoSMBus* device);

/////////////////////////////////////////////////////////////////////////////
// Feature Report Prototypes
/////////////////////////////////////////////////////////////////////////////

_Check_return_
_Ret_range_(HID_DEVICE_SUCCESS, HID_DEVICE_UNKNOWN_ERROR)
_Success_(HID_DEVICE_SUCCESS == return)
static HID_SMBUS_STATUS SetSmbusConfig(CHIDtoSMBus* device, _In_ const DWORD bitRate, _In_ const BYTE address, _In_ const BOOL autoReadRespond, _In_ const WORD writeTimeout, _In_ const WORD readTimeout, _In_ const BOOL sclLowTimeout, _In_ const WORD transferRetries);
_Check_return_
_Ret_range_(HID_DEVICE_SUCCESS, HID_DEVICE_UNKNOWN_ERROR)
_Success_(HID_DEVICE_SUCCESS == return)
static HID_SMBUS_STATUS GetSmbusConfig(CHIDtoSMBus* device, _Out_writes_bytes_(sizeof(DWORD)) DWORD* bitRate, _Out_writes_bytes_(sizeof(BYTE)) BYTE* address, _Out_writes_bytes_(sizeof(BOOL)) BOOL* autoReadRespond, _Out_writes_bytes_(sizeof(WORD)) WORD* writeTimeout, _Out_writes_bytes_(sizeof(WORD)) WORD* readTimeout, _Out_writes_bytes_(sizeof(BOOL)) BOOL* sclLowTimeout, _Out_writes_bytes_(sizeof(WORD)) WORD* transferRetries);

static HID_SMBUS_STATUS SetReset(CHIDtoSMBus* device);

_Check_return_
_Ret_range_(HID_DEVICE_SUCCESS, HID_DEVICE_UNKNOWN_ERROR)
_Success_(HID_DEVICE_SUCCESS == return)
static HID_SMBUS_STATUS SetGpioConfig(CHIDtoSMBus* device, _In_ const BYTE direction, _In_ const BYTE mode, _In_ const BYTE function, _In_ const BYTE clkDiv);
_Check_return_
_Ret_range_(HID_DEVICE_SUCCESS, HID_DEVICE_UNKNOWN_ERROR)
_Success_(HID_DEVICE_SUCCESS == return)
static HID_SMBUS_STATUS GetGpioConfig(CHIDtoSMBus* device, _Out_writes_bytes_(sizeof(BYTE)) BYTE* direction, _Out_writes_bytes_(sizeof(BYTE)) BYTE* mode, _Out_writes_bytes_(sizeof(BYTE)) BYTE* function, _Out_writes_bytes_(sizeof(BYTE)) BYTE* clkDiv);
_Check_return_
_Ret_range_(HID_DEVICE_SUCCESS, HID_DEVICE_UNKNOWN_ERROR)
_Success_(HID_DEVICE_SUCCESS == return)
static HID_SMBUS_STATUS GetReadLatch(CHIDtoSMBus* device, _Out_writes_bytes_(sizeof(BYTE)) BYTE* latchValue);
_Check_return_
_Ret_range_(HID_DEVICE_SUCCESS, HID_DEVICE_UNKNOWN_ERROR)
_Success_(HID_DEVICE_SUCCESS == return)
static HID_SMBUS_STATUS SetWriteLatch(CHIDtoSMBus* device, _In_ const BYTE latchValue, _In_ const BYTE latchMask);

_Check_return_
_Ret_range_(HID_DEVICE_SUCCESS, HID_DEVICE_UNKNOWN_ERROR)
_Success_(HID_DEVICE_SUCCESS == return)
static HID_SMBUS_STATUS GetPartNumber(CHIDtoSMBus* device, _Out_writes_bytes_(sizeof(BYTE)) BYTE* partNumber, _Out_writes_bytes_(sizeof(BYTE)) BYTE* version);

static HID_SMBUS_STATUS SetLock(CHIDtoSMBus* device, _In_ const BYTE lock);
_Check_return_
_Ret_range_(HID_DEVICE_SUCCESS, HID_DEVICE_UNKNOWN_ERROR)
_Success_(HID_DEVICE_SUCCESS == return)
static HID_SMBUS_STATUS GetLock(CHIDtoSMBus* device, _Out_writes_bytes_(sizeof(BYTE)) BYTE* lock);

static HID_SMBUS_STATUS SetUsbConfig(CHIDtoSMBus* device, _In_ const WORD vid, _In_ const WORD pid, _In_ const BYTE power, _In_ const BYTE powerMode, _In_ const WORD releaseVersion, _In_ const BYTE mask);
_Check_return_
_Ret_range_(HID_DEVICE_SUCCESS, HID_DEVICE_UNKNOWN_ERROR)
_Success_(HID_DEVICE_SUCCESS == return)
static HID_SMBUS_STATUS GetUsbConfig(CHIDtoSMBus* device, _Out_writes_bytes_(sizeof(WORD)) WORD* vid, _Out_writes_bytes_(sizeof(WORD)) WORD* pid, _Out_writes_bytes_(sizeof(BYTE)) BYTE* power, _Out_writes_bytes_(sizeof(BYTE)) BYTE* powerMode, _Out_writes_bytes_(sizeof(WORD)) WORD* releaseVersion);

static HID_SMBUS_STATUS SetManufacturingString(CHIDtoSMBus* device, char* manufacturingString, _In_ const BYTE strlen);
static HID_SMBUS_STATUS GetManufacturingString(CHIDtoSMBus* device, char* manufacturingString, _Out_writes_bytes_(sizeof(BYTE)) BYTE* strlen);

static HID_SMBUS_STATUS SetProductString(CHIDtoSMBus* device, char* productString, _In_ const BYTE strlen);
static HID_SMBUS_STATUS GetProductString(CHIDtoSMBus* device, char* productString, _Out_writes_bytes_(sizeof(BYTE)) BYTE* strlen);

static HID_SMBUS_STATUS SetSerialString(CHIDtoSMBus* device, char* serialString, _In_ const BYTE strlen);
static HID_SMBUS_STATUS GetSerialString(CHIDtoSMBus* device, char* serialString, _Out_writes_bytes_(sizeof(BYTE)) BYTE* strlen);

/////////////////////////////////////////////////////////////////////////////
// Static Functions
/////////////////////////////////////////////////////////////////////////////

// Set feature report for single report string customization
HID_SMBUS_STATUS SetCustomShortString(CHIDtoSMBus* device, const BYTE rID, char* ascii, BYTE strlen)
{
	HID_SMBUS_STATUS status = HID_SMBUS_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	report[0] = rID;				// Report ID
	report[1] = (strlen * 2) + 2;	// String Desc Length
	report[2] = 0x03;				// String Desc ID

	// Convert ASCII to Unicode string
	for (int i = 0; i < strlen; i++)
	{
		report[i*2+3] = ascii[i];
	}

	// Set feature report to the device
	if (HidDevice_SetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		status = HID_SMBUS_SUCCESS;
	}

	return status;
}

// Get feature report for single report string customization
HID_SMBUS_STATUS GetCustomShortString(CHIDtoSMBus* device, const BYTE rID, char* ascii, BYTE* strlen)
{
	HID_SMBUS_STATUS status = HID_SMBUS_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = rID;

	// Get feature report from the device
	if (HidDevice_GetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		// Get string length from report excluding
		// the report ID and 0x03 string descriptor
		// constant
		*strlen = (report[1] - 2) / 2;

		// Allocate a blank full-size unicode string buffer
		BYTE unicode[SHORT_STRING_LEN];
		memset(unicode, 0x00, sizeof(unicode));

		// Retrieve Unicode serial string from the device
		memcpy(unicode, &report[3], SHORT_STRING_LEN);

		// Convert Unicode to ASCII
		for (int i = 0; i < (SHORT_STRING_LEN/2); i++)
		{
			ascii[i] = unicode[i * 2];
		}

		status = HID_SMBUS_SUCCESS;
	}

	return status;
}

/////////////////////////////////////////////////////////////////////////////
// Interrupt Report Functions
/////////////////////////////////////////////////////////////////////////////

// Grab all available interrupt reports queued in the HID device driver
// Store read response and request status interrupt reports
// to separate queues
BYTE QueueInterruptReports(CHIDtoSMBus* device)
{
	DWORD	bytesRead	= 0;
	DWORD	numReports	= HidDevice_GetMaxReportRequest(device->hid);
	DWORD	reportSize	= HidDevice_GetInputReportBufferLength(device->hid);
	DWORD	requestSize = numReports * reportSize;
	BYTE*	buffer		= new BYTE[requestSize];
	BYTE	hidStatus	= HidDevice_GetInputReport_Interrupt(device->hid, buffer, requestSize, numReports, &bytesRead);

	// Successfully read some data
	if (hidStatus == HID_DEVICE_SUCCESS || hidStatus == HID_DEVICE_TRANSFER_TIMEOUT)
	{
		// Iterate through each report
		for (DWORD i = 0; i < bytesRead; i += reportSize)
		{
			// Enqueue read response interrupt reports to the read response queue
			if (buffer[i] == READ_RESPONSE)
			{
				device->queueReadResponse.Enqueue(&buffer[i], reportSize);
			}
			// Enqueue request status interrupt reports to the request status queue
			else if (buffer[i] == STATUS_RESPONSE)
			{
				device->queueTransferStatusResponse.Enqueue(&buffer[i], reportSize);
			}
		}
	}

	delete [] buffer;

	return hidStatus;
}

HID_SMBUS_STATUS SetReadRequest(CHIDtoSMBus* device, BYTE slaveAddress, WORD numBytesToRead)
{
	HID_SMBUS_STATUS status = HID_SMBUS_DEVICE_IO_FAILED;

	BYTE report[INTERRUPT_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Prepare interrupt report
	report[0] = READ_REQUEST;				// Report ID
	report[1] = slaveAddress;				// Slave Address
	report[2] = HIBYTE(numBytesToRead);		// Number of bytes to read (big-endian)
	report[3] = LOBYTE(numBytesToRead);

	// Send feature report to the device
	if (HidDevice_SetOutputReport_Interrupt(device->hid, report, INTERRUPT_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		status = HID_SMBUS_SUCCESS;
	}

	return status;
}

HID_SMBUS_STATUS SetAddressReadRequest(CHIDtoSMBus* device, BYTE slaveAddress, WORD numBytesToRead, BYTE targetAddressSize, BYTE targetAddress[16])
{
	HID_SMBUS_STATUS status = HID_SMBUS_DEVICE_IO_FAILED;

	BYTE report[INTERRUPT_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Prepare interrupt report
	report[0] = ADR_READ_REQUEST;			// Report ID
	report[1] = slaveAddress;				// Slave Address
	report[2] = HIBYTE(numBytesToRead);		// Number of bytes to read (big-endian)
	report[3] = LOBYTE(numBytesToRead);
	report[4] = targetAddressSize;			// Target address size in bytes
	memcpy(&report[5], targetAddress, 16);	// Target address

	// Send feature report to the device
	if (HidDevice_SetOutputReport_Interrupt(device->hid, report, INTERRUPT_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		status = HID_SMBUS_SUCCESS;
	}

	return status;
}

HID_SMBUS_STATUS SetForceReadResponse(CHIDtoSMBus* device, WORD numBytesToRead)
{
	HID_SMBUS_STATUS status = HID_SMBUS_DEVICE_IO_FAILED;

	BYTE report[INTERRUPT_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Prepare interrupt report
	report[0] = READ_FORCE_SEND;			// Report ID
	report[1] = HIBYTE(numBytesToRead);		// Number of bytes to read (big-endian)
	report[2] = LOBYTE(numBytesToRead);

	// Send feature report to the device
	if (HidDevice_SetOutputReport_Interrupt(device->hid, report, INTERRUPT_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		status = HID_SMBUS_SUCCESS;
	}

	return status;
}

_Use_decl_annotations_
static
HID_SMBUS_STATUS GetReadResponse(CHIDtoSMBus* device, HID_SMBUS_S0* status, BYTE* buffer, BYTE, BYTE* numBytesRead)
{
	HID_SMBUS_STATUS	retStatus	= HID_SMBUS_READ_TIMED_OUT;
	const ULONGLONG startTime = SiLabs_Utils_GetStartTime();

	*status			= 0;
	*numBytesRead	= 0;

	do
	{
		// Check for at least one read response report
		if (device->queueReadResponse.Size() >= INTERRUPT_REPORT_LENGTH)
		{
			retStatus = HID_SMBUS_SUCCESS;
			break;
		}

		// Read the max number of interrupt reports (HID read timeout set to 1 ms),
		// Copy read response reports to queueReadResponse
		// Copy transfer status response reports to queueTransferStatusResponse
		BYTE hidStatus = QueueInterruptReports(device);

		// Trap any device object errors
		if (hidStatus != HID_DEVICE_SUCCESS && hidStatus != HID_DEVICE_TRANSFER_TIMEOUT)
		{
			retStatus = HID_SMBUS_READ_ERROR;
			break;
		}

		// Check if we should start throttling QueueInterruptReports() now
		if (SiLabs_Utils_ElapsedTimeSince(startTime) > RESPONSE_THROTTLE_DELAY)
		{
			// Check for at least one read response report
			// (Check again here so we can return immediately without sleeping)
			if (device->queueReadResponse.Size() >= INTERRUPT_REPORT_LENGTH)
			{
				retStatus = HID_SMBUS_SUCCESS;
				break;
			}

			// Start throttling QueueInterruptReports() now by sleeping.
			//
			// This technique provides excellent latency when not throttling.
			// Once we start throttling, the latency will go up (typ. ~15ms).
			//
			// In other words, the CPU will thrash for the first 100 ms to provide the best
			// latency.  After 100 ms has elapsed, the CPU usage will throttle down and provide
			// mediocre latency.
			Sleep(RESPONSE_SLEEP_THROTTLE);
		}

	} while ((device->responseTimeout == 0) || (SiLabs_Utils_ElapsedTimeSince(startTime) < device->responseTimeout));

	if (retStatus == HID_SMBUS_SUCCESS)
	{
		DWORD	bytesDequeued;
		BYTE	report[INTERRUPT_REPORT_LENGTH];
		memset(report, 0x00, INTERRUPT_REPORT_LENGTH);

		// Get a single read response report out of the queue
		device->queueReadResponse.Dequeue(report, INTERRUPT_REPORT_LENGTH, &bytesDequeued);

		// Double check that the dequeued report is the correct size
		if (bytesDequeued == INTERRUPT_REPORT_LENGTH)
		{
			*status			= report[1];
			*numBytesRead	= report[2];
			memcpy(buffer, &report[3], 61);
		}
		else
		{
			retStatus = HID_SMBUS_READ_ERROR;
		}
	}

	return retStatus;
}

HID_SMBUS_STATUS SetWriteRequest(CHIDtoSMBus* device, BYTE slaveAddress, BYTE* buffer, BYTE numBytesToWrite)
{
	HID_SMBUS_STATUS status = HID_SMBUS_WRITE_ERROR;

	BYTE report[INTERRUPT_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Prepare interrupt report
	report[0] = WRITE_DATA;			// Report ID
	report[1] = slaveAddress;		// Slave Address
	report[2] = numBytesToWrite;	// Number of bytes to write
	memcpy(&report[3], buffer, numBytesToWrite);

	// Send feature report to the device
	if (HidDevice_SetOutputReport_Interrupt(device->hid, report, INTERRUPT_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		status = HID_SMBUS_SUCCESS;
	}

	return status;
}

HID_SMBUS_STATUS SetTransferStatusRequest(CHIDtoSMBus* device)
{
	HID_SMBUS_STATUS status = HID_SMBUS_DEVICE_IO_FAILED;

	BYTE report[INTERRUPT_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Prepare interrupt report
	report[0] = STATUS_REQUEST;		// Report ID
	report[1] = 0x01;				// Must be 0x01

	// Send feature report to the device
	if (HidDevice_SetOutputReport_Interrupt(device->hid, report, INTERRUPT_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		status = HID_SMBUS_SUCCESS;
	}

	return status;
}

HID_SMBUS_STATUS GetTransferStatusResponse(CHIDtoSMBus* device, HID_SMBUS_S0* status, HID_SMBUS_S1* detailedStatus, WORD* numRetries, WORD* bytesRead)
{
	HID_SMBUS_STATUS	retStatus	= HID_SMBUS_READ_TIMED_OUT;
	const ULONGLONG startTime	= SiLabs_Utils_GetStartTime();

	do
	{
		// Check for at least one read response report
		if (device->queueTransferStatusResponse.Size() >= INTERRUPT_REPORT_LENGTH)
		{
			retStatus = HID_SMBUS_SUCCESS;
			break;
		}

		// Read the max number of interrupt reports (HID read timeout set to 1 ms),
		// Copy read response reports to queueReadResponse
		// Copy transfer status response reports to queueTransferStatusResponse
		BYTE hidStatus = QueueInterruptReports(device);

		// Trap any device object errors
		if (hidStatus != HID_DEVICE_SUCCESS && hidStatus != HID_DEVICE_TRANSFER_TIMEOUT)
		{
			retStatus = HID_SMBUS_READ_ERROR;
			break;
		}

		// Check if we should start throttling QueueInterruptReports() now
		if (SiLabs_Utils_ElapsedTimeSince(startTime) > RESPONSE_THROTTLE_DELAY)
		{
			// Check for at least one read response report
			// (Check again here so we can return immediately without sleeping)
			if (device->queueReadResponse.Size() >= INTERRUPT_REPORT_LENGTH)
			{
				retStatus = HID_SMBUS_SUCCESS;
				break;
			}

			// Start throttling QueueInterruptReports() now by sleeping.
			//
			// This technique provides excellent latency when not throttling.
			// Once we start throttling, the latency will go up (typ. ~15ms).
			//
			// In other words, the CPU will thrash for the first 100 ms to provide the best
			// latency.  After 100 ms has elapsed, the CPU usage will throttle down and provide
			// mediocre latency.
			Sleep(RESPONSE_SLEEP_THROTTLE);
		}

	} while ((device->responseTimeout == 0) || (SiLabs_Utils_ElapsedTimeSince(startTime) < device->responseTimeout));

	if (retStatus == HID_SMBUS_SUCCESS)
	{
		DWORD	bytesDequeued;
		BYTE	report[INTERRUPT_REPORT_LENGTH];
		memset(report, 0x00, INTERRUPT_REPORT_LENGTH);

		// Get a single transfer status response report out of the queue
		device->queueTransferStatusResponse.Dequeue(report, INTERRUPT_REPORT_LENGTH, &bytesDequeued);

		// Double check that the dequeued report is the correct size
		if (bytesDequeued == INTERRUPT_REPORT_LENGTH)
		{
			*status				= report[1];
			*detailedStatus		= report[2];
			*numRetries			= MAKEWORD(report[4], report[3]);
			*bytesRead			= MAKEWORD(report[6], report[5]);
		}
		else
		{
			retStatus = HID_SMBUS_READ_ERROR;
		}
	}

	return retStatus;
}

HID_SMBUS_STATUS SetCancelTransfer(CHIDtoSMBus* device)
{
	HID_SMBUS_STATUS status = HID_SMBUS_DEVICE_IO_FAILED;

	BYTE report[INTERRUPT_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Prepare interrupt report
	report[0] = CANCEL_TRANSFER;	// Report ID
	report[1] = 0x01;				// Must be 0x01

	// Send feature report to the device
	if (HidDevice_SetOutputReport_Interrupt(device->hid, report, INTERRUPT_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		status = HID_SMBUS_SUCCESS;
	}

	return status;
}

/////////////////////////////////////////////////////////////////////////////
// Feature Report Functions
/////////////////////////////////////////////////////////////////////////////

// Set feature report for HidSmbus_SetSmbusConfig()
_Use_decl_annotations_
static
HID_SMBUS_STATUS
SetSmbusConfig(CHIDtoSMBus* device, DWORD bitRate, BYTE address, BOOL autoReadRespond, WORD writeTimeout, WORD readTimeout, BOOL sclLowTimeout, WORD transferRetries)
{
	HID_SMBUS_STATUS status = HID_SMBUS_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Normalize BOOL values to 0 or 1
	autoReadRespond		= (autoReadRespond != 0) ? TRUE : FALSE;
	sclLowTimeout		= (sclLowTimeout != 0) ? TRUE : FALSE;

	// Prepare feature report
	report[0]	= GS_SMB_CONFIG;				// Report ID
	report[1]	= (BYTE)(bitRate >> 24);		// SCL Bit Rate (big-endian)
	report[2]	= (BYTE)(bitRate >> 16);
	report[3]	= (BYTE)(bitRate >> 8);
	report[4]	= (BYTE)(bitRate);
	report[5]	= address;						// Device slave address for ACK only
	report[6]	= SILABS_CONVERT_BOOL_TO_BYTE(autoReadRespond);				// Device automatically send read response via interrupt report
	report[7]	= (BYTE)(writeTimeout >> 8);	// Write transfer timeout (big-endian)
	report[8]	= (BYTE)(writeTimeout);
	report[9]	= (BYTE)(readTimeout >> 8);		// Read transfer timeout (big-endian)
	report[10]	= (BYTE)(readTimeout);
	report[11]	= SILABS_CONVERT_BOOL_TO_BYTE(sclLowTimeout);				// Reset the SMBus if the SCL line is held low for more than 25 ms
	report[12]	= (BYTE)(transferRetries >> 8);	// Transfer retries (big-endian)
	report[13]  = (BYTE)(transferRetries);

	// Send feature report to the device
	if (HidDevice_SetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		status = HID_SMBUS_SUCCESS;
	}

	return status;
}

// Get feature report for HidSmbus_GetSmbusConfig()
_Use_decl_annotations_
static
HID_SMBUS_STATUS
GetSmbusConfig(CHIDtoSMBus* device, DWORD* bitRate, BYTE* address, BOOL* autoReadRespond, WORD* writeTimeout, WORD* readTimeout, BOOL* sclLowTimeout, WORD* transferRetries)
{
	HID_SMBUS_STATUS status = HID_SMBUS_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = GS_SMB_CONFIG;

	// Get feature report from the device
	if (HidDevice_GetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		*bitRate			= ((DWORD)report[1] << 24) | ((DWORD)report[2] << 16) | ((DWORD)report[3] << 8) | ((DWORD)report[4]);
		*address			= report[5];
		*autoReadRespond	= report[6];
		*writeTimeout		= ((WORD)report[7] << 8) | ((WORD)report[8]);
		*readTimeout		= ((WORD)report[9] << 8) | ((WORD)report[10]);
		*sclLowTimeout		= report[11];
		*transferRetries	= ((WORD)report[12] << 8) | ((WORD)report[13]);

		status = HID_SMBUS_SUCCESS;
	}

	return status;
}

// Set feature report for HidSmbus_Reset()
HID_SMBUS_STATUS SetReset(CHIDtoSMBus* device)
{
	HID_SMBUS_STATUS status = HID_SMBUS_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = RESET_DEVICE;
	report[1] = 0x01;			// Must be 0x01

	// Send feature report to the device
	BYTE hidStatus = HidDevice_SetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH);

	// The device may reset too fast to respond successfully
	if (hidStatus == HID_DEVICE_SUCCESS || hidStatus == HID_DEVICE_TRANSFER_FAILED)
	//if (hidStatus == HID_DEVICE_SUCCESS)
	{
		status = HID_SMBUS_SUCCESS;
	}

	return status;
}

// Set feature report for HidSmbus_SetGpioConfig()
_Use_decl_annotations_
HID_SMBUS_STATUS SetGpioConfig(CHIDtoSMBus* device, const BYTE direction, const BYTE mode, const BYTE function, const BYTE clkDiv)
{
	HID_SMBUS_STATUS status = HID_SMBUS_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = CONFIGURE_GPIO;
	report[1] = direction;
	report[2] = mode;
	report[3] = function;
	report[4] = clkDiv;

	// Set feature report to the device
	if (HidDevice_SetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		status = HID_SMBUS_SUCCESS;
	}

	return status;
}

// Get feature report for HidSmbus_GetGpioConfig()
_Use_decl_annotations_
HID_SMBUS_STATUS GetGpioConfig(CHIDtoSMBus* device, BYTE* direction, BYTE* mode, BYTE* function, BYTE* clkDiv)
{
	HID_SMBUS_STATUS status = HID_SMBUS_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = CONFIGURE_GPIO;

	// Get feature report from the device
	if (HidDevice_GetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		// Extract the GPIO configuration
		*direction	= report[1];
		*mode		= report[2];
		*function	= report[3];
		*clkDiv		= report[4];

		status = HID_SMBUS_SUCCESS;
	}

	return status;
}

// Get feature report for HidSmbus_ReadLatch()
_Use_decl_annotations_
HID_SMBUS_STATUS GetReadLatch(CHIDtoSMBus* device, BYTE* latchValue)
{
	HID_SMBUS_STATUS status = HID_SMBUS_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = GET_GPIO_VAL;

	// Get feature report from the device
	if (HidDevice_GetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		// Extract the GPIO latch values
		*latchValue = report[1];

		status = HID_SMBUS_SUCCESS;
	}

	return status;
}

// Set feature report for HidSmbus_WriteLatch()
_Use_decl_annotations_
HID_SMBUS_STATUS SetWriteLatch(CHIDtoSMBus* device, const BYTE latchValue, const BYTE latchMask)
{
	HID_SMBUS_STATUS status = HID_SMBUS_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = SET_GPIO_VAL;
	report[1] = latchValue;
	report[2] = latchMask;

	// Set feature report to the device
	if (HidDevice_SetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		status = HID_SMBUS_SUCCESS;
	}

	return status;
}

// Get feature report for HidSmbus_GetPartNumber()
_Use_decl_annotations_
HID_SMBUS_STATUS GetPartNumber(CHIDtoSMBus* device, BYTE* partNumber, BYTE* version)
{
	HID_SMBUS_STATUS status = HID_SMBUS_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = GET_VERSION;

	// Get feature report from the device
	if (HidDevice_GetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		// Retrieve version information from the device
		*partNumber	= report[1];
		*version	= report[2];

		status = HID_SMBUS_SUCCESS;
	}

	return status;
}

// Set feature report for HidSmbus_SetLock()
_Use_decl_annotations_
HID_SMBUS_STATUS SetLock(CHIDtoSMBus* device, const BYTE lock)
{
	HID_SMBUS_STATUS status = HID_SMBUS_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = GETSET_LOCK;		// Report ID
	report[1] = lock;				// Lock byte

	// Set feature report to the device
	if (HidDevice_SetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		status = HID_SMBUS_SUCCESS;
	}

	return status;
}

// Get feature report for HidSmbus_GetLock()
_Use_decl_annotations_
HID_SMBUS_STATUS GetLock(CHIDtoSMBus* device, BYTE* lock)
{
	HID_SMBUS_STATUS status = HID_SMBUS_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = GETSET_LOCK;

	// Get feature report from the device
	if (HidDevice_GetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		// Retrieve lock bytes from the device
		*lock = report[1];

		status = HID_SMBUS_SUCCESS;
	}

	return status;
}

// Set feature report for HidSmbus_SetUsbConfig()
_Use_decl_annotations_
HID_SMBUS_STATUS SetUsbConfig(CHIDtoSMBus* device, const WORD vid, const WORD pid, const BYTE power, const BYTE powerMode, const WORD releaseVersion, const BYTE mask)
{
	HID_SMBUS_STATUS status = HID_SMBUS_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = GETSET_USB_CONFIG;			// Report ID
	report[1] = LOBYTE(vid);				// VID (Little-endian)
	report[2] = HIBYTE(vid);
	report[3] = LOBYTE(pid);				// PID (Little-endian)
	report[4] = HIBYTE(pid);
	report[5] = power;						// Power (mA/2)
	report[6] = powerMode;					// Power Mode (0: bus, 1: self)
	report[7] = HIBYTE(releaseVersion);		// Release Version (major.minor)
	report[8] = LOBYTE(releaseVersion);
	report[9] = mask;						// Field mask

	// Set feature report to the device
	if (HidDevice_SetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		status = HID_SMBUS_SUCCESS;
	}

	return status;
}

// Get feature report for HidSmbus_GetUsbConfig()
_Use_decl_annotations_
HID_SMBUS_STATUS GetUsbConfig(CHIDtoSMBus* device, WORD* vid, WORD* pid, BYTE* power, BYTE* powerMode, WORD* releaseVersion)
{
	HID_SMBUS_STATUS status = HID_SMBUS_DEVICE_IO_FAILED;

	BYTE report[FEATURE_REPORT_LENGTH];
	memset(report, 0x00, sizeof(report));

	// Report ID
	report[0] = GETSET_USB_CONFIG;

	// Get feature report from the device
	if (HidDevice_GetFeatureReport_Control(device->hid, report, FEATURE_REPORT_LENGTH) == HID_DEVICE_SUCCESS)
	{
		// Retrieve USB customization from the device
		*vid				= MAKEWORD(report[1], report[2]);	// VID (Little-endian)
		*pid				= MAKEWORD(report[3], report[4]);	// PID (Little-endian)
		*power				= report[5];						// Power (mA/2)
		*powerMode			= report[6];						// Power Mode (0: bus, 1: self)
		*releaseVersion		= MAKEWORD(report[8], report[7]);	// Release Version (major.minor)

		status = HID_SMBUS_SUCCESS;
	}

	return status;
}

// Set feature report for HidSmbus_SetManufacturingString()
_Use_decl_annotations_
HID_SMBUS_STATUS SetManufacturingString(CHIDtoSMBus* device, char* manufacturingString, const BYTE strlen)
{
	HID_SMBUS_STATUS status = SetCustomShortString(device, GETSET_MANUSTR, manufacturingString, strlen);
	return status;
}

// Get feature report for HidSmbus_GetManufacturingString()
_Use_decl_annotations_
HID_SMBUS_STATUS GetManufacturingString(CHIDtoSMBus* device, char* manufacturingString, BYTE* strlen)
{
	HID_SMBUS_STATUS status = GetCustomShortString(device, GETSET_MANUSTR, manufacturingString, strlen);
	return status;
}

// Set feature report for HidSmbus_SetProductString()
_Use_decl_annotations_
HID_SMBUS_STATUS SetProductString(CHIDtoSMBus* device, char* productString, const BYTE strlen)
{
	HID_SMBUS_STATUS status = SetCustomShortString(device, GETSET_PRODSTR, productString, strlen);
	return status;
}

// Get feature report for HidSmbus_GetProductString()
_Use_decl_annotations_
HID_SMBUS_STATUS GetProductString(CHIDtoSMBus* device, char* productString, BYTE* strlen)
{
	HID_SMBUS_STATUS status = GetCustomShortString(device, GETSET_PRODSTR, productString, strlen);
	return status;
}

// Set feature report for HidSmbus_SetSerialString()
_Use_decl_annotations_
HID_SMBUS_STATUS SetSerialString(CHIDtoSMBus* device, char* serialString, const BYTE strlen)
{
	HID_SMBUS_STATUS status = SetCustomShortString(device, GETSET_SERSTR, serialString, strlen);
	return status;
}

// Get feature report for HidSmbus_GetSerialString()
_Use_decl_annotations_
HID_SMBUS_STATUS GetSerialString(CHIDtoSMBus* device, char* serialString, BYTE* strlen)
{
	HID_SMBUS_STATUS status = GetCustomShortString(device, GETSET_SERSTR, serialString, strlen);
	return status;
}

/////////////////////////////////////////////////////////////////////////////
// Exported Library Functions
/////////////////////////////////////////////////////////////////////////////

// HidSmbus_GetNumDevices
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_GetNumDevices(DWORD* lpdwNumDevices, const WORD vid, const WORD pid)
{
	HID_SMBUS_STATUS status = HID_SMBUS_UNKNOWN_ERROR;

	// Check parameters
	if (lpdwNumDevices)
	{
		// Get the number of devices with matching VID/PID
		*lpdwNumDevices = HidDevice_GetNumHidDevices(vid, pid);

		status = HID_SMBUS_SUCCESS;
	}
	else
	{
		status = HID_SMBUS_INVALID_PARAMETER;
	}

	return status;
}

// HidSmbus_GetString
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_GetString(const DWORD deviceNum, const WORD vid, const WORD pid, char* deviceString, const HID_SMBUS_GETSTRING options)
{
	HID_SMBUS_STATUS status = HID_SMBUS_UNKNOWN_ERROR;

	// Check parameters
	if (deviceString && IsValidHID_SMBUS_GETSTRING(options) )
	{
		// Get the product string
		BYTE hidStatus = HidDevice_GetHidString(deviceNum, vid, pid, (BYTE)options, deviceString, HID_SMBUS_DEVICE_STRLEN);

		// Translate HID status to HID_SMBUS status
		switch (hidStatus)
		{
			case HID_DEVICE_SUCCESS:				status = HID_SMBUS_SUCCESS;					break;
			case HID_DEVICE_NOT_FOUND:				status = HID_SMBUS_DEVICE_NOT_FOUND;		break;
			case HID_DEVICE_ALREADY_OPENED:			status = HID_SMBUS_DEVICE_ALREADY_OPENED;	break;
			default:								status = HID_SMBUS_DEVICE_ACCESS_ERROR;		break;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_PARAMETER;
	}

	return status;
}

// HidSmbus_GetOpenedString
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_GetOpenedString(HID_SMBUS_DEVICE device, char* deviceString, const HID_SMBUS_GETSTRING options)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check parameters
		if (deviceString && IsValidHID_SMBUS_GETSTRING(options))
		{
			// Get the product string
			BYTE hidStatus = HidDevice_GetString(hidSmbus->hid, (BYTE)options, deviceString, HID_SMBUS_DEVICE_STRLEN);

			// Translate HID status to HID_SMBUS status
			switch (hidStatus)
			{
				case HID_DEVICE_SUCCESS:				status = HID_SMBUS_SUCCESS;					break;
				default:								status = HID_SMBUS_DEVICE_ACCESS_ERROR;		break;
			}
		}
		else
		{
			status = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_GetIndexedString
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_GetIndexedString(const DWORD deviceNum, const WORD vid, const WORD pid, const DWORD stringIndex, char* deviceString)
{
	HID_SMBUS_STATUS status = HID_SMBUS_UNKNOWN_ERROR;

	// Check parameters
	if (deviceString)
	{
		// Get the indexed string
		BYTE hidStatus = HidDevice_GetHidIndexedString(deviceNum, vid, pid, stringIndex, deviceString, HID_SMBUS_DEVICE_STRLEN);

		// Translate HID status to HID_SMBUS status
		switch (hidStatus)
		{
			case HID_DEVICE_SUCCESS:				status = HID_SMBUS_SUCCESS;					break;
			case HID_DEVICE_NOT_FOUND:				status = HID_SMBUS_DEVICE_NOT_FOUND;		break;
			default:								status = HID_SMBUS_DEVICE_ACCESS_ERROR;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_PARAMETER;
	}

	return status;
}

// HidSmbus_GetOpenedIndexedString
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_GetOpenedIndexedString(HID_SMBUS_DEVICE device, const DWORD stringIndex, char* deviceString)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check parameters
		if (deviceString)
		{
			// Get the indexed string
			BYTE hidStatus = HidDevice_GetIndexedString(hidSmbus->hid, stringIndex, deviceString, HID_SMBUS_DEVICE_STRLEN);

			// Translate HID status to HID_SMBUS status
			switch (hidStatus)
			{
				case HID_DEVICE_SUCCESS:				status = HID_SMBUS_SUCCESS;					break;
				default:								status = HID_SMBUS_DEVICE_ACCESS_ERROR;
			}
		}
		else
		{
			status = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_GetAttributes
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_GetAttributes(const DWORD deviceNum, const WORD vid, const WORD pid, WORD* deviceVid, WORD* devicePid, WORD* deviceReleaseNumber)
{
	HID_SMBUS_STATUS status = HID_SMBUS_UNKNOWN_ERROR;

	// Check parameters
	if (deviceVid && devicePid && deviceReleaseNumber)
	{
		// Get the attributes
		const SLAB_HID_DEVICE_STATUS hidStatus = HidDevice_GetHidAttributes(deviceNum, vid, pid, deviceVid, devicePid, deviceReleaseNumber);

		// Translate HID status to HID_SMBUS status
		switch (hidStatus)
		{
			case HID_DEVICE_SUCCESS:		status = HID_SMBUS_SUCCESS;					break;
			case HID_DEVICE_NOT_FOUND:		status = HID_SMBUS_DEVICE_NOT_FOUND;		break;
			case HID_DEVICE_ALREADY_OPENED:	status = HID_SMBUS_DEVICE_ALREADY_OPENED;	break;
			default:						status = HID_SMBUS_DEVICE_ACCESS_ERROR;		break;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_PARAMETER;
	}

	return status;
}

// HidSmbus_GetOpenedAttributes
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_GetOpenedAttributes(HID_SMBUS_DEVICE device, WORD* deviceVid, WORD* devicePid, WORD* deviceReleaseNumber)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check parameters
		if (deviceVid && devicePid && deviceReleaseNumber)
		{
			// Get the attributes
			BYTE hidStatus = HidDevice_GetAttributes(hidSmbus->hid, deviceVid, devicePid, deviceReleaseNumber);

			// Translate HID status to HID_SMBUS status
			switch (hidStatus)
			{
				case HID_DEVICE_SUCCESS:				status = HID_SMBUS_SUCCESS;					break;
				default:								status = HID_SMBUS_DEVICE_ACCESS_ERROR;
			}
		}
		else
		{
			status = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_Open
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_Open(HID_SMBUS_DEVICE* device, const DWORD deviceNum, const WORD vid, const WORD pid)
{
	HID_SMBUS_STATUS status = HID_SMBUS_UNKNOWN_ERROR;

	// Check parameters
	if (device)
	{
		// Create a new device object and add it to the device list
		CHIDtoSMBus* hidSmbus = DeviceList.Construct();

		// Check device object
		if (DeviceList.Validate(hidSmbus))
		{
			BYTE partNumber;
			BYTE version;

			// Open the device by index (use max input report buffer)
			BYTE hidStatus = HidDevice_Open(&hidSmbus->hid, deviceNum, vid, pid, MAX_REPORT_REQUEST_XP);

			// Openned successfully
			if (hidStatus == HID_DEVICE_SUCCESS)
			{
				// Get part number for future use
				status = GetPartNumber(hidSmbus, &partNumber, &version);

				if (status == HID_SMBUS_SUCCESS)
				{
					// Only support CP2112
					if (partNumber == HID_SMBUS_PART_CP2112)
					{
						// Save part number
						// Used to determine device functionality
						hidSmbus->partNumber = partNumber;
					}
					else
					{
						status = HID_SMBUS_DEVICE_NOT_SUPPORTED;
					}
				}
			}
			// Failed to open device because the device was not found
			else if (hidStatus == HID_DEVICE_NOT_FOUND)
			{
				status = HID_SMBUS_DEVICE_NOT_FOUND;
			}
			// Could not access device (i.e. already opened)
			else
			{
				status = HID_SMBUS_DEVICE_ACCESS_ERROR;
			}

			// Device opened and initialized successfully
			if (status == HID_SMBUS_SUCCESS)
			{
				// Set interrupt read timeouts to 1 ms (return quickly)
				// Set interrupt write timeouts to the default write timeout
				if (hidSmbus->writeTimeout != 0)
				{
					// Write timeout: 1 - 1000 ms
					HidDevice_SetTimeouts(hidSmbus->hid, HID_QUEUE_READ_TIMEOUT, hidSmbus->writeTimeout);
				}
				else
				{
					// Write timeout: 0 (infinite)
					HidDevice_SetTimeouts(hidSmbus->hid, HID_QUEUE_READ_TIMEOUT, INFINITE);
				}

				// Return the device object pointer to the user
				*device = hidSmbus;
			}
			// Failure occurred
			// Clean up
			else
			{
				// Close the handle
				HidDevice_Close(hidSmbus->hid);

				// Delete the device object and
				// remove the device reference from the device list
				DeviceList.Destruct(hidSmbus);
			}
		}
		else
		{
			status = HID_SMBUS_INVALID_DEVICE_OBJECT;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_PARAMETER;
	}

	return status;
}


// HidSmbus_Close
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_Close(HID_SMBUS_DEVICE device)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Close the device
		const SLAB_HID_DEVICE_STATUS hidStatus = HidDevice_Close(hidSmbus->hid);

		switch (hidStatus)
		{
			case HID_DEVICE_SUCCESS:		status = HID_SMBUS_SUCCESS;				break;
			case HID_DEVICE_HANDLE_ERROR:	status = HID_SMBUS_INVALID_HANDLE;		break;
			default:						status = HID_SMBUS_DEVICE_ACCESS_ERROR; break;
		}

		// Deallocate the device object, remove the device reference
		// from the device list
		DeviceList.Destruct(hidSmbus);
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_IsOpened
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_IsOpened(HID_SMBUS_DEVICE device, BOOL* bIsOpened)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check parameters
	if (bIsOpened)
	{
		// Check device object
		if (DeviceList.Validate(hidSmbus))
		{
			// Check pointers
			if (bIsOpened)
			{
				// Check if the device is opened
				*bIsOpened = HidDevice_IsOpened(hidSmbus->hid);

				status = HID_SMBUS_SUCCESS;
			}
			else
			{
				status = HID_SMBUS_INVALID_PARAMETER;
			}
		}
		else
		{
			*bIsOpened = FALSE;
			status = HID_SMBUS_SUCCESS;	// successfully determined that device is invalid (I.e. would be in other circumstances HID_SMBUS_INVALID_DEVICE_OBJECT)
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_PARAMETER;
	}

	return status;
}

// HidSmbus_ReadRequest
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_ReadRequest(HID_SMBUS_DEVICE device, const BYTE slaveAddress, const WORD numBytesToRead)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check parameters
		if (slaveAddress >= HID_SMBUS_MIN_ADDRESS &&
			slaveAddress <= HID_SMBUS_MAX_ADDRESS &&
			((slaveAddress & 0x01) == 0x00))
		{
			// Check request length
			if (numBytesToRead >= HID_SMBUS_MIN_READ_REQUEST_SIZE && numBytesToRead <= HID_SMBUS_MAX_READ_REQUEST_SIZE)
			{
				status = SetReadRequest(hidSmbus, slaveAddress, numBytesToRead);
			}
			else
			{
				status = HID_SMBUS_INVALID_REQUEST_LENGTH;
			}
		}
		else
		{
			status = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_AddressReadRequest
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_AddressReadRequest(HID_SMBUS_DEVICE device, const BYTE slaveAddress, const WORD numBytesToRead, const BYTE targetAddressSize, BYTE targetAddress[16])
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check parameters
		if (slaveAddress >= HID_SMBUS_MIN_ADDRESS &&
			slaveAddress <= HID_SMBUS_MAX_ADDRESS &&
			((slaveAddress & 0x01) == 0x00) &&
			targetAddressSize >= HID_SMBUS_MIN_TARGET_ADDRESS_SIZE &&
			targetAddressSize <= HID_SMBUS_MAX_TARGET_ADDRESS_SIZE)
		{
			// Check request length
			if (numBytesToRead >= HID_SMBUS_MIN_READ_REQUEST_SIZE && numBytesToRead <= HID_SMBUS_MAX_READ_REQUEST_SIZE)
			{
				status = SetAddressReadRequest(hidSmbus, slaveAddress, numBytesToRead, targetAddressSize, targetAddress);
			}
			else
			{
				status = HID_SMBUS_INVALID_REQUEST_LENGTH;
			}
		}
		else
		{
			status = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_ForceReadResponse
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_ForceReadResponse(HID_SMBUS_DEVICE device, const WORD numBytesToRead)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check request length
		if (numBytesToRead >= HID_SMBUS_MIN_READ_REQUEST_SIZE && numBytesToRead <= HID_SMBUS_MAX_READ_REQUEST_SIZE)
		{
			status = SetForceReadResponse(hidSmbus, numBytesToRead);
		}
		else
		{
			status = HID_SMBUS_INVALID_REQUEST_LENGTH;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_ForceReadResponse
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_GetReadResponse(HID_SMBUS_DEVICE device, HID_SMBUS_S0* status, BYTE* buffer, const BYTE bufferSize, BYTE* numBytesRead)
{
	HID_SMBUS_STATUS	retStatus	= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check pointers
		if (status && buffer && numBytesRead)
		{
			// Check parameters
			if (bufferSize >= HID_SMBUS_MAX_READ_RESPONSE_SIZE)
			{
				retStatus = GetReadResponse(hidSmbus, status, buffer, bufferSize, numBytesRead);
			}
			else
			{
				retStatus = HID_SMBUS_INVALID_PARAMETER;
			}
		}
		else
		{
			retStatus = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		retStatus = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return retStatus;
}

// HidSmbus_WriteRequest
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_WriteRequest(HID_SMBUS_DEVICE device, const BYTE slaveAddress, BYTE* buffer, const BYTE numBytesToWrite)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check pointer
		if (buffer)
		{
			// Check parameters
			if (((slaveAddress & 0x01) == 0x00))
			{
				// Check request length
				if (numBytesToWrite >= HID_SMBUS_MIN_WRITE_REQUEST_SIZE && numBytesToWrite <= HID_SMBUS_MAX_WRITE_REQUEST_SIZE)
				{
					status = SetWriteRequest(hidSmbus, slaveAddress, buffer, numBytesToWrite);
				}
				else
				{
					status = HID_SMBUS_INVALID_REQUEST_LENGTH;
				}
			}
			else
			{
				status = HID_SMBUS_INVALID_PARAMETER;
			}
		}
		else
		{
			status = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_TransferStatusRequest
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_TransferStatusRequest(HID_SMBUS_DEVICE device)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Clear all received transfer status reponse interrupt reports
		QueueInterruptReports(hidSmbus);
		hidSmbus->queueTransferStatusResponse.Clear();

		status = SetTransferStatusRequest(hidSmbus);
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_GetTransferStatusResponse
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_GetTransferStatusResponse(HID_SMBUS_DEVICE device, HID_SMBUS_S0* status, HID_SMBUS_S1* detailedStatus, WORD* numRetries, WORD* bytesRead)
{
	HID_SMBUS_STATUS	retStatus	= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check pointers
		if (status && detailedStatus && numRetries && bytesRead)
		{
			retStatus = GetTransferStatusResponse(hidSmbus, status, detailedStatus, numRetries, bytesRead);
		}
		else
		{
			retStatus = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		retStatus = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return retStatus;
}

// HidSmbus_CancelTransfer
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_CancelTransfer(HID_SMBUS_DEVICE device)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		status = SetCancelTransfer(hidSmbus);

		// Clear all received read response interrupt reports
		QueueInterruptReports(hidSmbus);
		hidSmbus->queueReadResponse.Clear();
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_CancelIo
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_CancelIo(HID_SMBUS_DEVICE device)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Cancel pending reads/writes issued in this thread
		if (HidDevice_CancelIo(hidSmbus->hid))
		{
			status = HID_SMBUS_SUCCESS;
		}
		else
		{
			status = HID_SMBUS_DEVICE_IO_FAILED;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_SetTimeouts
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_SetTimeouts(HID_SMBUS_DEVICE device, const DWORD responseTimeout)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Set response timeouts
		hidSmbus->responseTimeout = responseTimeout;

		status = HID_SMBUS_SUCCESS;
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_GetTimeouts
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_GetTimeouts(HID_SMBUS_DEVICE device, DWORD* responseTimeout)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check pointers
		if (responseTimeout)
		{
			// Return response timeout
			*responseTimeout = hidSmbus->responseTimeout;
			
			status = HID_SMBUS_SUCCESS;
		}
		else
		{
			status = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_SetSmbusConfig
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_SetSmbusConfig(HID_SMBUS_DEVICE device, const DWORD bitRate, const BYTE address, const BOOL autoReadRespond, const WORD writeTimeout, const WORD readTimeout, const BOOL sclLowTimeout, const WORD transferRetries)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check parameters
		if (bitRate			>= HID_SMBUS_MIN_BIT_RATE	&&
			address			>= HID_SMBUS_MIN_ADDRESS	&& address		<= HID_SMBUS_MAX_ADDRESS && ((address & 0x01) == 0x00) &&
			writeTimeout	>= HID_SMBUS_MIN_TIMEOUT	&& writeTimeout <= HID_SMBUS_MAX_TIMEOUT &&
			readTimeout		>= HID_SMBUS_MIN_TIMEOUT	&& readTimeout	<= HID_SMBUS_MAX_TIMEOUT &&
			transferRetries <= HID_SMBUS_MAX_RETRIES)
		{
			// Set read and write timeout timeouts
			// These are the overall read and write timeouts
			hidSmbus->readTimeout	= readTimeout;
			hidSmbus->writeTimeout	= writeTimeout;

			// Set the HID get/set interrupt report timeouts
			if (writeTimeout != 0)
			{
				// Write timeout: 1 - 1000 ms
				HidDevice_SetTimeouts(hidSmbus->hid, HID_QUEUE_READ_TIMEOUT, writeTimeout);
			}
			else
			{
				// Write timeout: 0 (infinite)
				HidDevice_SetTimeouts(hidSmbus->hid, HID_QUEUE_READ_TIMEOUT, INFINITE);
			}

			status = SetSmbusConfig(hidSmbus, bitRate, address, autoReadRespond, writeTimeout, readTimeout, sclLowTimeout, transferRetries);
		}
		else
		{
			status = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_GetSmbusConfig
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_GetSmbusConfig(HID_SMBUS_DEVICE device, DWORD* bitRate, BYTE* address, BOOL* autoReadRespond, WORD* writeTimeout, WORD* readTimeout, BOOL* sclLowTimeout, WORD* transferRetries)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check pointers
		if (bitRate && address && autoReadRespond && writeTimeout && readTimeout && sclLowTimeout && transferRetries)
		{
			status = GetSmbusConfig(hidSmbus, bitRate, address, autoReadRespond, writeTimeout, readTimeout, sclLowTimeout, transferRetries);
		}
		else
		{
			status = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_Reset
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_Reset(HID_SMBUS_DEVICE device)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		status = SetReset(hidSmbus);
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_SetGpioConfig
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_SetGpioConfig(HID_SMBUS_DEVICE device, const BYTE direction, const BYTE mode, const BYTE function, const BYTE clkDiv)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		status = SetGpioConfig(hidSmbus, direction, mode, function, clkDiv);
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_GetGpioConfig
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_GetGpioConfig(HID_SMBUS_DEVICE device, BYTE* direction, BYTE* mode, BYTE* function, BYTE* clkDiv)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check pointers
		if (direction && mode && function && clkDiv)
		{
			status = GetGpioConfig(hidSmbus, direction, mode, function, clkDiv);
		}
		else
		{
			status = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_ReadLatch
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_ReadLatch(HID_SMBUS_DEVICE device, BYTE* latchValue)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check pointer
		if (latchValue)
		{
			status = GetReadLatch(hidSmbus, latchValue);
		}
		else
		{
			status = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_WriteLatch
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_WriteLatch(HID_SMBUS_DEVICE device, const BYTE latchValue, const BYTE latchMask)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		status = SetWriteLatch(hidSmbus, latchValue, latchMask);
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_GetPartNumber
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_GetPartNumber(HID_SMBUS_DEVICE device, BYTE* partNumber, BYTE* version)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check pointers
		if (partNumber && version)
		{
			status = GetPartNumber(hidSmbus, partNumber, version);
		}
		else
		{
			status = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_GetLibraryVersion
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_GetLibraryVersion(BYTE* major, BYTE* minor, BOOL* bIsRelease)
{
	HID_SMBUS_STATUS status = HID_SMBUS_UNKNOWN_ERROR;

	// Check pointers
	if (major && minor && bIsRelease)
	{
		*major		= HID_SMBUS_VERSION_MAJOR;
		*minor		= HID_SMBUS_VERSION_MINOR;
		*bIsRelease = HID_SMBUS_VERSION_RELEASE;

		status = HID_SMBUS_SUCCESS;
	}
	else
	{
		status = HID_SMBUS_INVALID_PARAMETER;
	}

	return status;
}

// HidSmbus_GetHidLibraryVersion
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_GetHidLibraryVersion(BYTE* major, BYTE* minor, BOOL* bIsRelease)
{
	HID_SMBUS_STATUS status = HID_SMBUS_UNKNOWN_ERROR;

	// Check pointers
	if (major && minor && bIsRelease)
	{
		// Get HID DLL library version
		BYTE hidStatus = HidDevice_GetHidLibraryVersion(major, minor, bIsRelease);

		// Translate HID status to HID_SMBUS status
		switch (hidStatus)
		{
			case HID_DEVICE_SUCCESS:				status = HID_SMBUS_SUCCESS;				break;
			case HID_DEVICE_INVALID_BUFFER_SIZE:	status = HID_SMBUS_INVALID_PARAMETER;	break;
			default:								status = HID_SMBUS_DEVICE_ACCESS_ERROR; break;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_PARAMETER;
	}

	return status;
}

// HidSmbus_GetHidGuid
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_GetHidGuid(void* guid)
{
	HID_SMBUS_STATUS status = HID_SMBUS_UNKNOWN_ERROR;

	// Check pointers
	if (guid)
	{
		// Return the HID GUID
		HidDevice_GetHidGuid(guid);

		status = HID_SMBUS_SUCCESS;
	}
	else
	{
		status = HID_SMBUS_INVALID_PARAMETER;
	}

	return status;
}

/////////////////////////////////////////////////////////////////////////////
// Exported Library Functions - Device Customization
/////////////////////////////////////////////////////////////////////////////

// HidSmbus_SetLock
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_SetLock(HID_SMBUS_DEVICE device, const BYTE lock)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		status = SetLock(hidSmbus, lock);
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_GetLock
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_GetLock(HID_SMBUS_DEVICE device, BYTE* lock)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check pointers
		if (lock)
		{
			status = GetLock(hidSmbus, lock);
		}
		else
		{
			status = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_SetUsbConfig
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_SetUsbConfig(HID_SMBUS_DEVICE device, const WORD vid, const WORD pid, const BYTE power, const BYTE powerMode, const WORD releaseVersion, const BYTE mask)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check parameters
		if (powerMode == HID_SMBUS_BUS_POWER && power > HID_SMBUS_BUS_POWER_MAX)
		{
			status = HID_SMBUS_INVALID_PARAMETER;
		}
		else
		{
			status = SetUsbConfig(hidSmbus, vid, pid, power, powerMode, releaseVersion, mask);
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_GetUsbConfig
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_GetUsbConfig(HID_SMBUS_DEVICE device, WORD* vid, WORD* pid, BYTE* power, BYTE* powerMode, WORD* releaseVersion)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check pointers
		if (vid && pid && power && powerMode && releaseVersion)
		{
			status = GetUsbConfig(hidSmbus, vid, pid, power, powerMode, releaseVersion);
		}
		else
		{
			status = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_SetManufacturingString
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_SetManufacturingString(HID_SMBUS_DEVICE device, char* manufacturingString, const BYTE strlen)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check pointers
		if (manufacturingString)
		{
			// Check parameters
			if (strlen <= HID_SMBUS_CP2112_MFG_STRLEN)
			{
				status = SetManufacturingString(hidSmbus, manufacturingString, strlen);
			}
			else
			{
				status = HID_SMBUS_INVALID_PARAMETER;
			}
		}
		else
		{
			status = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_GetManufacturingString
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_GetManufacturingString(HID_SMBUS_DEVICE device, char* manufacturingString, BYTE* strlen)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check pointers
		if (manufacturingString && strlen)
		{
			status = GetManufacturingString(hidSmbus, manufacturingString, strlen);
		}
		else
		{
			status = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_SetProductString
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_SetProductString(HID_SMBUS_DEVICE device, char* productString, const BYTE strlen)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check pointers
		if (productString)
		{
			// Check parameters
			if (strlen <= HID_SMBUS_CP2112_PRODUCT_STRLEN)
			{
				status = SetProductString(hidSmbus, productString, strlen);
			}
			else
			{
				status = HID_SMBUS_INVALID_PARAMETER;
			}
		}
		else
		{
			status = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_GetProductString
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_GetProductString(HID_SMBUS_DEVICE device, char* productString, BYTE* strlen)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check pointers
		if (productString && strlen)
		{
			status = GetProductString(hidSmbus, productString, strlen);
		}
		else
		{
			status = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_SetSerialString
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_SetSerialString(HID_SMBUS_DEVICE device, char* serialString, const BYTE strlen)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check pointers
		if (serialString)
		{
			// Check parameters
			if (strlen <= HID_SMBUS_CP2112_SERIAL_STRLEN)
			{
				status = SetSerialString(hidSmbus, serialString, strlen);
			}
			else
			{
				status = HID_SMBUS_INVALID_PARAMETER;
			}
		}
		else
		{
			status = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}

// HidSmbus_GetSerialString
_Use_decl_annotations_
HID_TO_SMBUS_API HID_SMBUS_STATUS WINAPI
HidSmbus_GetSerialString(HID_SMBUS_DEVICE device, char* serialString, BYTE* strlen)
{
	HID_SMBUS_STATUS	status		= HID_SMBUS_UNKNOWN_ERROR;
	CHIDtoSMBus*		hidSmbus	= ((CHIDtoSMBus*)device);

	// Check device object
	if (DeviceList.Validate(hidSmbus))
	{
		// Check pointers
		if (serialString && strlen)
		{
			status = GetSerialString(hidSmbus, serialString, strlen);
		}
		else
		{
			status = HID_SMBUS_INVALID_PARAMETER;
		}
	}
	else
	{
		status = HID_SMBUS_INVALID_DEVICE_OBJECT;
	}

	return status;
}
