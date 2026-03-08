import ctypes

# Load DLL
dll_path = r"C:\Users\nadav\source\repos\cmd_box\x64\Release\cmdbox.dll"

# Load DLL
mylib = ctypes.CDLL(dll_path)

# fncmdbox(const wchar_t*)
mylib.fncmdbox.argtypes = [ctypes.c_wchar_p]
mylib.fncmdbox.restype  = ctypes.c_int

# Call the function with ONE wide string
result = mylib.fncmdbox("ls")

if result:
    print(f"Returned with code: {result}")
    input("Press enter to exit...")