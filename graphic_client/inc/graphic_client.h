/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   graphic_client.h                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/30 10:59:49 by lagea             #+#    #+#             */
/*   Updated: 2025/07/30 16:44:43 by lagea            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GRAPHIC_CLIENT_H
# define GRAPHIC_CLIENT_H

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <signal.h>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "imgui.h"

#define SUCCESS 0
#define FAILURE -1
#define DAEMON_LOCK_FILE_PATH "/var/lock/matt_daemon.lock"
#define DAEMON_LOG_FILE_PATH "/var/log/matt_daemon/matt_daemon.log"
#define DAEMON_EXEC_PATH "/home/ubuntu/Desktop/mattdaemon/MattDaemon"

int killDaemon();
void clearLog();
pid_t getDaemonPid();
std::string getDaemonStatus();
std::string getLogContent();
bool connectToDaemon();
void disconnectFromDaemon();
bool sendMessage(const std::string& , bool &);
std::string getConnectionStatus();
bool isConnected();
bool isDaemonRunning();

#endif