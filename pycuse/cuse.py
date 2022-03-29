from typing import Final, Sequence

from ._cuse import ffi, lib

CUSE_UNRESTRICTED_IOCTL: Final[int] = 1 << 0


def cstr_array(strs: Sequence[str]):
    cstrs = [ffi.new("char[]", s.encode("utf-8")) for s in strs]
    res = ffi.new(f"char*[{len(strs)}]", cstrs)
    return res


class CUSEDev:
    def __init__(self, name: str, fg: bool = True, debug: bool = True):
        args = [f"pycuse-{name}"]
        if fg:
            args.append("-f")
        if debug:
            args.append("-d")
        ci = ffi.new("struct cuse_info *")
        print(ci)
        cargs = cstr_array(args)
        print(cargs)
        ci.flags = CUSE_UNRESTRICTED_IOCTL
        ci.dev_info_argc = 1
        ci.dev_info_argv = cstr_array([f"DEVNAME={name}"])
        ops = ffi.new("struct cuse_lowlevel_ops *")
        lib.cuse_lowlevel_main(len(args), cargs, ci, ops, ffi.NULL)
