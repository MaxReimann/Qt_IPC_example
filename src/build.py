import shutil
import os
import subprocess


if os.path.isdir('../autogen'):
	shutil.rmtree('../autogen')

os.makedirs('../autogen')

shutil.copyfile("datastreamer.h", "../autogen/datastreamer.h")
shutil.copyfile("datastreamer.sip", "../autogen/datastreamer.sip")

#runs config
import configure


proc=subprocess.Popen(['make', 'install'],cwd='../autogen')  
proc.communicate()