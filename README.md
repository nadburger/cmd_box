# cmd_box

When you want to run a command on a Windows machine but you don't want to open a process called cmd.

The argument_checker python file is an example on how to call the dll.

I'm going to add features so stay tuned!

---

## Features

- Takes you string like you'd expect cmd to do  
- Checks if the app you mentioned exists on the C:\Windows and C:\Windows\System32 directories
- Lightweight, minimal and easy to change according to your needs

---

## To do

- Adding full path compatability instead of only relative paths
- Load the executable and run it from the entry point instead of CreateProcess
- Usage of the PATH variables for better compatibility
- Obfuscation of the strings for more stealth 