/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MattDaemon.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 11:23:10 by lagea             #+#    #+#             */
/*   Updated: 2025/07/28 17:00:59 by lagea            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MATTDAEMON_H
# define MATTDAEMON_H

/*#############################################################################
# Includes
#############################################################################*/

#include <iostream> // for std::cout, std::cerr
#include <unistd.h> // for getuid()
#include <signal.h> // for signal handling
#include <fcntl.h> // for open, dup2
#include <filesystem> // for std::filesystem
#include <sys/stat.h> // for umask
#include <sys/file.h> // for flock

#include "Tintin_reporter.h" // for Tintin_reporter class

/*#############################################################################
# Defines
#############################################################################*/

#define LOCK_PATH "/var/lock/matt_daemon.lock"

/*#############################################################################
# Daemon.cpp
#############################################################################*/

void CreateDaemon();
void CreateLockFile(fs::path &);
void DaemonLoop();

#endif