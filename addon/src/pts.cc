#include "pts.h"
using namespace std;

void pts_fatal(const char* msg) {
    memset(PtsErrMsg, 0, PTS_ERR_MSG_SIZE);
    snprintf(PtsErrMsg, PTS_ERR_MSG_SIZE, "[%s] %s", msg, strerror(errno));
}

void err_exit(const char* msg) {
    pts_fatal(msg);
    fprintf(stderr, "%s\n", PtsErrMsg);
    exit(EXIT_FAILURE);
}

int fd_add_cloexec(int fd) {
    int flags;
    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        pts_fatal("fcntl");
        return -1;
    };
    flags |= O_CLOEXEC;
    return fcntl(fd, F_SETFL, flags);
}

int create_pts(int* child_pid_ptr) {
    int master_tty_fd, slave_tty_fd, child_pid;
    char slave_tty_name[FILENAME_MAX], *str, *shell;
    struct termios slave_tio;
    struct sigaction sia;

    if (child_pid_ptr == NULL) {
        child_pid_ptr = &child_pid;
    }

    if ((master_tty_fd = posix_openpt(O_RDWR | O_NOCTTY)) == -1) {
        pts_fatal("posix_openpt");
        return -1;
    }
    if (fd_add_cloexec(master_tty_fd) == -1) {
        pts_fatal("fd_add_cloexec");
        return -1;
    };
    if ((str = ptsname(master_tty_fd)) == NULL) {
        pts_fatal("ptsname");
        return -1;
    }
    strcpy(slave_tty_name, str);
    if (unlockpt(master_tty_fd) == -1) {
        pts_fatal("unlockpt");
        return -1;
    }
    if ((shell = getenv("SHELL")) == NULL)
        shell = (char*)"/bin/sh";
    if (tcgetattr(master_tty_fd, &slave_tio) == -1) {
        pts_fatal("tcgetattr");
        return -1;
    }

    memset(&sia, 0, sizeof(struct sigaction));
    sigemptyset(&sia.sa_mask);
    sia.sa_flags = SA_RESETHAND | SA_NOCLDWAIT;
    sia.sa_handler = SIG_DFL;
    if (sigaction(SIGCHLD, &sia, NULL) == -1) {
        pts_fatal("sigaction");
        return -1;
    }

    (*child_pid_ptr) = fork();
    if ((*child_pid_ptr) == -1) {
        pts_fatal("fork");
        return -1;
    }
    // child process
    if ((*child_pid_ptr) == 0) {
        if (setsid() == -1)
            err_exit("setsid");
        if (close(master_tty_fd) == -1)
            err_exit("close");
        if ((slave_tty_fd = open(slave_tty_name, O_RDWR)) == -1)
            err_exit("open");

        if (tcsetattr(slave_tty_fd, TCSANOW, &slave_tio) == -1)
            err_exit("tcsetattr");
        if (dup2(slave_tty_fd, STDIN_FILENO) == -1)
            err_exit("dup2");
        if (dup2(slave_tty_fd, STDOUT_FILENO) == -1)
            err_exit("dup2");
        if (dup2(slave_tty_fd, STDERR_FILENO) == -1)
            err_exit("dup2");
        execlp(shell, shell, (char*)NULL);
        err_exit("execlp");
    }
    return master_tty_fd;
}

int kill_pts_child(int child_pid) {
    kill(child_pid, SIGHUP);
    return 0;
}

int resize_pts(int pts_fd, unsigned short rows, unsigned short cols) {
    struct winsize wsize;
    if (ioctl(pts_fd, TIOCGWINSZ, &wsize) == -1) {
        pts_fatal("ioctl");
        return -1;
    };
    wsize.ws_row = rows;
    wsize.ws_col = cols;
    if (ioctl(pts_fd, TIOCSWINSZ, &wsize) == -1) {
        pts_fatal("ioctl");
        return -1;
    }
    return 0;
}