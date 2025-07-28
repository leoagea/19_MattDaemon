/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 11:24:57 by lagea             #+#    #+#             */
/*   Updated: 2025/07/28 14:51:55 by lagea            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/MattDaemon.h"

int main()
{
	if (getuid() != 0) {
		std::cout << "Please run this program as root." << std::endl;
		return 1;
	}

	
	CreateDaemon();
	
	Tintin_reporter reporter;
	reporter.Log(INFO, "Matt Daemon started successfully.");
	
	DaemonLoop(reporter);

	return 0;
}
