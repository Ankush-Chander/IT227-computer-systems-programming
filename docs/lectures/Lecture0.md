
**Why study this course?**

## This is the course where programming becomes _real_
```javascript
app.get("/users", async (req, res) => {
    const users = await db.query("SELECT * FROM users");
    res.json(users);
	});
```

a. Layer 1 — Your Application Logic   
b. Layer 2 — Web Framework (Express)  
c. Layer 3 — JavaScript Runtime (Node.js)  
d. Layer 4 — Native Runtime Libraries (libuv, V8)  
e. Layer 5 — System Call Interface  
f. Layer 6 — Operating System Kernel  
g. Layer 7 — Hardware  

## Every serious software engineer eventually needs this knowledge
This course gives you the mental model needed to debug and build real systems.

## You stop being “just an API user”

Most developers only know how to _use_ libraries.
This course teaches you:

- how processes are created
- how threads work internally
- how system calls interact with the OS
- how memory is virtualized
- how synchronization actually works
- how networking happens underneath frameworks

You begin understanding the machinery underneath modern software systems.
That is a huge transition.


## This course explains many mysterious bugs developers face

Ever seen:
- random crashes?  
- deadlocks?  
- memory leaks?  
- zombie processes?  
- programs hanging forever?  
- weird concurrency bugs?  

This course explains _why_ they happen.

And more importantly:  
it teaches you how professionals reason about them.

## You’ll write low-level programs that interact directly with the OS

Instead of only writing application logic, you’ll work with:

- processes  
- signals  
- pipes  
- threads  
- synchronization primitives  
- sockets  
- file descriptors  
- system calls  

This is the foundation of:

- shells
- servers
- databases
- operating systems
- networked applications
## Networking becomes less magical

You use the internet every day.
But after this course, you’ll understand:
- sockets  
- TCP vs UDP  
- client-server communication  
- ports  
- how apps talk over networks  

You’ll realize:
> “The internet is just programs talking to other programs.”


