from PyQt4 import QtCore, QtGui
import numpy as np
import qimage2ndarray
try:
	from datastreamreader import DataStreamReader
except ImportError:
	print("datastreamreader can not be imported, has to be built first and put in the pythonpath or this directory")
except Exception as e:
	print(e)


class IPCHandler(object):
	def __init__(self, process_name):
		self.comm_memory = QtCore.QSharedMemory()
		self.comm_memory.setKey(process_name + "_communication")
		while True:
			if self.comm_memory.attach():
				print("comm memory attached")
				break
		self.in_memory = QtCore.QSharedMemory()
		self.in_memory.setKey(process_name+"_in")
		while True:
			if self.in_memory.attach():
				print("in memory attached")
				break
		self.images = []

		self.out_memory = QtCore.QSharedMemory()
		self.out_memory.setKey(process_name+"_out")

		self.mem_handler = DataStreamReader()


	def receiveImages(self):
		self.images = []

		#wait for host to have data ready
		while self.readCommStatus().x() == 0:
			pass

		imlist = self.mem_handler.readImages(self.in_memory)


		for im in imlist:
			self.images.append(self.convertQImageToMat(im))


		print "received %d images"%(len(self.images))
		return self.images


	def sendImages(self, images):
		# lets be generous
		totalByteCount = 1.5 * sum([im.byteCount() for im in images])

		#creates the out memory of size totalBytecount on the c++ side
		self.updateCommStatus(totalByteCount)
		print "image width: %d"%(images[0].width())
		
		while True:
			if self.out_memory.attach():
				print("out memory attached")
				break

		self.mem_handler.sendImages(self.out_memory, images)
		self.updateCommStatus(0)


	def getImages(self):
		return self.images

	def convertMatToQImage(self, image):
		#return QtGui.QImage(image.data,image.shape[1], image.shape[0], QtGui.QImage.Format_RGB888)
		return qimage2ndarray.array2qimage(image)

	def convertQImageToMat(self, incomingImage):
		'''  Converts a QImage into an numpy ndarray format  '''

		# width = incomingImage.width()
		# height = incomingImage.height()

		# print "width %d height %d, byteCount: %d"%(width, height, incomingImage.byteCount())

		# ptr = incomingImage.bits()
		# ptr.setsize(incomingImage.byteCount())
		# arr = np.array(ptr).reshape(height, width, 3)  #  Copies the data
		# return arr
		return qimage2ndarray.rgb_view(incomingImage.convertToFormat(QtGui.QImage.Format_RGB32))


	def updateCommStatus(self, this_status):
		buf = QtCore.QBuffer()
		buf.open(QtCore.QBuffer.ReadWrite)

		readStatus = self.readCommStatus()
		status = QtCore.QPoint(readStatus.x(), this_status)

		datastream = QtCore.QDataStream(buf)
		datastream << status

		self.comm_memory.lock()
		self.comm_memory.data()[:] = buf.data().data()
		self.comm_memory.unlock()

	def readCommStatus(self):
		status = QtCore.QPoint()
		buf = QtCore.QBuffer()
		datastream = QtCore.QDataStream(buf)

		self.comm_memory.lock()
		buf.setData(self.comm_memory.constData())
		buf.open(QtCore.QBuffer.ReadOnly)
		datastream >> status
		self.comm_memory.unlock()
		return status

	def __del__(self):
		if self.comm_memory.isAttached():
			self.comm_memory.detach()
		if self.in_memory.isAttached():
			self.in_memory.detach()
		if self.out_memory.isAttached():
			self.out_memory.detach()
		print "cleaned up"


try:
	ipchandler = IPCHandler('process_example3')
	ipchandler.receiveImages()
	images = ipchandler.getImages()
	ipchandler.sendImages([ipchandler.convertMatToQImage(img) for img in images])
	del ipchandler
except Exception as e:
	print(e)
	del ipchandler

print "finished"