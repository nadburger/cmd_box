// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the CMDBOX_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// CMDBOX_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef CMDBOX_EXPORTS
#define CMDBOX_API __declspec(dllexport)
#else
#define CMDBOX_API __declspec(dllimport)
#endif

extern "C" __declspec(dllexport) int fncmdbox(const wchar_t* args);
