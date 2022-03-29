#!/usr/bin/env python3

from pycuse import cuse

print("pycuse hello")

cd = cuse.CUSEDev("cuse-hello", fg=True, debug=True)
print(cd)
