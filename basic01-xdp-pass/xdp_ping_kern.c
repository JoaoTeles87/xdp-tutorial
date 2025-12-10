#include <linux/bpf.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

#define ICMP_ECHO_REQUEST 8
#define ICMP_ECHO_REPLY 0

SEC("xdp")
int  xdp_ping(struct xdp_md *ctx)
{
    void *data_end = (void *)(long)ctx->data_end;
    void *data = (void *)(long)ctx->data;
    struct ethhdr *eth = data;
    long result = XDP_PASS;

    /* 1. Parse Ethernet Header */
    if (data + sizeof(*eth) > data_end)
        return XDP_PASS;

    if (eth->h_proto != bpf_htons(ETH_P_IP))
        return XDP_PASS;

    /* 2. Parse IP Header */
    struct iphdr *iph = data + sizeof(*eth);
    if ((void *)(iph + 1) > data_end)
        return XDP_PASS;

    if (iph->protocol != IPPROTO_ICMP)
        return XDP_PASS;

    /* 3. Parse ICMP Header */
    struct icmphdr *icmp = (void *)iph + (iph->ihl * 4);
    if ((void *)(icmp + 1) > data_end)
        return XDP_PASS;

    /* 4. Check for Echo Request */
    if (icmp->type == ICMP_ECHO_REQUEST) {
        
        /* 5. Swap MAC Addresses */
        unsigned char tmp_mac[ETH_ALEN];
        __builtin_memcpy(tmp_mac, eth->h_dest, ETH_ALEN);
        __builtin_memcpy(eth->h_dest, eth->h_source, ETH_ALEN);
        __builtin_memcpy(eth->h_source, tmp_mac, ETH_ALEN);

        /* 6. Swap IP Addresses */
        __be32 tmp_ip = iph->daddr;
        iph->daddr = iph->saddr;
        iph->saddr = tmp_ip;

        /* 7. Change ICMP Type to Echo Reply */
        // Old Type: 8 (0x08), Code: 0. Word: 0x0800
        // New Type: 0 (0x00), Code: 0. Word: 0x0000
        icmp->type = ICMP_ECHO_REPLY;

        /* 8. Recalculate Checksum (Incremental Update)
         * Since we changed 0x0800 to 0x0000 (decreasing data sum by 0x0800),
         * we must INCREASE the checksum by 0x0800 to maintain validity.
         * csum_field = csum_field - old_val + new_val (generic formula) -> this is for data_sum
         * Checksum Equation: ~(DataSum) = Checksum
         * If DataSum decreases by X, Checksum must increase by X.
         */
        
        // We need to do 1's complement arithmetic
        // 0x0800 in network order is 0x0800 (BE).
        // Let's assume the checksum field is properly aligned.
        
        __u32 csum = bpf_ntohs(icmp->checksum);
        csum += 0x0800;
        if (csum > 0xFFFF)
             csum -= 0xFFFF;
        
        icmp->checksum = bpf_htons(csum);

        /* 9. Send Packet Back */
        result = XDP_TX;
    }

    return result;
}

char _license[] SEC("license") = "GPL";
