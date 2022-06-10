#!/usr/bin/env python

"""
usage: hex2boot.py [-h] [-v] -o OUT [-b {0,1}] [-e {0,1,2}] [-i [ID [ID ...]]]
                   [-l LOCK] [-m {bb2,sb2,ub1}] [-s ADDR] [-t ADDR] [-w]
                   [hexfile]

Hex to Boot Record conversion utility.

positional arguments:
  hexfile               hex file to convert

optional arguments:
  -h, --help            show this help message and exit
  -v, --version         show program's version number and exit
  -o OUT, --out OUT     boot record output file (required)

conversion parameters:
  -b {0,1}, --bank {0,1}
                        flash bank (default=0)
  -e {0,1,2}, --erase {0,1,2}
                        0=none, 1=separate, (2=with data)
  -i [ID [ID ...]], --id [ID [ID ...]]
                        identity values (default=None)
  -l LOCK, --lock LOCK  lock value (default=None)
  -m {bb2,bb50,bb51,bb52,sb2,ub1}, --map {bb2,bb50,bb51,bb52,sb2,ub1}
                        special map type (default: None)
  -s ADDR, --start ADDR
                        start address (default=0)
  -t ADDR, --top ADDR   top address (default=65535)
  -w, --wait            remain in bootloader mode (default: False)
---
# TODO: Skip page size holes
"""

epilog = """
If hexfile is not provided, the utility will create records to erase 
the specified address range.
"""

import argparse
import intelhex
import struct

VERSION = '1.10'

_CRC16_XMODEM_TABLE = [
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0,
    ]

def crc16(data, crc=0, table=_CRC16_XMODEM_TABLE):
    for byte in bytearray(data):
        crc = ((crc<<8) & 0xff00) ^ table[((crc>>8) & 0xff) ^ byte]
    return crc & 0xffff

FSB = b'$'
S_IDENT  = struct.Struct('>cBBH')
S_SETUP  = struct.Struct('>cBBHB')
S_ERASE  = struct.Struct('>cBBH')
S_WRITE  = struct.Struct('>cBBH')
S_VERIFY = struct.Struct('>cBBHHH')
S_LOCK   = struct.Struct('>cBBH')
S_RUNAPP = struct.Struct('>cBBH')

def bin_ident(blid):
    return S_IDENT.pack(FSB, 3, 0x30, blid)

def bin_setup(bank, keys=0xA5F1):
    return S_SETUP.pack(FSB, 4, 0x31, keys, bank)

def bin_erase(addr, data=b''):
    return S_ERASE.pack(FSB, 3+len(data), 0x32, addr) + data

def bin_write(addr, data):
    return S_WRITE.pack(FSB, 3+len(data), 0x33, addr) + data

def bin_verify(org, end, crc):
    return S_VERIFY.pack(FSB, 7, 0x34, org, end, crc)

def bin_lock(lock):
    return S_LOCK.pack(FSB, 3, 0x35, lock)

def bin_runapp(option=0):
    return S_RUNAPP.pack(FSB, 3, 0x36, option)

def regions(type):
    return {
        'bb2': [ [ 0x0000, 0x3DFF,  512 ], [ 0xF800, 0xFBBF, 64 ] ],
        'bb50': [ [ 0x0000, 0x37FF,  2048] ],
        'bb51': [ [ 0x0000, 0x37FF,  2048] ],
        'bb52': [ [ 0x0000, 0x77FF,  2048] ],
        'sb2': [ [ 0x0000, 0xF7FF, 1024 ] ], 
        'ub1': [ [ 0x0000, 0x3DFF,  512 ], [ 0xF800, 0xFBBF, 64 ] ],
        }.get(type, [ [ 0x0000, 0xFBFF, 512 ] ])
    
def get_regions(org, top, type):
    for start, stop, page in regions(type):
        if max(org, start) < min(top, stop):
            yield max(org, start), min(top, stop), page

def mem2boot(brec, ih, page=512, erase=2):
    addresses = ih.addresses()
    if addresses:
        crc = 0
        minaddr = int(addresses[0] / page) * page
        maxaddr = addresses[-1]
        recsize = min(128, page)
        for addr in range(minaddr, maxaddr+1, recsize):
            size = min(recsize, maxaddr - addr + 1)
            data = ih.tobinstr(start=addr, size=size)
            crc = crc16(data, crc)
            if (erase == 0) or (addr % page):
                bin = bin_write(addr, data)
            elif erase == 1:
                bin = bin_erase(addr) + bin_write(addr, data)
            else:
                bin = bin_erase(addr, data)
            brec.write(bin)
        brec.write(bin_verify(minaddr, maxaddr, crc))

def erase2boot(brec, start, stop, page=512):
    page_start = int(start / page) * page
    for addr in range(page_start, stop+1, page):
        brec.write(bin_erase(addr))
    size = stop - start + 1
    brec.write(bin_verify(start, stop, crc16(b'\xff'*size, 0)))

def hex2boot(brec, args):
    failsafe = (args.bank == 0) and (args.start == 0)

    for blid in args.id:
        brec.write(bin_ident(blid))
    brec.write(bin_setup(args.bank))

    if args.hexfile:
        ih = intelhex.IntelHex(args.hexfile)
        resetv = ih[0]
        if failsafe and resetv != 0xFF:
            ih[0] = 0xFF
        for start, stop, page in get_regions(args.start, args.top, args.map):
            mem2boot(brec, ih[slice(start, stop)], page, args.erase)
        if failsafe and resetv != 0xFF:
            brec.write(bin_write(0, bytearray([resetv])))
    elif args.lock is None:
        for start, stop, page in get_regions(args.start, args.top, args.map):
            erase2boot(brec, start, stop, page)

    if args.lock is not None:
        brec.write(bin_lock(args.lock))
    if not args.wait:
        brec.write(bin_runapp())

if __name__ == "__main__":
    def literal(str):
        value = int(str, 0)
        if value > 0xFFFF:
            raise argparse.ArgumentTypeError("value is too large")
        return value

    ap = argparse.ArgumentParser(description='Hex to Boot Record conversion utility.', epilog=epilog)
    ap.add_argument('-v', '--version', action='version', version='%(prog)s ' + VERSION)
    ap.add_argument('hexfile', nargs='?', help='hex file to convert')
    ap.add_argument('-o', '--out', required=True, type=argparse.FileType('wb'), 
        help='boot record output file (required)')

    ag = ap.add_argument_group('conversion parameters')
    ag.add_argument('-b', '--bank', choices=range(2), default=0, type=int, 
        help='flash bank (default=%(default)s)')
    ag.add_argument('-e', '--erase', choices=range(3), default=2, type=int, 
        help='0=none, 1=separate, (2=with data)')
    ag.add_argument('-i', '--id', nargs='*', default=[], type=literal, 
        help='identity values (default=None)')
    ag.add_argument('-l', '--lock', type=literal, 
        help='lock value (default=%(default)s)')
    ag.add_argument('-m', '--map', choices=['bb2', 'bb50', 'bb51', 'bb52', 'sb2', 'ub1'], 
        help='special map type (default: %(default)s)')
    ag.add_argument('-s', '--start', default=0, type=literal, metavar='ADDR', 
        help='start address (default=%(default)s)')
    ag.add_argument('-t', '--top', default=0xFFFF, type=literal, metavar='ADDR', 
        help='top address (default=%(default)s)')
    ag.add_argument('-w', '--wait', action='store_true', 
        help='remain in bootloader mode (default: %(default)s)')

    args = ap.parse_args()
    hex2boot(args.out, args)
