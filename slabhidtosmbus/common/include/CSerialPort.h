// Copyright (c) 2016 by Silicon Laboratories Inc.  All rights reserved.
// The program contained in this listing is proprietary to Silicon Laboratories,
// headquartered in Austin, Texas, U.S.A. and is subject to worldwide copyright
// protection, including protection under the United States Copyright Act of 1976
// as an unpublished work, pursuant to Section 104 and Section 408 of Title XVII
// of the United States code.  Unauthorized copying, adaptation, distribution,
// use, or display is prohibited by this law.

#ifndef __SERIAL_PORT_H__
#define __SERIAL_PORT_H__

#include <string>
#include <vector>
#include <Windows.h>
#include <ntddser.h>
#ifndef TRACE
#error Define TRACE before including this. E.g., #define TRACE printf
#endif

class CWinApiErr // thrown any time a Windows API fails
{
public:
    CWinApiErr( char *ApiName )
    {
        TRACE( "%s: LastError %d\n", ApiName, ::GetLastError());
    }
};

//---------------------------------------------------------------------
// Serial port primitives needed for the tests
// Throws CWinApiErr when unexpected happens.
//---------------------------------------------------------------------
class CSerialPort
{
    HANDLE m_hSer;
public:
    CSerialPort( const std::string ComName)
    {
        m_hSer = ::CreateFile( std::string( "\\\\.\\" + ComName).c_str(),
                               GENERIC_READ | GENERIC_WRITE, 0 /*share*/,
                               NULL /*security*/, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (m_hSer == INVALID_HANDLE_VALUE) {
            throw CWinApiErr( "CreateFile");
        }
    }
    ~CSerialPort()
    {
        if( m_hSer != INVALID_HANDLE_VALUE) {
            ::CloseHandle( m_hSer);
        }
    }
    void Mode( const std::string Mode )
    {
        DCB Dcb = { 0 };
        Dcb.DCBlength = sizeof( Dcb);
        if (!::BuildCommDCB( Mode.c_str(), &Dcb)) {
            throw CWinApiErr( "BuildCommDCB");
        }
        if( !::SetCommState( m_hSer, &Dcb)) {
            throw CWinApiErr( "SetCommState");
        }
    }
    DWORD Write( std::vector<BYTE> Data)
    {
        DWORD CbWritten = 0;
        if( !::WriteFile( m_hSer, Data.data(), Data.size(), &CbWritten, NULL)) {
            throw CWinApiErr( "WriteFile");
        }
        TRACE( "Wrote %x bytes\n", CbWritten);
        if( CbWritten != Data.size()) {
            TRACE( "ERROR: was asked to write %x bytes\n", Data.size());
        }
        return CbWritten;
    }
    void SetCommMask( DWORD Mask)
    {
        if( !::SetCommMask( m_hSer, Mask )) {
            throw CWinApiErr( "SetCommMask");
        }
    }
    DWORD WaitCommEvent()
    {
        DWORD Events;
        if( !::WaitCommEvent( m_hSer, &Events, NULL)) {
            throw CWinApiErr( "WaitCommEvent");
        }
        TRACE( "Received Events %x\n", Events);
        return Events;
    }
    void GetSerialStatus( SERIAL_STATUS &s)
    {
        DWORD CbReturned = 0;
        if( !::DeviceIoControl( m_hSer, IOCTL_SERIAL_GET_COMMSTATUS, NULL, 0,
                                &s, sizeof( s), &CbReturned, NULL)) {
            throw CWinApiErr( "DeviceIoControl");
	    } else if( CbReturned != sizeof( s)) {
            ::SetLastError( ERROR_SUCCESS);
            throw CWinApiErr( "unexpected IOCTL_SERIAL_GET_COMMSTATUS size");
        } else {
            TRACE( "AmountInOutQueue %x\n", s.AmountInOutQueue);
        }
    }
};

#endif // __SERIAL_PORT_H__
