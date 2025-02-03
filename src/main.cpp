#include <windows.h>
#include <tlhelp32.h>
#include <thread>
#include <vector>
#include <string>
#include <shellapi.h>
#include <shlobj.h>
#include <iostream>

const std::vector<std::string> MYDOCKFINDER_PROCESS_NAMES = {
    "Mydock.exe", "dockmod64.exe", "dock.exe", "dockmod.exe", "Dock_64.exe", "Dock_32.exe"
};

const std::string MYDOCKFINDER_PATH = "C:\\Program Files\\MyDockFinder\\Dock_64.exe"; // Percorso principale di MyDockFinder

NOTIFYICONDATA nid = {};

bool is_process_running(const std::string& process_name) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

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
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return;
    }

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
                    }
                }
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
}

void start_process(const std::string& process_path) {
    ShellExecuteA(nullptr, "open", process_path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
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
                    desktopRect.bottom == mi.rcMonitor.bottom &&
                    !(GetWindowLong(hwnd, GWL_STYLE) & WS_BORDER));
        }
    }

    return false;
}

bool is_screen_sharing() {
    HWND hwnd = GetForegroundWindow();
    if (hwnd) {
        char className[256];
        GetClassNameA(hwnd, className, sizeof(className));
        std::string classNameStr(className);
        // Aggiungi qui i nomi delle classi delle finestre di condivisione dello schermo
        if (classNameStr.find("ScreenShare") != std::string::npos || classNameStr.find("Zoom") != std::string::npos) {
            return true;
        }
    }
    return false;
}

void wait_for_fullscreen_change(bool& dockfinder_running, const std::vector<std::string>& process_names, const std::string& process_path) {
    while (true) {
        if (is_fullscreen() && !is_screen_sharing() && dockfinder_running) {
            stop_process(process_names);
            dockfinder_running = false;
        } else if (!is_fullscreen() && !dockfinder_running) {
            start_process(process_path);
            dockfinder_running = true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == 1) {
                PostQuitMessage(0);
            }
            break;

        case WM_DESTROY:
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

bool request_admin_privileges(const std::string& executablePath) {
    SHELLEXECUTEINFO sei = { sizeof(sei) };
    sei.lpVerb = "runas";
    sei.lpFile = executablePath.c_str();
    sei.hwnd = NULL;
    sei.nShow = SW_NORMAL;

    if (!ShellExecuteEx(&sei)) {
        DWORD dwError = GetLastError();
        if (dwError == ERROR_CANCELLED) {
            std::cout << "L'utente ha rifiutato la richiesta di privilegi di amministratore." << std::endl;
        }
        return false;
    }
    return true;
}

void add_to_startup(const std::string& executablePath) {
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        RegSetValueEx(hKey, "MyDockFinderToggle", 0, REG_SZ, (BYTE*)executablePath.c_str(), executablePath.size() + 1);
        RegCloseKey(hKey);
    }
}

std::string get_current_executable_path() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::string(path);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Ottieni il percorso dell'eseguibile corrente
    std::string executablePath = get_current_executable_path();

    // Richiedi i privilegi di amministratore al primo avvio
    if (!request_admin_privileges(executablePath)) {
        return 1;
    }

    // Aggiungi l'applicazione all'avvio del computer
    add_to_startup(executablePath);

    const char* className = "MyDockFinderToggleClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, className, "MyDockFinderToggle", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr, hInstance, nullptr);

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_APP + 1;
    nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    strcpy_s(nid.szTip, "MyDockFinder Toggle");

    Shell_NotifyIcon(NIM_ADD, &nid);

    bool dockfinder_running = true;
    std::thread monitor_thread(wait_for_fullscreen_change, std::ref(dockfinder_running), MYDOCKFINDER_PROCESS_NAMES, MYDOCKFINDER_PATH);
    monitor_thread.detach();

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}