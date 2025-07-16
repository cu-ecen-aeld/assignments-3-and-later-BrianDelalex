# faulty-oops analysis

The full kernel fault
```
01| Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000
02| Mem abort info:
03|   ESR = 0x0000000096000045
04|   EC = 0x25: DABT (current EL), IL = 32 bits
05|   SET = 0, FnV = 0
06|   EA = 0, S1PTW = 0
07|   FSC = 0x05: level 1 translation fault
08| Data abort info:
09|   ISV = 0, ISS = 0x00000045
10|  CM = 0, WnR = 1
11| user pgtable: 4k pages, 39-bit VAs, pgdp=0000000041b63000
12| [0000000000000000] pgd=0000000000000000, p4d=0000000000000000, pud=0000000000000000
13| Internal error: Oops: 0000000096000045 [#1] SMP
14| Modules linked in: hello(O) faulty(O) scull(O)
15| CPU: 0 PID: 152 Comm: sh Tainted: G           O       6.1.44 #1
16| Hardware name: linux,dummy-virt (DT)
17| pstate: 80000005 (Nzcv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--)
18| pc : faulty_write+0x10/0x20 [faulty]
19| lr : vfs_write+0xc8/0x390
20| sp : ffffffc008db3d20
21| x29: ffffffc008db3d80 x28: ffffff8001b30d40 x27: 0000000000000000
22| x26: 0000000000000000 x25: 0000000000000000 x24: 0000000000000000
23| x23: 000000000000000c x22: 000000000000000c x21: ffffffc008db3dc0
24| x20: 0000005560e854f0 x19: ffffff8001bfa700 x18: 0000000000000000
25| x17: 0000000000000000 x16: 0000000000000000 x15: 0000000000000000
26| x14: 0000000000000000 x13: 0000000000000000 x12: 0000000000000000
27| x11: 0000000000000000 x10: 0000000000000000 x9 : 0000000000000000
28| x8 : 0000000000000000 x7 : 0000000000000000 x6 : 0000000000000000
29| x5 : 0000000000000001 x4 : ffffffc000787000 x3 : ffffffc008db3dc0
30| x2 : 000000000000000c x1 : 0000000000000000 x0 : 0000000000000000
31| Call trace:
32|  faulty_write+0x10/0x20 [faulty]
33|  ksys_write+0x74/0x110
34|  __arm64_sys_write+0x1c/0x30
35|  invoke_syscall+0x54/0x130
36|  el0_svc_common.constprop.0+0x44/0xf0
37|  do_el0_svc+0x2c/0xc0
38|  el0_svc+0x2c/0x90
39|  el0t_64_sync_handler+0xf4/0x120
40|  el0t_64_sync+0x18c/0x190
41| Code: d2800001 d2800000 d503233f d50323bf (b900003f) 
42| ---[ end trace 0000000000000000 ]---
```

**Analysing the full kernel fault**

In line 1 we can see what happened. For instance a NULL pointer as been dereferenced.  
To localize the faulty line we can look at the Call trace that start at line 31.  
In Line 32 the call trace tells us that the error occured in module faulty (at the end of line between brackets).  
Still in line 32 we can see the name of the function responsible for the fault, faulty_write for instance. After the name of the function we can see +0x10/0x20, this means that the error is 16 bytes into the function and that the total size of the function is 32 bytes.

To summarize, the module faulty encountered a NULL pointer error 16 bytes into the function faulty_write.  