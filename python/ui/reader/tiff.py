"""
Author: Nathan Clack <clackn@janelia.hhmi.org>
Date  : 

Copyright 2010 Howard Hughes Medical Institute.
All rights reserved.
Use is subject to Janelia Farm Research Campus Software Copyright 1.1
license terms (http://license.janelia.org/license/jfrc_copyright_1_1.html).
"""
import os
import sys
from ctypes import *
from ctypes.util import find_library

import numpy
from numpy import zeros

from .reader import StackReader

dllpath = os.path.split(os.path.split(os.path.abspath(__file__))[0])[0]
if sys.platform == 'win32':
    lib = os.path.join(dllpath, 'whisk.dll')
else:
    lib = os.path.join(dllpath, 'libwhisk.so')
# import pdb; pdb.set_trace()
os.environ['PATH'] += os.pathsep + os.pathsep.join(['.', '..', dllpath])
name = find_library('whisk')
if not name:
    name = lib
cReader = CDLL(name)

cReader.Get_Stack_Dimensions_px.restype = c_int
cReader.Get_Stack_Dimensions_px.argtypes = [
    POINTER(c_char),  # filename
    POINTER(c_int),  # width
    POINTER(c_int),  # height
    POINTER(c_int),  # depth
    POINTER(c_int)]  # kind

cReader.Read_Tiff_Stack_Into_Buffer.restype = c_int
cReader.Read_Tiff_Stack_Into_Buffer.argtypes = [
    POINTER(c_char),  # filename
    POINTER(c_uint8)]  # buffer

_bpp = {1: numpy.uint8,
        2: numpy.uint16,
        4: numpy.float32,
        8: numpy.float64}


def readstack(filename):
    width, height, depth, kind = c_int(0), c_int(0), c_int(0), c_int(0)
    if not os.path.exists(filename):
        raise IOError("File not found. (%s)" % filename)
    if (not cReader.Get_Stack_Dimensions_px(filename,
                                            byref(width),
                                            byref(height),
                                            byref(depth),
                                            byref(kind))):
        raise IOError("Couldn't read dimensions for %s" % filename)
    stack = zeros((depth.value, height.value, width.value), dtype=_bpp[kind.value])
    cReader.Read_Tiff_Stack_Into_Buffer(filename,
                                        stack.ctypes.data_as(POINTER(c_uint8)))
    return stack


class TiffReader(StackReader):
    def __init__(self, filename):
        StackReader.__init__(self, readstack(filename))
