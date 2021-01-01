from ctypes import *

handle = cdll.LoadLibrary("./libjudge.so")
func = handle.main
print("func: ", func)

func.restype = c_int
func.argtypes = c_int, POINTER(c_char_p)
# b"./test", b"1000", b"2048", b"1.in", b"1.out", b"1.tmp.out", b"1.log"
args = (c_char_p * 10)(
    b"judge",
    b"judge",
    b"./main",
    b"1000",
    b"2048",
    b"./tests/1/1.in",
    b"./tests/1/1.out",
    b"1.tmp.out",
    b"-l",
    b"1.log",
)
print(args)
print(111111111111111)
tmp = func(len(args), args)
print(111111111111111)
print(tmp)
