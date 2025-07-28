/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 11:24:57 by lagea             #+#    #+#             */
/*   Updated: 2025/07/28 21:55:30 by lagea            ###   ########.fr       */
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
		return EXIT_FAILURE;
	}

	CreateDaemon();

	CreateLockFile(lockFilePath);
	
	DaemonLoop();

	return 0;
}
