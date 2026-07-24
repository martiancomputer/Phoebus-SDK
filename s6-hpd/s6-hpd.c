/*
 * s6-hpd — a hotplug daemon for s6 / Phoebus-OS.
 *
 * Listens on the kernel uevent netlink socket (NETLINK_KOBJECT_UEVENT) and
 * dispatches every device event to a handler program, with the event's
 * variables placed in the handler's environment (ACTION, SUBSYSTEM, DEVPATH,
 * and any others the kernel provides). This is the piece procd/udev give you
 * that s6 does not — reacting to kernel device events — done the s6 way: one
 * small, supervised, do-one-thing daemon. No ubus, no config language, no magic.
 *
 * Design notes / sensible-engineering choices:
 *   - Netlink multicast, NOT the legacy /proc/sys/kernel/hotplug helper (which
 *     fork+exec's a process *from the kernel* on every event -- slow and racy).
 *   - Handlers are fire-and-forget: SIGCHLD is ignored, so the kernel reaps them
 *     and we never accumulate zombies. A hotplug handler's exit status is not
 *     something we act on.
 *   - The env array points straight into the receive buffer (the kernel already
 *     hands us NUL-separated KEY=VALUE strings), so dispatch allocates nothing.
 *   - Runs as a supervised longrun; if it dies, s6 restarts it. It logs to
 *     stderr (i.e. to its s6 log chain / the console), never daemonizes.
 *   - FAILURES ARE LOUD. A hotplug daemon that silently does nothing is worse
 *     than one that crashes: we check the handler up front and report a failed
 *     exec from the child, so a broken dispatch can never look like "no events".
 *
 * Usage: s6-hpd [-v] [handler]   (default handler: /etc/s6-hpd/dispatch)
 *        -v   log every received event to stderr (the s6 log chain / console)
 */

#define _GNU_SOURCE
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>

#define UEVENT_BUFSZ   8192   /* kernel uevents are well under this */
#define MAX_ENV         128   /* plenty for a single event */

int main (int argc, char **argv)
{
	const char *handler = "/etc/s6-hpd/dispatch";
	int verbose = 0;

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-v")) verbose = 1;
		else handler = argv[i];
	}

	/* Fire-and-forget handlers: SIG_IGN on SIGCHLD makes the kernel auto-reap,
	   so no zombies without us ever calling waitpid(). */
	signal(SIGCHLD, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);

	/* Fail loudly at startup rather than silently doing nothing per event. */
	if (access(handler, X_OK) < 0)
		fprintf(stderr, "s6-hpd: warning: handler %s not executable: %s\n",
		        handler, strerror(errno));

	int fd = socket(AF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_KOBJECT_UEVENT);
	if (fd < 0) { fprintf(stderr, "s6-hpd: fatal: socket: %s\n", strerror(errno)); return 111; }

	/* Grow the receive buffer so event bursts (e.g. coldplug replay) don't drop.
	   RCVBUFFORCE needs CAP_NET_ADMIN, which we have as root; fall back if not. */
	int rcvbuf = 1 << 20;
	if (setsockopt(fd, SOL_SOCKET, SO_RCVBUFFORCE, &rcvbuf, sizeof rcvbuf) < 0)
		setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof rcvbuf);

	struct sockaddr_nl sa;
	memset(&sa, 0, sizeof sa);
	sa.nl_family = AF_NETLINK;
	sa.nl_groups = 1;          /* the kernel uevent multicast group */
	sa.nl_pid    = 0;          /* let the kernel assign a unique unicast id */
	if (bind(fd, (struct sockaddr *)&sa, sizeof sa) < 0) {
		fprintf(stderr, "s6-hpd: fatal: bind: %s\n", strerror(errno));
		return 111;
	}

	fprintf(stderr, "s6-hpd: listening on uevent netlink; handler=%s%s\n",
	        handler, verbose ? " (verbose)" : "");

	char buf[UEVENT_BUFSZ + 1];
	for (;;) {
		ssize_t n = recv(fd, buf, UEVENT_BUFSZ, 0);
		if (n < 0) {
			if (errno == EINTR) continue;
			fprintf(stderr, "s6-hpd: recv: %s\n", strerror(errno));
			continue;
		}
		if (n == 0) continue;
		buf[n] = '\0';

		/* Two message formats arrive on this socket: the kernel's
		   "ACTION@DEVPATH\0KEY=VALUE\0..." and libudev's "libudev\0<binary>".
		   We only handle the kernel format; skip anything libudev-shaped. */
		if ((size_t)n >= 8 && memcmp(buf, "libudev", 8) == 0) continue;

		/* Build envp directly from the KEY=VALUE entries. The first NUL-
		   terminated chunk is the "action@devpath" summary line -> skip it. */
		char *envp[MAX_ENV];
		int e = 0;
		envp[e++] = (char *)"PATH=/bin:/sbin:/usr/bin:/usr/sbin";
		size_t i = strlen(buf) + 1;
		while (i < (size_t)n && e < MAX_ENV - 1) {
			char *entry = buf + i;
			size_t len = strlen(entry);
			if (len && memchr(entry, '=', len)) envp[e++] = entry;
			i += len + 1;
		}
		envp[e] = NULL;

		if (verbose)
			fprintf(stderr, "s6-hpd: event %s (%d vars)\n", buf, e - 1);

		if (e < 2) continue;   /* nothing but PATH -> not a real event */

		pid_t pid = fork();
		if (pid < 0) { fprintf(stderr, "s6-hpd: fork: %s\n", strerror(errno)); continue; }
		if (pid == 0) {
			char *av[2] = { (char *)handler, NULL };
			execve(handler, av, envp);
			/* Never silent: a broken handler must not look like "no events". */
			fprintf(stderr, "s6-hpd: exec %s: %s\n", handler, strerror(errno));
			_exit(127);
		}
		/* parent: do not wait; the handler is reaped by SIG_IGN */
	}
}
