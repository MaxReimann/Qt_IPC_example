from PyQt5 import QtCore, QtGui
import numpy as np


class IPCHandler(object):
	def __init__(self, process_name):
		self.in_memory = QtCore.QSharedMemory()
		self.in_memory.setKey(process_name+"_in")
		while True:
			if self.in_memory.attach():
				print("memory attached")
				break
		self.images = []

		self.out_memory = QtCore.QSharedMemory()
		self.out_memory.setKey(process_name+"_out")

		self.comm_memory = QtCore.QSharedMemory()
		self.comm_memory.setKey(process_name + "_communication")
		while True:
			if self.comm_memory.attach():
				print("comm memory attached")
				break

	def receiveImages(self):
		self.images = []
		buf = QtCore.QBuffer()
		datastream = QtCore.QDataStream(buf)

		self.in_memory.lock();
		buf.setData(self.in_memory.constData())
		buf.open(QtCore.QBuffer.ReadOnly)

		while True:
			qimg = QtGui.QImage()
			try:
				datastream >> qimg
			except Exception as e:
				print(e)
				break
			if qimg.isNull(): #no more images in stream
				break
			self.images.append(self.convertQImageToMat(qimg)) #appends copy

		print "received %d images"%(len(self.images))
		self.in_memory.unlock()


	def sendImages(self, images):
		buf = QtCore.QBuffer()
		buf.open(QtCore.QBuffer.WriteOnly)
		datastream = QtCore.QDataStream(buf)
		for img in images:
			datastream << img

		self.updateCommStatus(buf.size())
		print "image width: %d"%(images[0].width())
		
		while True:
			if self.out_memory.attach():
				print("out memory attached")
				break

		self.out_memory.lock()
		try:
			self.out_memory.data()[:] = buf.data().data()
		except Exception as e:
			print(e)

		self.out_memory.unlock()


	def getImages(self):
		return self.images

	def convertMatToQImage(self, image):
		return QtGui.QImage(image.data,image.shape[1], image.shape[0], QtGui.QImage.Format_RGB888)

	def convertQImageToMat(self, incomingImage):
		'''  Converts a QImage into an numpy mat format  '''

		incomingImage = incomingImage.convertToFormat(QtGui.QImage.Format_RGB888)

		width = incomingImage.width()
		height = incomingImage.height()

		ptr = incomingImage.bits()
		ptr.setsize(incomingImage.byteCount())
		arr = np.array(ptr).reshape(height, width, 3)  #  Copies the data
		return arr

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

		print status

		return status

	def __del__(self):
		self.in_memory.detach()
		self.out_memory.detach()
		self.comm_memory.detach()
		print "calling destructor"


try:
	ipchandler = IPCHandler('process_example')
	ipchandler.receiveImages()
	images = ipchandler.getImages()
	ipchandler.sendImages([ipchandler.convertMatToQImage(img) for img in images])
	del ipchandler
except Exception as e:
	print(e)

print "finished"