/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   deamoncmd.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/30 10:52:50 by lagea             #+#    #+#             */
/*   Updated: 2025/07/30 16:45:06 by lagea            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/graphic_client.h"

static int client_socket = -1;
static bool is_connected = false;
static std::string daemonStatus = "";
static std::string connection_status = "Disconnected";

pid_t getDaemonPid()
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

int killDaemon()
{
	pid_t pid = getDaemonPid();
	if (pid <= 0){
		daemonStatus = "Daemon not running or invalid PID.";
		return FAILURE;
	}

	if (kill(pid, 0) == 0){
		if (isConnected() && client_socket != -1) {
			disconnectFromDaemon();
		}
		if (kill(pid, SIGTERM) == 0){
			daemonStatus = "Successfully killed daemon with PID: " + std::to_string(pid);
			return SUCCESS;
		}
		else 
			daemonStatus = "Failed to kill daemon with PID: " + std::to_string(pid);
	}
	else 
		daemonStatus = "Failed to send signal to daemon with PID: " + std::to_string(pid) + ".";

	return FAILURE;
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

bool connectToDaemon() 
{
	if (is_connected) return true;
	
	client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket < 0) {
		connection_status = "Failed to create socket";
		return false;
	}
	
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(4242);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		connection_status = "Failed to connect to daemon on port 4242";
		close(client_socket);
		client_socket = -1;
		return false;
	}
	
	is_connected = true;
	connection_status = "Connected to daemon on port 4242";
	return true;
}

void disconnectFromDaemon()
{
	if (client_socket != -1) {
		close(client_socket);
		client_socket = -1;
	}
	is_connected = false;
	connection_status = "Disconnected";
}

bool sendMessage(const std::string& message, bool &daemon_running)
{
	if (!is_connected || client_socket == -1) {
		connection_status = "Not connected to daemon";
		return false;
	}
	
	std::string msg = message + "\n";
	ssize_t sent = send(client_socket, msg.c_str(), msg.length(), 0);
	if (sent < 0) {
		connection_status = "Failed to send message";
		return false;
	}
	
	if (msg == "quit\n"){
		disconnectFromDaemon();
		daemon_running = false;
	}

	return true;
}

std::string getConnectionStatus()
{
	return connection_status;
}

bool isConnected()
{
	return is_connected;
}

bool isDaemonRunning()
{
	pid_t pid = getDaemonPid();
	if (pid <= 0)
		return false;
	
	if (kill(pid, 0) == 0)
		return true;
	return false;
}