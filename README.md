# Qt Interprocess Communication Example

This code shows how to do efficient IPC calls between two applications with qt using a c++ host and a python client. The c++ host spawns the python process, using QProcess, and passes multiple images which are converted to numpy arrays, then converted back and sent to the c++ process. 

The bidirectional example requires much more synchronization effort than the one directional example. A sip extension is used to pass a `QVector<QImage>` into the python process. 

To try out the example, execute python build.py to build the sip extension, it will be written into the sitepackages directory by default.

The master branch contains a one directional example
