# ARTHUR
## Description
ARTHUR (A Real Time Heap Usage Reporter) helps visualize memory allocation of C/C++ programs as they run. It does this by intercepting calls to malloc/free, sending that allocation information over to a separate program using named pipes, and then displaying the information as a heat map.
This does mean, however, that if a program has a custom memory allocator, this program will not work. It also will only run in Linux operations systems.

## Usage
There is a precompiled shared library called 'libmalloc.so' from the file allocator_override.cpp. This is the shared library used in order to intercept calls to malloc/free. If it is not there or the library needs to be recompiled, you can compile allocator_override.cpp. The only requirement is that the shared library has the name 'libmalloc.so'.\\
Steps:\\
1) Start up the memory visualizer. There is a compiled version called memory_visualizer. If this does not work, you will have to download Qt then run make.\\
2) Run ```./preload.sh ./target_program```

## GUI
When starting up the memory visualizer, you will see a black square that will turn to gradients from green to red. These blocks represent chunks of memory with the prefix of the address hex value being shown in the top right corner. 
To go into more detail, click on the desired block and it will zoom to that chunk of memory. To zoom back out, use the back button located on the bottom.
A block that is more green means that most of the memory there is free while red means that most of the memory is allocated. Black represents chunks where no memory has been allocated yet.\\
