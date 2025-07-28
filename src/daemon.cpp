/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   daemon.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 14:47:41 by lagea             #+#    #+#             */
/*   Updated: 2025/07/28 21:48:35 by lagea            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/MattDaemon.h"

fd_t listen_fd;
struct sigaction sa;

void TermHandler(int signum)
{
	(void)signum; // Unused parameter

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

void InitSignalHandler()
{
	sa.sa_handler = TermHandler;
	sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;                         // no SA_RESTART: pause() will exit
    sigaction(SIGTERM, &sa, nullptr);
}

void InitSocket()
{
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		std::cerr << "Failed to create socket." << std::endl;
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	server_addr.sin_port = htons(4242);

	if (bind(listen_fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) < 0) {
		std::cerr << "Failed to bind socket." << std::endl;
		close(listen_fd);
		exit(EXIT_FAILURE);
	}

	if (listen(listen_fd, 3) < 0) {
		std::cerr << "Failed to listen on socket." << std::endl;
		close(listen_fd);
		exit(EXIT_FAILURE);
	}

	if (fcntl(listen_fd, F_SETFL, O_NONBLOCK) < 0) {
		std::cerr << "Failed to set socket to non-blocking mode." << std::endl;
		close(listen_fd);
		exit(EXIT_FAILURE);
	}

	reporter.Log(INFO, "Socket initialized and listening on port 4242.");
}

void DaemonLoop()
{
	InitSignalHandler();
	InitSocket();
	
	while (true) {
		
		// Here you can implement the main functionality of your daemon
		sleep(5); // Simulate work by sleeping for 5 seconds
	}
}