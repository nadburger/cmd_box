import ctypes

# Load DLL
mylib = ctypes.CDLL(r"C:\Users\nadav\source\repos\cmd_box\x64\Debug\cmdbox.dll")

# fncmdbox(const wchar_t*)
mylib.fncmdbox.argtypes = [ctypes.c_wchar_p]
mylib.fncmdbox.restype  = ctypes.c_int

# Call the function with ONE wide string
result = mylib.fncmdbox("whoami /all")
