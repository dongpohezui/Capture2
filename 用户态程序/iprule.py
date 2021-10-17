import sys
import time
from PyQt5.QtCore import QThread, pyqtSignal,Qt
from PyQt5.QtWidgets import *
from PyQt5.QtWidgets import QApplication, QMainWindow,QTableWidgetItem,QAbstractItemView,QHeaderView
from ipruleUI import Ui_MainWindow
import random
import winreg
import struct
import socket

from others import *

handle = ctypes.windll.kernel32.CreateFileA("\\\\.\\Capture2".encode("ascii"), 
				GENERIC_READ | GENERIC_WRITE, 
				FILE_SHARE_READ | FILE_SHARE_WRITE, 
				None, 
				OPEN_EXISTING, 
				0, None )

if (not handle) or (handle == -1):
	print("connect Capture2 symbollink failed\n")

key = winreg.CreateKeyEx(winreg.HKEY_LOCAL_MACHINE,r"SYSTEM\CurrentControlSet\Services\Capture2\iprules",reserved=0,access=winreg.KEY_ALL_ACCESS)

class ipruleMainForm(QMainWindow, Ui_MainWindow):

	def __init__(self, parent=None):
		super(ipruleMainForm, self).__init__(parent)
		self.setupUi(self)
		self.flushiprules()
		
		self.pushButton.clicked.connect(self.addiprules)
		self.pushButton_2.clicked.connect(self.flushiprules)
		self.pushButton_3.clicked.connect(self.cleariprules)
		self.pushButton_4.clicked.connect(self.startipblock)
		self.pushButton_5.clicked.connect(self.stopipblock)
		self.tableWidget.setEditTriggers(QAbstractItemView.NoEditTriggers) #将表格变为禁止编辑
		self.tableWidget.setSelectionBehavior(QAbstractItemView.SelectRows)#设置只能选中整行
		self.tableWidget.setContextMenuPolicy(Qt.CustomContextMenu)        #允许右键产生子菜单
		self.tableWidget.customContextMenuRequested.connect(self.generateMenu)  ####右键菜单
		
		self.lineEdit_11.setText(str(random.randint(1,100000))) 



	def generateMenu(self, pos):
		try:
			self.contextMenu = QMenu()#创建对象
			self.actionA = self.contextMenu.addAction(u'删除')#添加动作
			self.actionA.triggered.connect(self.deletesomerules)
			self.contextMenu.exec_(self.tableWidget.mapToGlobal(pos))#随指针的位置显示菜单
			self.contextMenu.show()#显示
		except Exception as e:
			print(e)
	
	def deletesomerules(self):
		indexs = self.tableWidget.selectionModel().selectedRows()
		for index in indexs:
			winreg.DeleteValue(key, self.tableWidget.item(index.row(), 0).text())
		
		self.flushiprules()
		return_code = ctypes.windll.kernel32.DeviceIoControl(handle, 
					CAPTURE2_IOCTL_NINE, 
					None, 0, 
					None, 0, 
					0, None)
		
		
		
		

	def addiprules(self):
		global handle,key
		try:
			print("addiprules")	

			iprule = IPBLOCK()
			iprule.id          =  int(self.lineEdit_11.text())
			iprule.direction   =  int(self.lineEdit_8.text())
			iprule.protocol    =  int(self.lineEdit_7.text())
			iprule.srcIp       =  struct.unpack('>I',socket.inet_aton(self.lineEdit.text()))[0]
			iprule.srcIpMask   =  struct.unpack('>I',socket.inet_aton(self.lineEdit_2.text()))[0]
			iprule.srcPort     =  int(self.lineEdit_3.text())
			iprule.srcPortMask =  int(self.lineEdit_9.text())
			iprule.dstIp       =  struct.unpack('>I',socket.inet_aton(self.lineEdit_4.text()))[0]
			iprule.dstIpMask   =  struct.unpack('>I',socket.inet_aton(self.lineEdit_5.text()))[0]
			iprule.dstPort     =  int(self.lineEdit_6.text())
			iprule.dstPortMask =  int(self.lineEdit_10.text())
			winreg.SetValueEx(key,self.lineEdit_11.text(),0,winreg.REG_BINARY,iprule)
			
			return_code = ctypes.windll.kernel32.DeviceIoControl(handle, 
					CAPTURE2_IOCTL_NINE, 
					None, 0, 
					None, 0, 
					0, None)

			self.lineEdit_11.setText(str(random.randint(1,2147483000)))

		except Exception as e:
			# 访问异常的错误编号和详细信息
			print(e.args)
			print(str(e))
			print(repr(e))		
	
	def startipblock(self):
		global handle
		print("startipblock")
		return_code = ctypes.windll.kernel32.DeviceIoControl(handle, 
					CAPTURE2_IOCTL_SEVEN, 
					None, 0, 
					None, 0, 
					0, None)

	def stopipblock(self):
		global handle
		print("stopipblock")
		return_code = ctypes.windll.kernel32.DeviceIoControl(handle, 
					CAPTURE2_IOCTL_EIGHT, 
					None, 0, 
					None, 0, 
					0, None)
	
	def cleariprules(self):
		global handle,key
		self.tableWidget.clearContents()
		self.tableWidget.setRowCount(0)
		try:
			winreg.DeleteKey(key, "")
			key = winreg.CreateKeyEx(winreg.HKEY_LOCAL_MACHINE,r"SYSTEM\CurrentControlSet\Services\Capture2\iprules",reserved=0,access=winreg.KEY_ALL_ACCESS)
		except WindowsError as e:
			pass
		return_code = ctypes.windll.kernel32.DeviceIoControl(handle, 
					CAPTURE2_IOCTL_TEN, 
					None, 0, 
					None, 0, 
					0, None)
	
	
	def flushiprules(self):
		global key
		self.tableWidget.clearContents()
		self.tableWidget.setRowCount(0)
		try:
			i=0
			while 1:
				#EnumValue方法用来枚举键值，EnumKey用来枚举子键
				name,value,type = winreg.EnumValue(key,i)
				i+=1

				if len(value) != 56:
					continue
				iprule = IPBLOCK()
				memmove(addressof(iprule),value,sizeof(IPBLOCK))
				items = []
				items.append(str(iprule.id) )
				items.append(str(iprule.protocol) )
				items.append(str(iprule.direction) )
				items.append(socket.inet_ntoa(struct.pack(">I", iprule.srcIp)) )
				items.append(socket.inet_ntoa(struct.pack(">I", iprule.srcIpMask)) )
				items.append(str(iprule.srcPort) )
				items.append(str(iprule.srcPortMask) )
				items.append(socket.inet_ntoa(struct.pack(">I", iprule.dstIp)) )
				items.append(socket.inet_ntoa(struct.pack(">I", iprule.dstIpMask)) )
				items.append(str(iprule.dstPort) )
				items.append(str(iprule.dstPortMask))
				
				self.tableWidget.insertRow(i-1)
				for j in range(11):
					item = QTableWidgetItem(str(items[j]))
					self.tableWidget.setItem(i-1,j,item)
				
				
		except Exception as e:
			pass
			#print(e.args)
			#print(str(e))
			#print(repr(e))
		

if __name__ == "__main__":
	app = QApplication(sys.argv)
	myWin = ipruleMainForm()
	myWin.show()
	sys.exit(app.exec_())
