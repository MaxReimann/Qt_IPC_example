from PyQt5 import QtCore, QtGui
import numpy as np
import qimage2ndarray


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


	def receiveImages(self):
		self.images = []
		buf = QtCore.QBuffer()
		datastream = QtCore.QDataStream(buf)

		l = self.in_memory.lock();
		if l != True:
			print "locking error: %s"%(self.in_memory.errorString())

		print "size mem: %d"%(self.in_memory.size())
		buf.setData(self.in_memory.constData())
		buf.open(QtCore.QBuffer.ReadOnly)

		while True:
			qimg = QtGui.QImage()
			try:
				datastream >> qimg

			except Exception as e:
				print "exception in receive Function: %s"%(e)
				break
			if qimg.isNull(): #no more images in stream
				break
			self.images.append(qimage2ndarray.rgb_view(qimg)) #appends copy

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
		print "buf: %d"%(buf.size())
		
		while True:
			if self.out_memory.attach():
				print("out memory attached")
				break

		self.out_memory.lock()
		try:
			self.out_memory.data()[:] = buf.data().data()
		except Exception as e:
			print(e)

		print("sending Imafes finished")
		self.out_memory.unlock()
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
	ipchandler = IPCHandler('process_example17')
	ipchandler.receiveImages()
	images = ipchandler.getImages()
	ipchandler.sendImages([ipchandler.convertMatToQImage(img) for img in images])
	del ipchandler
except Exception as e:
	print(e)
	del ipchandler

print "finished"