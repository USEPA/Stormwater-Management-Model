"""
EPASWMM Python API

This module provides a Python interface to the EPASWMM library.

"""
import os
import platform
import sys
import importlib.metadata

try:
    from epaswmm import *
except ImportError:
    if platform.system() == "Windows":
        lib_dir = os.path.join(sys.prefix, "bin")
        if hasattr(os, "add_dll_directory"):
            conda_exists = os.path.exists(os.path.join(sys.prefix, "conda-meta"))
            if conda_exists:
                os.environ["CONDA_DLL_SEARCH_MODIFICATION_ENABLE"] = "1"
            os.add_dll_directory(lib_dir)
        else:
            os.environ["PATH"] = lib_dir + ";" + os.environ["PATH"]

    elif platform.system() == "Linux":
        lib_dir = os.path.join(sys.prefix, "lib")
        sys.path.append(lib_dir)
        os.environ["LD_LIBRARY_PATH"] = lib_dir + ":" + os.environ.get("LD_LIBRARY_PATH", "")

    elif platform.system() == "Darwin":  # macOS
        lib_dir = os.path.join(sys.prefix, "lib")
        sys.path.append(lib_dir)

        
        os.environ["DYLD_LIBRARY_PATH"] = lib_dir + ":" + os.environ.get("DYLD_LIBRARY_PATH", "")

        lib_dir = os.path.join(sys.prefix, "bin")
        os.environ["DYLD_LIBRARY_PATH"] = lib_dir + ":" + os.environ.get("DYLD_LIBRARY_PATH", "")

__version__ = importlib.metadata.version('epaswmm')

from epaswmm import *
