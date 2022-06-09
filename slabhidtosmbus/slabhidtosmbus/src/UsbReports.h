#pragma once

/////////////////////////////////////////////////////////////////////////////
// Report ID Definitions
/////////////////////////////////////////////////////////////////////////////

#define RESET_DEVICE			0x01 // Reset Device
#define CONFIGURE_GPIO			0x02 // Get Set GPIO direction and mode
#define GET_GPIO_VAL			0x03 // Get GPIO Values
#define SET_GPIO_VAL			0x04 // Set GPIO Values
#define GET_VERSION				0x05 // Get Version Information
#define GS_SMB_CONFIG			0x06 // Get Set SMBus Config

#define READ_REQUEST            0x10  // out
#define ADR_READ_REQUEST        0x11  // out
#define READ_FORCE_SEND         0x12  // out
#define READ_RESPONSE           0x13  // in
#define WRITE_DATA              0x14  // out
#define STATUS_REQUEST          0x15  // out
#define STATUS_RESPONSE         0x16  // in
#define CANCEL_TRANSFER         0x17  // out

#define GETSET_LOCK				0x20 // Get Set Lock Byte
#define GETSET_USB_CONFIG		0x21 // All customizable data that's not strings
#define GETSET_MANUSTR			0x22 // Get Set Manufacturing String
#define GETSET_PRODSTR			0x23 // Get Set Product String
#define GETSET_SERSTR			0x24 // Get Set Serial String

/////////////////////////////////////////////////////////////////////////////
// Bit Mask Definitions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// String Buffer Definitions
/////////////////////////////////////////////////////////////////////////////

#define SHORT_STRING_LEN				60
