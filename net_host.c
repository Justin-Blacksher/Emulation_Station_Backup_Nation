#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/sys_domain.h>
#include <net/if.h>
#include <net/if_utun.h>
#include <sys/socket.h>
#include <sys/sys_domain.h>
#include <sys/kern_control.h>
#include "net_host.h"

int mac_utun_alloc(char *ifname_out, int ifname_len) {
    int fd;
    struct sockaddr_ctl sc;
    struct ctl_info ctlInfo;

    fd = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
    if (fd == -1) {
        perror("Opening PF_SYSTEM socket failed");
        return -1;
    }

    memset(&ctlInfo, 0, sizeof(ctlInfo));
    strlcpy(ctlInfo.ctl_name, UTUN_CONTROL_NAME, sizeof(ctlInfo.ctl_name));

    if (ioctl(fd, CTLIOCGINFO, &ctlInfo) == -1) {
        perror("ioctl CTLIOCGINFO failed");
        close(fd);
        return -1;
    }

    sc.sc_id = ctlInfo.ctl_id;
    sc.sc_len = sizeof(sc);
    sc.sc_family = AF_SYSTEM;
    sc.ss_sysaddr = AF_SYS_CONTROL;
    sc.sc_unit = 0; // Kernel auto-assigns available utun slot

    if (connect(fd, (struct sockaddr *)&sc, sizeof(sc)) == -1) {
        perror("connect to utun control failed");
        close(fd);
        return -1;
    }

    socklen_t optlen = ifname_len;
    if (getsockopt(fd, SYSPROTO_CONTROL, UTUN_OPT_IFNAME, ifname_out, &optlen) == -1) {
        perror("getsockopt UTUN_OPT_IFNAME failed");
        close(fd);
        return -1;
    }

    printf("[Host macOS] Allocated interface: %s\n", ifname_out);
    return fd;
}

void configure_mac_tun(const char *dev_name, const char *local_ip, const char *remote_ip) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "sudo ifconfig %s %s %s up", dev_name, local_ip, remote_ip);
    system(cmd);
    printf("[Host macOS] Configured %s with IP %s -> %s\n", dev_name, local_ip, remote_ip);
}

void make_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}