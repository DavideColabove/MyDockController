#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <thread>
#include <vector>

const std::vector<std::string> MYDOCKFINDER_PROCESS_NAMES = {
    "Mydock.exe", "dockmod64.exe", "dock.exe", "dockmod.exe"
};

const std::string MYDOCKFINDER_PATH = "C:\\Program Files\\MyDockFinder\\Dock_64.exe"; // Percorso principale di MyDockFinder

bool is_process_running(const std::string& process_name) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe)) {
        do {
            if (process_name == pe.szExeFile) {
                CloseHandle(hSnapshot);
                return true;
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return false;
}

void stop_process(const std::vector<std::string>& process_names) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe)) {
        do {
            for (const auto& process_name : process_names) {
                if (process_name == pe.szExeFile) {
                    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                    if (hProcess) {
                        TerminateProcess(hProcess, 0);
                        CloseHandle(hProcess);
                        std::cout << process_name << " terminated." << std::endl;
                    }
                }
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
}

void start_process(const std::string& process_path) {
    if (ShellExecuteA(nullptr, "open", process_path.c_str(), nullptr, nullptr, SW_SHOWNORMAL) > (HINSTANCE)32) {
        std::cout << process_path << " started." << std::endl;
    } else {
        std::cerr << "Failed to start " << process_path << std::endl;
    }
}

bool is_fullscreen() {
    RECT desktopRect;
    HWND hwnd = GetForegroundWindow();

    if (hwnd) {
        HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO mi = { sizeof(mi) };
        if (GetMonitorInfo(monitor, &mi)) {
            GetWindowRect(hwnd, &desktopRect);
            return (desktopRect.left == mi.rcMonitor.left &&
                    desktopRect.top == mi.rcMonitor.top &&
                    desktopRect.right == mi.rcMonitor.right &&
                    desktopRect.bottom == mi.rcMonitor.bottom);
        }
    }

    return false;
}

void wait_for_fullscreen_change(bool& dockfinder_running, const std::vector<std::string>& process_names, const std::string& process_path) {
    while (true) {
        if (is_fullscreen() && dockfinder_running) {
            stop_process(process_names);
            dockfinder_running = false;
        } else if (!is_fullscreen() && !dockfinder_running) {
            start_process(process_path);
            dockfinder_running = true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    bool dockfinder_running = true;

    wait_for_fullscreen_change(dockfinder_running, MYDOCKFINDER_PROCESS_NAMES, MYDOCKFINDER_PATH);

    return 0;
}
