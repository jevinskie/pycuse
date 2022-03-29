from typing import Final

from ._cuse import ffi, lib

CUSE_UNRESTRICTED_IOCTL: Final[int] = 1 << 0


@ffi.def_extern()
def my_callback(x, y):
    return 42
