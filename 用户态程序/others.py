
import ctypes
from ctypes import * 

GENERIC_READ = 0x80000000
GENERIC_WRITE = 0x40000000
FILE_SHARE_READ = 1
FILE_SHARE_WRITE = 2
OPEN_EXISTING = 3



CAPTURE2_IOCTL_ONE = 0x122004
CAPTURE2_IOCTL_TWO = 0x122008
CAPTURE2_IOCTL_THREE = 0x12200c
CAPTURE2_IOCTL_FOUR = 0x122010
CAPTURE2_IOCTL_FIVE = 0x122014
CAPTURE2_IOCTL_SIX = 0x122018
CAPTURE2_IOCTL_SEVEN = 0x12201c
CAPTURE2_IOCTL_EIGHT = 0x122020
CAPTURE2_IOCTL_NINE = 0x122024
CAPTURE2_IOCTL_TEN = 0x120040
CAPTURE2_IOCTL_ELEVEN = 0x120044
CAPTURE2_IOCTL_TWELVE = 0x120048


pcap_head = b'\xd4\xc3\xb2\xa1\x02\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x01\x00\x00\x00'


class IPBLOCK(Structure):  
	_fields_ = [ 
				("pointer1"   , c_void_p),  
				("pointer2"   , c_void_p),  
				("id"         , c_int),
				("direction"  , c_int),  
				("protocol"   , c_uint8 ),  
				("srcIp"      , c_uint32),
				("srcIpMask"  , c_uint32),  
				("srcPort"    , c_uint16),  
				("srcPortMask", c_uint16 ),
				("dstIp"      , c_uint32),
				("dstIpMask"  , c_uint32),  
				("dstPort"    , c_uint16),  
				("dstPortMask", c_uint16 ),
				]  








