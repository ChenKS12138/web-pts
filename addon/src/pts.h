#ifndef _NODEJS_PTS
#define _NODEJS_PTS

#define _XOPEN_SOURCE 600
#define _GUN_SOURCE
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#ifndef PTS_ERR_MSG_SIZE
#define PTS_ERR_MSG_SIZE 256
#endif

static char PtsErrMsg[PTS_ERR_MSG_SIZE];

int create_pts(int* child_pid);
int kill_pts_child(int child_pid);
int resize_pts(int pts_fd, unsigned short rows, unsigned short cols);

#endif