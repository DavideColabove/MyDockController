# MyDockFinder Toggle Utility

This program is a utility designed to enhance the gaming experience by automatically managing MyDockFinder processes. It stops all related processes when a fullscreen application (not maximized) is launched and restarts them when the fullscreen application is closed.

---

## Features
- Detects when a fullscreen application is active.
- Stops all MyDockFinder processes during fullscreen applications.
- Restarts MyDockFinder processes automatically after exiting fullscreen mode.

---

## Requirements
- **Windows OS**: The program uses Windows-specific APIs for process management.
- **Administrator Privileges**: Required to terminate and restart processes.
- **MyDockFinder Path**: Replace the path in the code with the correct installation directory of MyDockFinder (default: `C:\Program Files\MyDockFinder\Dock_64.exe`).

---

## Installation and Usage

### 1. Replace MyDockFinder Path
Open the source code and update the following line with the correct path to `Dock_64.exe`:
```cpp
const std::string MYDOCKFINDER_PATH = "C:\\Program Files\\MyDockFinder\\Dock_64.exe";
```

### 2. Compile the Code
To compile the program, you can use either `g++` or `gcc`. Below are the commands:

#### Using `g++`:
```bash
g++ -o mydockfinder_toggle mydockfinder_toggle.cpp -lgdi32 -luser32
```
#### Using `g++`:
```bash
gcc -o mydockfinder_toggle mydockfinder_toggle.cpp -lgdi32 -luser32
```

Ensure you have the necessary compilers installed and available in your system's PATH.

---

### 3. Run the Program as Administrator
To ensure the program works as intended, it must be executed with administrator privileges. Follow these steps:

1. Locate the compiled executable (`mydockfinder_toggle.exe`).
2. Right-click on the file and select **"Run as administrator"**.
3. Confirm any User Account Control (UAC) prompts.

Running as administrator is necessary for managing system processes.

---

### 4. Add to Startup
To automatically start the program when your system boots, follow these steps:

1. Press `Win + R` to open the **Run** dialog.
2. Type `shell:startup` and press Enter. This opens the Startup folder.
3. Copy the compiled executable (`mydockfinder_toggle.exe`) into this folder.

The program will now launch automatically every time the system starts.

---

## Disclaimer
- **Third-Party Software**: This program is **not affiliated** with the developers of MyDockFinder.
- **Personal Utility**: It is a personal implementation aimed at enhancing gaming efficiency by optimizing MyDockFinder's behavior.
- **Use at Your Own Risk**: This utility modifies system processes. Ensure you understand its functionality before using it.

---

## Support
If you experience issues or need to adjust the program for your system, follow these steps:

1. **Verify Process Names**:
   - Open Task Manager and confirm the MyDockFinder process names match those listed in the `MYDOCKFINDER_PROCESS_NAMES` array in the source code.
   - Update the list if there are discrepancies.

2. **Administrator Privileges**:
   - Ensure the program is run as administrator.

3. **Correct Path**:
   - Replace `MYDOCKFINDER_PATH` in the code with the actual path to the `Dock_64.exe` executable. For example:
     ```cpp
     const std::string MYDOCKFINDER_PATH = "C:\\Program Files\\MyDockFinder\\Dock_64.exe";
     ```

4. **Debugging**:
   - Run the program in a command prompt to view detailed output and identify potential issues.

5. **Contact Me**:
   - You can find my email address as well as other platforms on the main page of my account, feel free to ask me about anything!


Feel free to adapt or improve the program as needed for your specific use case!
