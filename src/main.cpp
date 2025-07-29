/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 11:24:57 by lagea             #+#    #+#             */
/*   Updated: 2025/07/29 11:17:24 by lagea            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/MattDaemon.h"

Tintin_reporter g_reporter;

int main()
{
	fs::path lockFilePath = fs::path(LOCK_PATH);
	if (getuid() != 0) {
		std::cout << "Please run this program as root." << std::endl;
		return EXIT_FAILURE;
	}

	g_reporter.Log(INFO, "Matt Daemon started.");
	
	if (fs::exists(lockFilePath)) {
		std::cerr << "Can't run the deamon, /var/lock/matt_daemon.lock exists." << std::endl;
		g_reporter.Log(ERROR, "File locked, daemon already running.");
		g_reporter.Log(INFO, "Quitting.");
		return EXIT_FAILURE;
	}

	InitSocket();
	CreateDaemon();
	CreateLockFile(lockFilePath);
	DaemonLoop();

	return 0;
}
