import ctypes

# Load DLL
mylib = input("The DLL path -> ")

# fncmdbox(const wchar_t*)
mylib.fncmdbox.argtypes = [ctypes.c_wchar_p]
mylib.fncmdbox.restype  = ctypes.c_int

# Call the function with ONE wide string
result = mylib.fncmdbox("whoami /all")
