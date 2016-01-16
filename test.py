from PyQt5 import QtCore, QtGui


_memory = QtCore.QSharedMemory()
_memory.setKey('process_example')
attached = False
while not attached:
	if _memory.attach():
		print("memory attached")
		attached = True


_memory.lock();
try:
	qimg = QtGui.QImage()
	buf = QtCore.QBuffer()
	datastream = QtCore.QDataStream(buf)
	buf.setData(_memory.constData())
	buf.open(QtCore.QBuffer.ReadOnly)
	datastream >> qimg
except Exception as e:
	print(e)


_memory.unlock();
print("memory unlocked")

try:
	qimg.save("out.png")
except Exception as e:
	print(e)

print("image saved")

_memory.detach();