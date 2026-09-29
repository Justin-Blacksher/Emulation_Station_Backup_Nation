#ifndef NET_HOST_H
#define NET_HOST_H

int mac_utun_alloc(char *ifname_out, int ifname_len);
void configure_mac_tun(const char *dev_name, const char *local_ip, const char *remote_ip);
void make_nonblocking(int fd);

#endif