/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MattDaemon.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 11:23:10 by lagea             #+#    #+#             */
/*   Updated: 2025/07/28 22:44:22 by lagea            ###   ########.fr       */
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
# Global Variables
#############################################################################*/

typedef int fd_t;

/*#############################################################################
# Global Variables
#############################################################################*/

extern bool g_stop;
extern fd_t g_listen_fd;
extern struct sigaction g_sa;
extern Tintin_reporter g_reporter;
extern std::array<int, 3> g_client_fds;

/*#############################################################################
# Daemon.cpp
#############################################################################*/

void CreateDaemon();
void CreateLockFile(fs::path &);
void InitSignalHandler();
void InitSocket();
void DaemonLoop();
void ExitHandler();
int AcceptClient();
int HandleClient(int , fd_set *);

#endif