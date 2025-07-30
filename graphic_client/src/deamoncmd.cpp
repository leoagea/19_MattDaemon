/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   deamoncmd.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/30 10:52:50 by lagea             #+#    #+#             */
/*   Updated: 2025/07/30 14:02:12 by lagea            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/graphic_client.h"

static std::string daemonStatus = "";

static pid_t getDaemonPid()
{
	std::ifstream lockFile(DAEMON_LOCK_FILE_PATH);
	if (!lockFile.is_open()){
		std::cerr << "Could not open lock file: " << DAEMON_LOCK_FILE_PATH << "\n";
		std::cerr << "Daemon not running or lock file does not exist.\n";
		return -1;
	}
	
	pid_t pid;
	lockFile >> pid;
	return pid;
}

void killDaemon()
{
	pid_t pid = getDaemonPid();
	if (pid <= 0){
		daemonStatus = "Daemon not running or invalid PID.";
		return;
	}

	if (kill(pid, 0) == 0){
		if (kill(pid, SIGTERM) == 0)
			daemonStatus = "Successfully killed daemon with PID: " + std::to_string(pid);
		else 
			daemonStatus = "Failed to kill daemon with PID: " + std::to_string(pid);
	}
	else 
		daemonStatus = "Failed to send signal to daemon with PID: " + std::to_string(pid) + ".";
}

void clearLog()
{
	std::ofstream logFile(DAEMON_LOG_FILE_PATH, std::ios::trunc);
	if (!logFile.is_open()){
		std::cerr << "Could not open log file for clearing: " << DAEMON_LOG_FILE_PATH << std::endl;
		return;
	}
	logFile.close();
}

std::string getDaemonStatus()
{
	return daemonStatus;
}

std::string getLogContent()
{
	std::ifstream logFile(DAEMON_LOG_FILE_PATH);
	if (!logFile.is_open()){
		return "Could not open log file.";
	}

	std::string content((std::istreambuf_iterator<char>(logFile)), std::istreambuf_iterator<char>());
	logFile.close();
	return content;
}
