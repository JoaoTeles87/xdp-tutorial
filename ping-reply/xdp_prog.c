#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <arpa/inet.h>

// MACROS for easier reading
#ifndef htons
#define htons(x) ((__be16)___constant_swab16((x)))
#endif

SEC("xdp")
int xdp_ping_responder(struct xdp_md *ctx) {
    void *data_end = (void *)(long)ctx->data_end;
    void *data = (void *)(long)ctx->data;

    // 1. Parse Ethernet Header
    struct ethhdr *eth = data;
    if ((void *)(eth + 1) > data_end)
        return XDP_PASS;

    // 2. Check if it's an IP packet (0x0800)
    if (eth->h_proto != htons(ETH_P_IP))
        return XDP_PASS;

    // 3. Parse IP Header
    struct iphdr *iph = (void *)(eth + 1);
    if ((void *)(iph + 1) > data_end)
        return XDP_PASS;

    // 4. Check if it's ICMP
    if (iph->protocol != IPPROTO_ICMP)
        return XDP_PASS;

    // 5. Parse ICMP Header
    struct icmphdr *icmph = (void *)(iph + 1);
    if ((void *)(icmph + 1) > data_end)
        return XDP_PASS;

    // 6. DETECT ECHO REQUEST (Type 8)
    if (icmph->type != 8)
                 return XDP_PASS;

    // 7. SWAP MAC ADDRESSES
    unsigned char tmp_mac[ETH_ALEN];
    __builtin_memcpy(tmp_mac, eth->h_dest, ETH_ALEN);
    __builtin_memcpy(eth->h_dest, eth->h_source, ETH_ALEN);
    __builtin_memcpy(eth->h_source, tmp_mac, ETH_ALEN);

    // 8. SWAP IP ADDRESSES
    __be32 tmp_ip = iph->daddr;
    iph->daddr = iph->saddr;
    iph->saddr = tmp_ip;

    // 9. CHANGE TYPE TO ECHO REPLY (Type 0)
    icmph->type = 0;

    // 10. RECALCULATE CHECKSUM (Incremental Update)
    // Old: Type=8, Code=0 -> 0x0800
    // New: Type=0, Code=0 -> 0x0000
    // Diff: -0x0800.
    // Checksum Equation (RFC 1624): HC' = ~(~HC + m - m')
    // We can simplify: NewChecksum = OldChecksum + 0x0800.
    // Logic: As data sum DECREASES, checksum MUST INCREASE to balance 1s complement sum.

    __u32 csum = ntohs(icmph->checksum);
    csum += 0x0800; // The difference in the 16-bit word
    
    // Handle 16-bit overflow (carry)
    if (csum > 0xFFFF)
        csum -= 0xFFFF;
    
    icmph->checksum = htons(csum);

    // Proof of XDP: Print to trace_pipe
    char fmt[] = "XDP Ping Reply Sent! seq=%d\n";
    // We can print the sequence number to prove it's live
    // Sequence is at offset 6 in ICMP header (un.echo.sequence)
    // But for simplicity, just a message:
    bpf_trace_printk(fmt, sizeof(fmt), ntohs(icmph->un.echo.sequence));

    // 11. TX: Send it back out the same interface
    return XDP_TX; 
}

char _license[] SEC("license") = "GPL";
