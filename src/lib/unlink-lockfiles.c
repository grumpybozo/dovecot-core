/*
 unlink-lockfiles.c : Utility function for easier deletion of lock files.

    Copyright (c) 2002 Timo Sirainen

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to
    the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "lib.h"
#include "unlink-lockfiles.h"

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

int unlink_lockfiles(const char *dir, const char *pidprefix,
		     const char *otherprefix, time_t other_min_time)
{
	DIR *dirp;
	struct dirent *d;
	struct stat st;
	char path[PATH_MAX];
	unsigned int pidlen, otherlen;
	int ret = 0;

	/* check for any invalid access files */
	dirp = opendir(dir);
	if (dirp == NULL)
		return -1;

	pidlen = pidprefix == NULL ? 0 : strlen(pidprefix);
	otherlen = otherprefix == NULL ? 0 : strlen(otherprefix);

	while ((d = readdir(dirp)) != NULL) {
		const char *fname = d->d_name;

		if (pidprefix != NULL &&
		    strncmp(fname, pidprefix, pidlen) == 0 &&
		    is_numeric(fname+pidlen, '\0')) {
			/* found a lock file from our host - see if the PID
			   is valid (meaning it exists, and the it's with
			   the same UID as us) */
			if (kill(atol(fname+pidlen), 0) == 0 || errno != ESRCH)
				continue; /* valid */

			if (str_path(path, sizeof(path), dir, fname) == 0) {
				if (unlink(path) < 0 && errno != ENOENT) {
					i_error("unlink(%s) failed: %m", path);
					ret = 0;
				}
			}
		} else if (otherprefix != NULL &&
			   strncmp(fname, otherprefix, otherlen) == 0) {
			if (str_path(path, sizeof(path), dir, fname) == 0 &&
			    stat(path, &st) == 0 &&
			    st.st_mtime < other_min_time &&
			    st.st_ctime < other_min_time)
				if (unlink(path) < 0 && errno != ENOENT) {
					i_error("unlink(%s) failed: %m", path);
					ret = 0;
				}
		}
	}

	if (closedir(dirp) < 0)
		i_error("closedir(%s) failed: %m", dir);

	return ret;
}
