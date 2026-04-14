# OS_PROJECT
An Operating System project based on XV6 that involves implementing and modifying system calls, along with a multithreaded file management system enabling concurrent file operations.

## 📌 Overview

This repository contains our Operating Systems projects based on the xv6 OS.  
Each project focuses on implementing and understanding core OS concepts.

### 🌿 Branching Information
- **Project 1**: Each team member worked on their own isolated branch. These branches were **not merged** into `main`. To view a member's individual work and documentation, please switch to their respective branch. (**feature/<`name`>**)
- **Project 2**: Each team member initially worked in their own separate branch, and all individual contributions have been combined and merged into the **`project2-final`** branch.  (**project2-<`name`>**)

---

## 👨‍💻 Team Members

* VISHNU TEJA VASAM
* VEM SAI PRATHIK REDDY
* SAI TEJA
* Member 4
* Member 5
* VENNELA PURNA CHANDRA

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

## 👤 Sai Teja

### 🔹 System Calls:
- `getchildcount(void)`

### 🔹 Description:
- Implemented a system call to count the number of child processes of the current process.
- Traverses the process table in xv6 to identify child processes.
- Returns the total number of child processes.
- Helps in understanding process hierarchy and parent-child relationships in OS.

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

## 👤 VENNELA PURNA CHANDRA

### 🔹 System Calls:
- `initlock_6()`
- `acquire_6(lock_id)`
- `release_6(lock_id)`

### 🔹 Description:
- Designed user-level process synchronization primitives to coordinate execution across multiple processes.
- Added a global array of locks (`int ulocks[10]`) synchronized by a kernel struct spinlock.
- `acquire_6(lock_id)` requests a lock and puts the caller to sleep if held, efficiently avoiding busy-waiting.
- `release_6(lock_id)` successfully releases the lock and wakes up waiting processes.
- Created and successfully validated process coordination via the remote application `lock_test_6.c`.


---

### 🔹 Project 2: Multithreading

**Folder:** `G22_Project2_Multithreading`

**Objective:**
Implement a multithreaded file management system in C using POSIX threads (`pthreads`). It is a standalone Linux/WSL program that provides an interactive CLI for performing common file operations concurrently.

**Key Features:**
- **9 Core Operations**: Read, Write, Delete, Rename, Copy, Metadata, Compress (zlib), Decompress, Logging.
- **Thread-safe**: Per-file reader-writer locks prevent data races.
- **Deadlock-free**: Alphabetical lock ordering prevents deadlocks during rename operations.
- **Graceful Shutdown**: `SIGINT` handler ensures active threads safely finish their operations before exiting.

**Team Contributions:**
- **Vishnu**: Core Architecture, Synchronization (File Lock Table), CLI menu, Thread dispatching.
- **Pratik**: Concurrent File Reading & Exclusive File Writing.
- **Akshara**: File Deletion & Deadlock-free File Renaming.
- **Vishesh**: File Copying & Metadata Display.
- **Saiteja**: Thread-safe Error Handling & Logging subsystem.
- **Purna**: File Compression & Decompression using zlib.

---

## ⚙️ How to Run

### Project 1: System Call Customization

```bash
# Navigate to project folder
cd G22_Project1_xv6_CustomizeSystemCalls/xv6-riscv

# Build and run xv6
make qemu
```

### Project 2: Multithreaded File Management System

**Prerequisites:** Linux/WSL, GCC (`-pthread`), zlib (`-lz`).  
*(To install zlib on Debian/Ubuntu: `sudo apt-get install zlib1g-dev`)*

```bash
# Navigate to project folder
cd G22_Project2_Multithreading

# Build the project
make clean
make

# Run the application
make run
# or
./file_manager
```

---

## 📄 Documentation

Detailed explanations and screenshots are available in the `docs/` folder of each project and in the respective team members' branches.


