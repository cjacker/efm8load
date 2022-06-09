#pragma once

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "SLABHIDDevice.h"
#include "ReportQueue.h"
#include "OsDep.h"

#include <vector>

/////////////////////////////////////////////////////////////////////////////
// CHIDtoSMBus Class
/////////////////////////////////////////////////////////////////////////////

class CHIDtoSMBus
{
public:
	HID_DEVICE		hid;
	CReportQueue	queueReadResponse;
	CReportQueue	queueTransferStatusResponse;
	WORD			readTimeout;
	WORD			writeTimeout;
	DWORD			responseTimeout;
	BYTE			partNumber;

	CHIDtoSMBus()
	{
		hid				= NULL;
		readTimeout		= 0;
		writeTimeout	= 0;
		responseTimeout	= 1000;
		partNumber		= 0x00;
	}
};
