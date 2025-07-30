/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   graphic_client.h                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/30 10:59:49 by lagea             #+#    #+#             */
/*   Updated: 2025/07/30 13:49:30 by lagea            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GRAPHIC_CLIENT_H
# define GRAPHIC_CLIENT_H

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <signal.h>
#include <sstream>

#include "imgui.h"

#define DAEMON_LOCK_FILE_PATH "/var/lock/matt_daemon.lock"
#define DAEMON_LOG_FILE_PATH "/var/log/matt_daemon/matt_daemon.log"

void killDaemon();
void clearLog();
std::string getDaemonStatus();
std::string getLogContent();

#endif