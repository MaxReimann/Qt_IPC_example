# Qt_IPC_example

This code shows the interprocess communication between two applications. The c++ host spawns the python process, using QProcess, and passes multiple images which are converted to numpy arrays, then converted back and sent to the cpp process. 

The bidirectional example requires much more synchronization effort than the one directional example. A sip extension is used to pass a QVector<QImage> into the python process. 

To try out the example, execute python build.py to build the sip extension, it will be written into the sitepackages directory by default.
