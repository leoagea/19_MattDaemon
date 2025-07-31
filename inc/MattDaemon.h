/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MattDaemon.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 11:23:10 by lagea             #+#    #+#             */
/*   Updated: 2025/07/31 14:28:54 by lagea            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MATTDAEMON_H
# define MATTDAEMON_H

/*#############################################################################
# Includes
#############################################################################*/

#include <iostream> // cout, cerr
#include <unistd.h> // getuid()
#include <signal.h> // signal handling
#include <fcntl.h> // open, dup2
#include <filesystem> // filesystem
#include <array> // array
#include <algorithm> // max, find
#include <errno.h> // errno
#include <sstream> // stringstream
#include <pty.h> // openpty
#include <termio.h> // termios
#include <sys/wait.h> // waitpid
#include <sys/select.h> // select
#include <sys/socket.h> // socket, bind, listen
#include <sys/stat.h> // umask
#include <sys/file.h> // flock
#include <netinet/in.h> // sockaddr_in, INADDR_LOOPBACK

#include "Tintin_reporter.h" // Tintin_reporter class

/*#############################################################################
# Defines
#############################################################################*/

#define LOCK_PATH "/var/lock/matt_daemon.lock"

/*#############################################################################
# Typedef Variables
#############################################################################*/

typedef int fd_t;

/*#############################################################################
# Struct 
#############################################################################*/

typedef struct {
	fd_t client_fd;
	fd_t pty_fd;
	pid_t shell_pid;
}		t_shell_session;

/*#############################################################################
# Global Variables
#############################################################################*/

extern bool g_stop;
extern fd_t g_listen_fd;
extern struct sigaction g_sa;
extern Tintin_reporter g_reporter;
extern std::array<int, 3> g_client_fds;
extern std::array<t_shell_session, 3> g_shell_sessions;

/*#############################################################################
# Daemon.cpp
#############################################################################*/

void CreateDaemon();
void CreateLockFile(fs::path &);
void InitSignalHandler();
void InitSocket();
void DaemonLoop();
void ExitHandler();
int AcceptNewClient();
int HandleClient(int , fd_set *);
void HandleShellCommand(fd_t);
void CloseShellSession(t_shell_session &);
void HandleShellIO(t_shell_session &, fd_set *);

#endif