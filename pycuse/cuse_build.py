import cffi
from path import Path

pkgdir = Path(__file__).parent
cuse_c = open(pkgdir / "cuse.c").read()
cuse_h = open(pkgdir / "cuse.h").read()

ffibuilder = cffi.FFI()
ffibuilder.set_source("pycuse._cuse", cuse_c, libraries=["fuse3"])
ffibuilder.cdef(cuse_h)

if __name__ == "__main__":
    ffibuilder.compile(verbose=True)
