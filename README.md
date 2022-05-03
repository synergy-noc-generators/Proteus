# Proteus: NoC simulator on FPGA using HLS
In this work, we want to design a functional NoC that can send ackets to and from various random nodes in the topology and trying to collect the data for the run-time metrics like average packet delay, max package latency, and average throughput. Although we might not get same values of what we get from GARNET, we expect to get similar trends in the hardware implementation as well.

# Instruction
## Vitis HLS
To run one of the topology: Ring/Mesh/Torus, user will need to first start with Vitis HLS 2021.1. After open Vitis HLS, execpt the main.cpp file should go to testbench, others should all go to source. Hence, the user will be able to run the C-level simulation. In addition, in the "common.h" file, the user can specify the number of nodes they want. And they can find what routing algorithm and traffic pattern we support. In "main.cpp" file, user can specify all these parameters including: Deadlock threshold, Number of packets per node, Packet inject period (1/Inject Rate), Routing Algorithm, Traffic Pattern.

To run sythesis for the Vitis resource utilization report and RTL code export, the user will need to specify the top funtion in "Project->Project Settings". The top function should be the topology name like "mesh". The next step after sythesis will be "Export RTL". This will generate a IP that can be used in vivado design.

## Vivado
User can open up Vivado and add the folder that contains this IP to "IP Catalog". Then, create a block design and add a zynq core and the IP to the design. Notice that we are using ultra96 in our design. Then, running the block automation and connect automation, and a block design similar to Figure 1 should be done. Hence, the user can "create HDL weapper" and run "Generate bistream". After the generation is done, user can export the hardware and put the "*.bit" and "*.hwh" file on FPGA board. And the user can open up the jupternotebook we put in the "script" directory. Replacing the bit file name and those control parameters, user should be able to run the design easily.

![ block_design_mesh](./image/mesh.PNG "block_design_mesh")


