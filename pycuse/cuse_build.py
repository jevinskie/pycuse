import importlib.resources

import cffi
from path import Path

pkgdir = Path(__file__).parent

ffibuilder = cffi.FFI()
ffibuilder.set_source("pycuse._cuse", open(pkgdir / "cuse.c").read())
ffibuilder.cdef(open(pkgdir / "cuse.h").read())

if __name__ == "__main__":
    ffibuilder.compile(verbose=True)
