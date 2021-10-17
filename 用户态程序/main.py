import sys
import os
import time
from multiprocessing import Process
import ctypes

from PyQt5.QtCore import QThread, pyqtSignal
from PyQt5.QtWidgets import QApplication, QMainWindow

def is_admin():
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except:
        return False

if is_admin() == False:
	ctypes.windll.shell32.ShellExecuteW(None, "runas", sys.executable, __file__, None, 1)
	sys.exit()

os.system("sc.exe start Capture2")

from mainUI import Ui_MainWindow
from packet import packetMainForm
from iprule import ipruleMainForm


class MainForm(QMainWindow, Ui_MainWindow):
	def __init__(self, parent=None):
		super(MainForm, self).__init__(parent)
		self.setupUi(self)
		self.pushButton.released.connect(lambda: packet.show()   )
		self.pushButton_2.released.connect(lambda: iprule.show()   )
	
if __name__ == "__main__":
	

	
	try:
		app = QApplication(sys.argv)
		packet = packetMainForm()
		iprule = ipruleMainForm()
		
		main = MainForm()
		main.show()

		app.exec_()
	except Exception as e:
		# 访问异常的错误编号和详细信息
		print(e.args)
		print(str(e))
		print(repr(e))