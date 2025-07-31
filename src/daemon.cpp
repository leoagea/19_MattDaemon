/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   daemon.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 14:47:41 by lagea             #+#    #+#             */
/*   Updated: 2025/07/31 14:35:04 by lagea            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/MattDaemon.h"

bool g_stop = false;
fd_t g_listen_fd = -1;
struct sigaction g_sa;
std::array<int, 3> g_client_fds = {-1, -1, -1};
std::array<t_shell_session, 3> g_shell_sessions = {
	t_shell_session{-1, -1, -1},
	t_shell_session{-1, -1, -1},
	t_shell_session{-1, -1, -1}
};

void TermHandler(int signum)
{
	(void)signum;

	g_reporter.Log(INFO, "Received SIGKILL signal, shutting down.");
	ExitHandler();

	exit(EXIT_SUCCESS);
}

void CreateDaemon()
{
	g_reporter.Log(INFO, "Entering daemon mode...");

	pid_t pid = fork();
	if (pid < 0) {
		g_reporter.Log(ERROR, "Failed to fork the daemon process.");
		exit(EXIT_FAILURE);
	} else if (pid > 0)
		exit(EXIT_SUCCESS); // Parent process exits

	if (setsid() < 0) {
		g_reporter.Log(ERROR, "Failed to create a new session.");
		exit(EXIT_FAILURE);
	}

	signal(SIGCHLD, SIG_IGN); // Ignore SIGCHLD to prevent zombie processes
	signal(SIGHUP, SIG_IGN); // Ignore SIGHUP to prevent termination on terminal close
	
	pid = fork();
	if (pid < 0) {
		g_reporter.Log(ERROR, "Failed to fork the daemon process.");
		exit(EXIT_FAILURE);
	} else if (pid > 0)
		exit(EXIT_SUCCESS); // Parent process exits

	umask(0);
	if (chdir("/") == -1) { 
		g_reporter.Log(ERROR, "Failed to change working directory to root.");
		exit(EXIT_FAILURE);
	}

	// Close the inherited descriptors
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	// Re-open them all on /dev/null
	int fd = open("/dev/null", O_RDWR);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);
	if (fd > 2) 
		close(fd);

	pid_t daemon_pid = getpid();
	std::stringstream ss;
	ss << "Daemon mode activated. PID: ";
	ss << daemon_pid;
	g_reporter.Log(INFO, ss.str());
}

void CreateLockFile(fs::path &lockpath)
{
	if (!fs::exists(lockpath.parent_path()))
		fs::create_directory(lockpath.parent_path());

	std::ofstream lockfile;
	if (!fs::exists(lockpath)){
		lockfile.open(lockpath);
		if (!lockfile.is_open()){
			g_reporter.Log(ERROR, "Failed to create lock file.");
			exit(EXIT_FAILURE);
		}
		lockfile << getpid() << std::endl;
	}

	int fd = open(lockpath.c_str(), O_RDWR|O_CREAT, 0666);
	if (flock(fd, LOCK_EX | LOCK_NB) == -1){
		g_reporter.Log(ERROR, "Failed to lock file.");
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
		g_reporter.Log(ERROR, "Failed to create socket.");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	server_addr.sin_port = htons(4242);

	if (bind(g_listen_fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) < 0) {
		g_reporter.Log(ERROR, "Failed to bind socket.");
		close(g_listen_fd);
		exit(EXIT_FAILURE);
	}

	if (listen(g_listen_fd, 3) < 0) {
		g_reporter.Log(ERROR, "Failed to listen on socket.");
		close(g_listen_fd);
		exit(EXIT_FAILURE);
	}

	if (fcntl(g_listen_fd, F_SETFL, O_NONBLOCK) < 0) {
		g_reporter.Log(ERROR, "Failed to set socket to non-blocking mode.");
		close(g_listen_fd);
		exit(EXIT_FAILURE);
	}

	g_reporter.Log(INFO, "Server created on port 4242.");
}

void DaemonLoop()
{
	InitSignalHandler();
	
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

		for (const auto &session : g_shell_sessions) {
			if (session.pty_fd != -1) {
				FD_SET(session.pty_fd, &rset);
				maxfd = std::max(maxfd, session.pty_fd);
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

		for (auto &session : g_shell_sessions){
			if (session.pty_fd != -1 && session.client_fd != -1)
				HandleShellIO(session, &rset);
		}
	}

	ExitHandler();
}

void ExitHandler()
{
	g_reporter.Log(INFO, "Quitting.");
	remove(LOCK_PATH);
	close(g_listen_fd);

	for (int &fd : g_client_fds) {
		if (fd != -1) 
			close(fd);
	}

	exit(EXIT_SUCCESS);
}
