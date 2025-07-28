/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   daemon.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 14:47:41 by lagea             #+#    #+#             */
/*   Updated: 2025/07/28 17:00:13 by lagea            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/MattDaemon.h"

void TermHandler(int signum)
{
	(void)signum; // Unused parameter

	Tintin_reporter reporter;
	reporter.Log(INFO, "Received SIGKILL signal, shutting down.");

	remove(LOCK_PATH);

	exit(EXIT_SUCCESS);
}

void CreateDaemon()
{
	pid_t pid = fork();
	if (pid < 0) {
		std::cerr << "Fork failed." << std::endl;
		exit(EXIT_FAILURE);
	} else if (pid > 0) {
		std::cout << "First fork successful, parent process exiting." << std::endl;
		exit(EXIT_SUCCESS); // Parent process exits
	}

	if (setsid() < 0) {
		std::cerr << "Failed to create a new session." << std::endl;
		exit(EXIT_FAILURE);
	}

	signal(SIGCHLD, SIG_IGN); // Ignore SIGCHLD to prevent zombie processes
	signal(SIGHUP, SIG_IGN); // Ignore SIGHUP to prevent termination on terminal close
	
	pid = fork();
	if (pid < 0) {
		std::cerr << "Fork failed." << std::endl;
		exit(EXIT_FAILURE);
	} else if (pid > 0) {
		std::cout << "Second fork successful, parent process exiting." << std::endl;
		exit(EXIT_SUCCESS); // Parent process exits
	}

	umask(0); // Change file mode mask to 0
	if (chdir("/") == -1) { // Change working directory to root{
		std::cerr << "Failed to change working directory to root." << std::endl;
		exit(EXIT_FAILURE);
	}

		// 1. Close the inherited descriptors
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	// 2. Re-open them all on /dev/null
	int fd = open("/dev/null", O_RDWR);   // returns lowest unused number → 0
	dup2(fd, STDOUT_FILENO);              // 1
	dup2(fd, STDERR_FILENO);              // 2
	if (fd > 2) close(fd);                // keep things tidy
}

void CreateLockFile(fs::path &lockpath)
{
	if (!fs::exists(lockpath.parent_path()))
		fs::create_directory(lockpath.parent_path());

	std::ofstream lockfile;
	if (!fs::exists(lockpath)){
		lockfile.open(lockpath);
		if (!lockfile.is_open()){
			std::cerr << "Failed to create lock file." << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	int fd = open(lockpath.c_str(), O_RDWR|O_CREAT, 0666);
	if (flock(fd, LOCK_EX | LOCK_NB) == -1){
		std::cerr << "Failed to locked file." << std::endl;
		exit(EXIT_FAILURE);
	}

	close(fd);
}

void DaemonLoop()
{
	struct sigaction sa = {};
	sa.sa_handler = TermHandler;
	sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;                         // no SA_RESTART: pause() will exit
    sigaction(SIGTERM, &sa, nullptr);

	while (true) {
		
		// Here you can implement the main functionality of your daemon
		sleep(5); // Simulate work by sleeping for 5 seconds
	}
}