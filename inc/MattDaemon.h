/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MattDaemon.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 11:23:10 by lagea             #+#    #+#             */
/*   Updated: 2025/07/28 14:51:28 by lagea            ###   ########.fr       */
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
#include <sys/stat.h> // for umask

#include "Tintin_reporter.h" // for Tintin_reporter class

/*#############################################################################
# Daemon.cpp
#############################################################################*/

void CreateDaemon();
void DaemonLoop(Tintin_reporter &reporter);

#endif