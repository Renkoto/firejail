/*
 * Copyright (C) 2014-2016 Firejail Authors
 *
 * This file is part of firejail project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
// http://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/?id=770fe30a46a12b6fb6b63fbe1737654d28e84844
// sudo mount -o loop krita-3.0-x86_64.appimage mnt

#include "firejail.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <linux/loop.h>



char *appimage_set(const char *appimage_path) {
	assert(appimage_path);
	EUID_ASSERT();
	
	// check appimage_path
	if (access(appimage_path, R_OK) == -1) {
		fprintf(stderr, "Error: cannot access AppImage file\n");
		exit(1);
	}
	
	EUID_ROOT();
	
	// find or allocate a free loop device to use
	int cfd = open("/dev/loop-control", O_RDWR);
	int devnr = ioctl(cfd, LOOP_CTL_GET_FREE);
	if (devnr == -1) {
		fprintf(stderr, "Error: cannot allocate a new loopback device\n");
		exit(1);
	}
	close(cfd);
	char *devloop;
	if (asprintf(&devloop, "/dev/loop%d", devnr) == -1)
		errExit("asprintf");
		
	int ffd = open(appimage_path, O_RDONLY|O_CLOEXEC);
	int lfd = open(devloop, O_RDONLY);
	if (ioctl(lfd, LOOP_SET_FD, ffd) == -1) {
		fprintf(stderr, "Error: cannot configure the loopback device\n");
		exit(1);
	}
	close(lfd);
	close(ffd);
	
	char dirname[] = "/tmp/firejail-mnt-XXXXXX";
	char *mntdir =  strdup(mkdtemp(dirname));
	if (mntdir == NULL) {
		fprintf(stderr, "Error: cannot create temporary directory\n");
		exit(1);
	}
	mkdir(mntdir, 755);
	chown(mntdir, getuid(), getgid());
	chmod(mntdir, 755);

	char *mode;
	if (asprintf(&mode, "mode=755,uid=%d,gid=%d", getuid(), getgid()) == -1)
		errExit("asprintf");

	if (mount(devloop, mntdir, "iso9660",MS_MGC_VAL|MS_RDONLY,  mode) < 0)
		errExit("mounting appimage");

	if (arg_debug)
		printf("appimage mounted on %s\n", mntdir);
	EUID_USER();
	
	// build new command line
	if (asprintf(&cfg.command_line, "%s/AppRun", mntdir) == -1)
		errExit("asprintf");
	
	free(devloop);
	free(mode);
	
	return mntdir;
}
