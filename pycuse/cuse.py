from typing import Final

from ._cuse import ffi, lib

CUSE_UNRESTRICTED_IOCTL: Final[int] = 1 << 0


class CUSEDev:
    def __init__(self, name: str, fg: bool = True, debug: bool = True):
        args = [f"pycuse-{name}"]
        if fg:
            args.append("-f")
        if debug:
            args.append("-d")
