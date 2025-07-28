/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   daemon.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 14:47:41 by lagea             #+#    #+#             */
/*   Updated: 2025/07/28 22:57:34 by lagea            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/MattDaemon.h"

bool g_stop = false;
fd_t g_listen_fd = -1;
struct sigaction g_sa;
std::array<int, 3> g_client_fds = {-1, -1, -1};

void TermHandler(int signum)
{
	(void)signum; // Unused parameter

	g_reporter.Log(INFO, "Received SIGKILL signal, shutting down.");

	remove(LOCK_PATH);
	close(g_listen_fd);

	for (int &fd : g_client_fds) {
		if (fd != -1) 
			close(fd);
	}

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
	g_sa.sa_handler = TermHandler;
	sigemptyset(&g_sa.sa_mask);
	g_sa.sa_flags = 0;
	sigaction(SIGTERM, &g_sa, nullptr);
}

void InitSocket()
{
	g_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (g_listen_fd < 0) {
		std::cerr << "Failed to create socket." << std::endl;
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	server_addr.sin_port = htons(4242);

	if (bind(g_listen_fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) < 0) {
		std::cerr << "Failed to bind socket." << std::endl;
		close(g_listen_fd);
		exit(EXIT_FAILURE);
	}

	if (listen(g_listen_fd, 3) < 0) {
		std::cerr << "Failed to listen on socket." << std::endl;
		close(g_listen_fd);
		exit(EXIT_FAILURE);
	}

	if (fcntl(g_listen_fd, F_SETFL, O_NONBLOCK) < 0) {
		std::cerr << "Failed to set socket to non-blocking mode." << std::endl;
		close(g_listen_fd);
		exit(EXIT_FAILURE);
	}

	g_reporter.Log(INFO, "Socket initialized and listening on port 4242.");
}

int AcceptNewClient()
{
	int cli = accept(g_listen_fd, nullptr, nullptr);
	if (cli == -1) return 0;

	auto free_slot = std::find(g_client_fds.begin(), g_client_fds.end(), -1);
	if (free_slot == g_client_fds.end()) {
		close(cli);  // >3 → reject
	} else {
		*free_slot = cli;
		fcntl(cli, F_SETFL, O_NONBLOCK);
	}
	return 1;
}

int HandleClient(int client_fd, fd_set *rset)
{
	if (FD_ISSET(client_fd, rset)) {
		char buf[1024];
		ssize_t r = recv(client_fd, buf, sizeof(buf) - 1, 0);
		if (r <= 0) {
			close(client_fd);
			client_fd = -1; 
			return 0; 
		}

		buf[r] = '\0';
		std::string msg(buf);
		msg.erase(msg.find_last_not_of("\r\n") + 1);

		if (msg == "quit") {
			g_reporter.Log(INFO, "Request quit.");
			g_stop = true;
		} else {
			g_reporter.Log(INFO,  "User input: " + msg);
		}
	}
	return 1;
}

void DaemonLoop()
{
	InitSignalHandler();
	InitSocket();
	
	while (!g_stop) {
		fd_set rset;  
		FD_ZERO(&rset);
		FD_SET(g_listen_fd, &rset);
		int maxfd = g_listen_fd;

		for (int fd : g_client_fds){
			if (fd != -1) {
				FD_SET(fd, &rset);
				maxfd = std::max(maxfd, fd);
			}
		}

		int n = select(maxfd+1, &rset, nullptr, nullptr, nullptr);
		if (n < 0 && errno == EINTR) 
			continue;   // interrupted by signal
		
		if (FD_ISSET(g_listen_fd, &rset))
			if (!AcceptNewClient()) 
				continue;

		for (int &fd : g_client_fds) {
			if (fd != -1) 
				if (!HandleClient(fd, &rset)) 
					continue;
		}
	}

	ExitHandler();
}

void ExitHandler()
{
	g_reporter.Log(INFO, "Quitting daemon...");
	remove(LOCK_PATH);
	close(g_listen_fd);

	for (int &fd : g_client_fds) {
		if (fd != -1) 
			close(fd);
	}

	exit(EXIT_SUCCESS);
}