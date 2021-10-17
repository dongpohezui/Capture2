import sys
import time
import ctypes
from ctypes import * 
from ctypes import wintypes
import time
import datetime
import struct
from serial import win32

from PyQt5.Qt import *
from PyQt5.QtCore import QThread, pyqtSignal
from PyQt5.QtWidgets import QApplication, QMainWindow
from packetUI import Ui_MainWindow

from others import *


#是否开启数据包捕获
global_MacDebug = 0
global_packet = []

handle = ctypes.windll.kernel32.CreateFileA("\\\\.\\Capture2".encode("ascii"), 
				GENERIC_READ | GENERIC_WRITE, 
				FILE_SHARE_READ | FILE_SHARE_WRITE, 
				None, 
				OPEN_EXISTING, 
				0, None )

if (not handle) or (handle == -1):
	print("connect Capture2 symbollink failed\n")


def DeviceIoControl_three():
	
	global global_packet
	global handle
	
	outLength = wintypes.DWORD()

	macdata = ctypes.create_string_buffer(65540)

	return_code = ctypes.windll.kernel32.DeviceIoControl(handle, 
				CAPTURE2_IOCTL_THREE, 
				None, 0, 
				macdata, 65540, 
				ctypes.byref(outLength), None)
	
	if struct.unpack('i',outLength)[0] == 0:
		return ''
	
	if not return_code:
		return 'DeviceIoControl_three wrong'
	
	t = time.time()
	#int(t)#秒级时间戳
	#int(round(t * 1000000))  #微秒级时间戳
	pcapdata =  bytes()
	pcapdata =  int(t).to_bytes(length=4, byteorder='little', signed=True)      
	#'big'代表高位在前
	pcapdata += int((t-int(t) )* 1000000).to_bytes(length=4, byteorder='little', signed=True)  
	pcapdata +=	outLength           
	pcapdata += outLength           
	pcapdata += macdata[0:struct.unpack('i',outLength)[0]]
				
	global_packet.append(pcapdata)
	
	return macdata[0:struct.unpack('i',outLength)[0]].hex()

class packetMainForm(QMainWindow, Ui_MainWindow):
	def __init__(self, parent=None):
		super(packetMainForm, self).__init__(parent)
		self.setupUi(self)
		# 实例化线程对象
		self.work = StartGetPacket()
		# 线程自定义信号连接的槽函数
		self.work.trigger.connect(self.display)
		self.pushButton.clicked.connect(self.execute)
		self.pushButton_2.clicked.connect(self.DeviceIoControl_two)
		self.pushButton_3.clicked.connect(self.savepacket)

	def execute(self):
		# 启动线程
		self.work.start()
	
	def savepacket(self):
		try :
			global global_packet
			result = QFileDialog.getSaveFileName(self, "保存为pcap文件", "./",
												 "ALL(*, *);")
			file = open(result[0], "wb")
			
			for i in global_packet:
				file.write(i)
			file.close()
			global_packet = []
			self.listWidget.clear()
		except Exception as e:
			# 访问异常的错误编号和详细信息
			print(e.args)
			print(str(e))
			print(repr(e))

	def display(self,str):
		# 由于自定义信号时自动传递一个字符串参数，所以在这个槽函数中要接受一个参数
		self.listWidget.addItem(str)
		
	def DeviceIoControl_two(self):
		print("stop capture")
		global global_MacDebug
		global handle
		
		bytes_returned = wintypes.DWORD()
		
		return_code = ctypes.windll.kernel32.DeviceIoControl(handle, 
				CAPTURE2_IOCTL_TWO, 
				None, 0, 
				None, 0, 
				ctypes.byref(bytes_returned), None)
		
		int_return = struct.unpack('i',bytes_returned)
		while int_return[0] != 0 :
			for i in range(int_return[0]):
				pcaket_str = DeviceIoControl_three()
				self.listWidget.addItem(pcaket_str)
			
			return_code = ctypes.windll.kernel32.DeviceIoControl(handle, 
				CAPTURE2_IOCTL_TWO, 
				None, 0, 
				None, 0, 
				ctypes.byref(bytes_returned), None)
			int_return = struct.unpack('i',bytes_returned)
			
		global_MacDebug = 0

class StartGetPacket(QThread):
	# 自定义信号对象。参数str就代表这个信号可以传一个字符串
	trigger = pyqtSignal(str)
	def __int__(self):
		# 初始化函数
		super(WorkThread, self).__init__()
		
	#重写线程执行的run函数
	#触发自定义信号
	def run(self):
		print("start capture")
		global global_packet
		global global_MacDebug
		global handle
		
		if global_MacDebug == 1:
			return
		
		bytes_returned = wintypes.DWORD()
		
		hEvent = wintypes.HANDLE(ctypes.windll.kernel32.CreateEventA(None, 0, 0, None))
		
		#非零表示成功，零表示失败。
		return_code = ctypes.windll.kernel32.DeviceIoControl(handle, 
			CAPTURE2_IOCTL_ONE, 
			ctypes.byref(hEvent), 16, 
			None, 0, 
			ctypes.byref(bytes_returned), None)
		
		#调用失败
		if not return_code:
			return
		global_packet.append(pcap_head)
		global_MacDebug = 1
		
		while 1:
			
			if global_MacDebug !=1 :
				return
			
			ctypes.windll.kernel32.WaitForSingleObject(hEvent,0xffffffff)
			
			pcaket_str = DeviceIoControl_three()
			if len(pcaket_str) > 8:
				# 通过自定义信号把待显示的字符串传递给槽函数
				self.trigger.emit(pcaket_str)
			
			pcaket_str = DeviceIoControl_three()
			if len(pcaket_str) > 8:
				# 通过自定义信号把待显示的字符串传递给槽函数
				self.trigger.emit(pcaket_str)

if __name__ == "__main__":
	app = QApplication(sys.argv)
	myWin = packetMainForm()
	myWin.show()
	sys.exit(app.exec_())