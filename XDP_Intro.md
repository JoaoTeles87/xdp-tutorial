# Introduction to XDP (Express Data Path)

## What is XDP?
XDP (Express Data Path) is a high-performance, programmable network data path that is integrated directly into the Linux kernel. It allows developers to attach **eBPF (extended Berkeley Packet Filter)** programs to the network interface card (NIC) driver.

## Why XDP?
Traditional Linux networking involves a heavy stack (allocate sk_buff, protocol parsing, traversing netfilter hooks, socket buffers, etc.). For high-speed packet processing (like DDoS mitigation, load balancing, or specialized routing), this overhead can be prohibitive.

XDP solves this by processing packets **before** they enter the standard kernel networking stack.

## Architecture & Hooks
1.  **Packet Arrival**: A packet arrives at the NIC and is DMA'd to a ring buffer.
2.  **XDP Hook**: The XDP program is executed immediately by the NIC driver (in Native mode) or shortly after (in Generic/SKB mode).
3.  **Verdict**: The program inspects the raw packet data and returns a verdict:
    -   `XDP_DROP`: Drop the packet immediately (high performance for firewalls).
    -   `XDP_PASS`: Pass the packet to the standard kernel stack.
    -   `XDP_TX`: Bounce the packet back out the same interface (used for simple forwarding or echo).
    -   `XDP_REDIRECT`: Send packet to another interface or CPU.
    -   `XDP_ABORTED`: Error condition.

## Advantages
-   **Speed**: Can process millions of packets per second.
-   **Safety**: eBPF verifier ensures programs won't crash the kernel.
-   **Integration**: Works with existing kernel routing tables and userspace tools.

## Use Cases
-   DDoS protection (dropping malicious traffic early).
-   Load Balancing (layer 4 forwarding).
-   Monitoring and Sampling.
-   Simple packet rewriting (like our custom Ping Responder).
