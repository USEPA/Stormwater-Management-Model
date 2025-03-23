"""
EPASWMM Python API

This module provides a Python interface to the EPASWMM library.

"""
import os
import platform
import sys

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
    os.environ["LD_LIBRARY_PATH"] = lib_dir + ":" + os.environ.get("LD_LIBRARY_PATH", "")

elif platform.system() == "Darwin":  # macOS
    # Only set DYLD_LIBRARY_PATH if RPATH fails
    try:
        from epaswmm import _solver  # Test if the module loads without setting path
    except ImportError:
        lib_dir = os.path.join(sys.prefix, "lib")
        os.environ["DYLD_LIBRARY_PATH"] = (
            lib_dir + ":" + os.environ.get("DYLD_LIBRARY_PATH", "")
        )


from epaswmm import *
