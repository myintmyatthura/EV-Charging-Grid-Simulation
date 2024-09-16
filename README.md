# Electric Vehicle Charging Grid Simulation with **Distributed Memory Parallelism in OpenMPI**

---

## A simulation of an Electric Vehicle Charging Grid managed by a distributed Wireless Sensor Network, utilizing OpenMPI for parallel processing and communication.

---

An advanced simulation tool designed to model an Electric Vehicle Charging Grid using distributed memory parallelism with OpenMPI. This project leverages Master-Slave interactions and MPI Virtual Topologies to emulate a charging grid, exploring optimization strategies such as Sphere Topological Architecture. The tool demonstrates significant performance improvements with minimal message exchanges and efficient communication between nodes. Ideal for studying distributed systems and parallel computing in a realistic context.

**Features:**

- Simulation of an Electric Vehicle Charging Grid with distributed Wireless Sensor Network management
- Utilization of Master-Slave interactions for communication and MPI Virtual Topologies for grid emulation
- Optimization through Sphere Topological Architecture for enhanced performance
- Efficient message management and communication with minimal overhead
- Detailed analysis and results on grid sizes, charging ports, and communication efficiency
- Log file output for monitoring and analyzing simulation details

---

**Pre-requisites for execution:**

- OpenMPI library
- C++ Standard Library
- Libraries: `<mpi.h>`, `<iostream>`, `<fstream>`, `<vector>`, `<algorithm>`, `<unistd.h>`

---

**How to use:**

To run the simulation, simply execute the make file to compile and execute the program:

```
make
```
