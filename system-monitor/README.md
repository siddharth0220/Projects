# 🖥️ System Resource Monitor Using C++

## 📘 Overview
The **System Resource Monitor** is a terminal-based C++ application designed to monitor and display **real-time CPU and memory usage** in Linux systems.  
It reads system and process-level data from the **/proc** filesystem and provides a lightweight performance tracking interface similar to tools like `top` or `htop`.

This project demonstrates **Linux system programming**, **file handling**, and **process management** using standard C++ libraries.

---

## 🚀 Features
- Displays **real-time CPU and memory usage**
- Shows **active processes** with PID, Name, CPU%, MEM%, and RSS
- Supports **interactive sorting** by CPU or memory usage (`c` and `m` keys)
- Provides a **refreshing live console view**
- Uses modular C++ design with multiple source and header files

---

## 🗂️ Project Structure

```
SRC/
│
├── .vscode/
│   ├── c_cpp_properties.json
│   ├── launch.json
│   └── settings.json
│
├── main.cpp              # Entry point, UI rendering, user input handling
├── process.cpp           # Process data parsing from /proc/[pid]/stat
├── process.hpp           # Process class declarations
├── system_info.cpp       # CPU and Memory usage calculation
├── system_info.hpp       # System info header definitions
├── utils.cpp             # Helper functions for string and data operations
└── utils.hpp             # Utility function declarations
```

---

## ⚙️ Requirements
- **Operating System:** Linux (Ubuntu preferred)
- **Compiler:** g++ (C++17 or above)
- **Libraries:** Standard C++ STL (no external dependencies)

---

## 🧩 How to Build and Run

1. Open a terminal in the `SRC` folder.  
2. Compile all source files:
   ```bash
   g++ -std=c++17 main.cpp process.cpp system_info.cpp utils.cpp -o system_monitor
   ```
3. Run the executable:
   ```bash
   ./system_monitor
   ```

---

## 🖼️ Output Preview

Below is a sample output view from the terminal:

```
PID    NAME             CPU%   MEM%   RSS(kB)
---------------------------------------------
1      init             0.03   1.51   120240
118    dockerd          0.03   1.35   107396
5772   system-monitor   0.00   0.05   3712
...
---------------------------------------------
Controls: c=CPU sort | m=MEM sort | q=quit
```

---

## 🧠 Learning Outcomes
- Working with **Linux /proc filesystem**
- Understanding **process scheduling and CPU statistics**
- Using **file I/O and modular programming in C++**
- Developing **real-time CLI tools**

---

## 🔮 Future Enhancements
- Add **graphical UI** using Qt or GTK
- Include **network usage monitoring**
- Enable **remote system monitoring** over SSH
- Integrate **log export** functionality (CSV/JSON)

---

## 👨‍💻 Author
**Siddharth Kumar Pradhan**  
Department of Computer Science and Engineering  
**ITER, Siksha 'O' Anusandhan University, Bhubaneswar**  
**Date:** 9th November 2025
