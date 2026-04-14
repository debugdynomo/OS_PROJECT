# OS_PROJECT
An Operating System project based on XV6 that involves implementing and modifying system calls, along with a multithreaded file management system enabling concurrent file operations.

## 📌 Overview

This repository contains our Operating Systems projects based on the xv6 OS.  
Each project focuses on implementing and understanding core OS concepts.

---

## 👨‍💻 Team Members

* VISHNU TEJA VASAM
* VEM SAI PRATHIK REDDY
* Member 3
* Member 4
* Member 5
* Member 6

---

## 📂 Project Structure

### 🔹 Project 1: System Call Customization

**Folder:** `G22_Project1_xv6_CustomizeSystemCalls`

---

## 🎯 Objective

Modify and add new system calls in xv6 to understand kernel-user interaction, synchronization, and process management.

---

# ⚙️ System Calls Implemented (Member-wise)

---

## 👤 VISHNU TEJA VASAM

### 🔹 System Calls:
- `send(int value)`
- `recv(void)`
- `sleep(int ticks)`

### 🔹 Description:
- Implemented IPC using message passing with a single-slot kernel buffer.
- Used spinlocks to ensure mutual exclusion.
- Eliminated busy waiting using sleep-wakeup mechanism.
- Implemented `sleep` syscall using timer ticks and scheduler.
- Demonstrated parent-child communication using `fork()`.

---

## 👤 VEM SAI PRATHIK REDDY

### 🔹 System Calls:
- `fork_stats_25(int priority, int max_children)`

### 🔹 Description:
- Extended traditional `fork()` with priority assignment and child process limiting.
- Tracks process creation statistics including attempts, successful forks, and blocked forks.
- Blocks fork requests when `max_children` limit is reached and returns `-1`.
- Priority level is assigned directly to child process struct upon creation.
- Demonstrated all features through a dedicated test program `testprathik.c`.

---

## 👤 Member 3

### 🔹 System Calls:
- ---
- ---

### 🔹 Description:
- ---
- ---
- ---

---

## 👤 Member 4

### 🔹 System Calls:
- ---
- ---

### 🔹 Description:
- ---
- ---
- ---

---

## 👤 Member 5

### 🔹 System Calls:
- ---
- ---

### 🔹 Description:
- ---
- ---
- ---

---

## 👤 Member 6

### 🔹 System Calls:
- ---
- ---

### 🔹 Description:
- ---
- ---
- ---

---

# 🧠 Key Concepts Covered

- System Calls  
- Kernel ↔ User Interaction  
- Process Creation (`fork`)  
- Inter-Process Communication (IPC)  
- Synchronization (Spinlocks, Sleep/Wakeup)  
- Scheduler & Blocking Mechanisms  
- Timer Interrupts  

---

### 🔹 Project 2: Multithreading

**Folder:** `G22_Project2_Multithreading`

**Objective:**
Implement multithreading support in xv6.

**Features Implemented:**

* ---
* ---
* ---

**Description of Changes:**

* ---
* ---
* ---

---

## ⚙️ How to Run

```bash
# Navigate to project folder
cd G22_Project1_xv6_CustomizeSystemCalls/xv6-riscv

# Build and run xv6
make qemu
```

---

## 📄 Documentation

Detailed explanations and screenshots are available in the `docs/` folder of each project.

---

## 🔀 Contributions

Each team member worked on different parts of the project.
Pull requests include detailed descriptions of individual contributions.

---

## 📝 Notes

* Based on xv6 (RISC-V version)
* Educational project for understanding OS internals
