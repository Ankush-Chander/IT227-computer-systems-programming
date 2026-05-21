---
model: ollama@my-qwen3.6:latest
---
# IT227 – Computer Systems Programming

**Course Placement:** Core course for second-year students of the BTech ICT program.  
**Course Format:** 3 hours lecture + 2 hours lab practical per week.  
**Prerequisites:** 
- IT112 & IT113: Introduction to Programming (C Programming)
- IT121: Digital Logic Design and Computer Organization

---

## Course Objective
The course provides an introductory look at the core abstractions in operating systems: processes, virtual memory, and files. It takes an in-depth look at OS services provided by system calls, how system calls work, and how they can be used. Students will become familiar with writing application programs using system calls.

## Text Books
1. Bryant, R. & O'Hallaron, D. *Computer Systems: A Programmer's Perspective*, 3rd Edition, Pearson India, 2016.
2. Arpaci-Dusseau, R. H. & Arpaci-Dusseau, A. C. *Operating Systems: Three Easy Pieces*, 2018, Version 1.00. (Freely available online.)

---

##  Assessment Method
| Assessment Component        | Weightage |
| :-------------------------- | :-------: |
| Lab Programming Assignments |    30%    |
| End-Semester Exam           |    70%    |
| **Total**                   | **100%**  |

---

## Course Outcomes
After successful completion of the course, students will be able to:
1. **Understand** the concept of virtualization, operating system abstraction concepts such as processes and threads, and advanced concepts such as inter-process communication using signals, race conditions, the mutual exclusion problem, and solutions using various locking mechanisms.
2. **Acquire practical knowledge** by developing parallel applications using multiprocessing and multithreading, and performing inter-process communication in systems with shared-memory architecture using signals.

### CO-PO Mapping
| | P1 | P2 | P3 | P4 | P5 | P6 | P7 | P8 | P9 | P10 | P11 | P12 |
|:---|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:---:|:---:|:---:|
| **Mapping** |   |   | X | X | X |   |   |   |   |    |    |    |

---

## Lecture Schedule
|  SI. No.  | Description                                                                                                                                                                                                                                                       | No. of Lectures |
| :-------: | :---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :-------------: |
|     1     | **Introduction:** System Programming Introduction. <br>Key OS abstractions: processes, virtual memory and files, virtual address space, system calls, interrupts, user and kernel mode, process state transition, context switching, saving and restoring context |        3        |
|     2     | **Process** creation, process termination, reaping child processes, putting processes to sleep, loading and running programs, Unix shell, IPC Pipes                                                                                                               |        5        |
|     3     | **Signal** terminology, sending signals, receiving signals, normal and abnormal termination, signal blocking, job control using signals                                                                                                                           |        5        |
|     4     | **Memory** Address translation, segmentation, page tables, TLB, page fault control flow, page replacement policies, Belady’s anomaly, thrashing, case study: Linux VM system                                                                                      |        5        |
|     5     | Opening and closing **files**, Unbuffered I/O vs buffered I/O, directories, file metadata, file sharing, symbolic link, I/O redirection                                                                                                                           |        5        |
|     6     | **Thread** creation, thread termination, reaping terminated threads, thread memory model, shared variables, race conditions                                                                                                                                       |        5        |
|     7     | **Concurrency** Mutual exclusion problem, solutions to mutual exclusion problem using locks/semaphores, deadlocks, necessary conditions for deadlock, dining-philosophers problem, producer-consumer problem, readers-writers problem                             |        5        |
|     8     | **Network Programming:** Communication Layers (Network, Transport), Protocols basics: Internet Protocol (IP), TCP: connection-oriented, UDP: connectionless, standard services and assigned ports, client-server communication using sockets                      |        5        |
| **Total** |                                                                                                                                                                                                                                                                   |     **38**      |


