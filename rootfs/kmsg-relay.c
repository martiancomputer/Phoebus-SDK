/* SPDX-License-Identifier: GPL-2.0-only */
/* kmsg-relay: independent /dev/kmsg reader with bounded UDP datagrams. */
#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static volatile sig_atomic_t stopping;
static void stop(int signal_number) { (void)signal_number; stopping = 1; }

int main(int argc, char **argv)
{
	struct sockaddr_in peer = { .sin_family = AF_INET };
	struct pollfd pollfd;
	char record[16385], datagram[1400], boot[64] = "unknown";
	unsigned long long sequence;
	unsigned long lost = 0;
	char *end;
	long port;
	int fd, sock, bootfd;

	if (argc != 3 || inet_pton(AF_INET, argv[1], &peer.sin_addr) != 1)
		return 2;
	port = strtol(argv[2], &end, 10);
	if (!argv[2][0] || *end || port < 1024 || port > 65535)
		return 2;
	peer.sin_port = htons((unsigned short)port);
	bootfd = open("/proc/sys/kernel/random/boot_id", O_RDONLY | O_CLOEXEC);
	if (bootfd >= 0) {
		ssize_t n = read(bootfd, boot, sizeof(boot) - 1);
		if (n > 0) { boot[n] = 0; boot[strcspn(boot, "\n")] = 0; }
		close(bootfd);
	}
	fd = open("/dev/kmsg", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	sock = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (fd < 0 || sock < 0) { perror("kmsg relay open"); return 1; }
	/* Open the privileged log first, then retain no root credentials. */
	if (setgroups(0, NULL) || setgid(65534) || setuid(65534)) {
		perror("kmsg relay privileges"); return 1;
	}
	signal(SIGTERM, stop);
	signal(SIGINT, stop);
	pollfd.fd = fd; pollfd.events = POLLIN;
	while (!stopping) {
		ssize_t n;
		if (poll(&pollfd, 1, 1000) < 0) {
			if (errno == EINTR) continue;
			break;
		}
		n = read(fd, record, sizeof(record) - 1);
		if (n < 0) {
			if (errno == EPIPE) { ++lost; continue; }
			if (errno == EAGAIN || errno == EINTR) continue;
			perror("kmsg relay read"); break;
		}
		if (!n) break;
		record[n] = 0;
		/* Kernel sequence IDs remain meaningful across relay restarts/replay. */
		if (sscanf(record, "%*u,%llu", &sequence) != 1) { ++lost; continue; }
		for (size_t offset = 0; offset < (size_t)n; offset += 1200) {
			size_t chunk = (size_t)n - offset;
			int prefix;
			if (chunk > 1200) chunk = 1200;
			prefix = snprintf(datagram, sizeof(datagram), "boot=%s record=%llu offset=%zu total=%zd lost=%lu ",
				boot, sequence, offset, n, lost);
			if (prefix < 0 || (size_t)prefix + chunk > sizeof(datagram)) { ++lost; break; }
			memcpy(datagram + prefix, record + offset, chunk);
			if (sendto(sock, datagram, (size_t)prefix + chunk, MSG_DONTWAIT,
				   (struct sockaddr *)&peer, sizeof(peer)) < 0) ++lost;
		}
	}
	close(fd); close(sock);
	return stopping ? 0 : 1;
}
