/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   daemon.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 14:47:41 by lagea             #+#    #+#             */
/*   Updated: 2025/07/31 14:32:23 by lagea            ###   ########.fr       */
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
			auto session_it = std::find_if(g_shell_sessions.begin(), g_shell_sessions.end(),
				[client_fd](const t_shell_session& s) { return s.client_fd == client_fd; });
			if (session_it != g_shell_sessions.end())
				CloseShellSession(*session_it);
			
			auto client_it = std::find(g_client_fds.begin(), g_client_fds.end(), client_fd);
			if (client_it != g_client_fds.end())
				*client_it = -1;
			
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
		} else if (msg == "shell") {
			HandleShellCommand(client_fd);
		} else {
			auto session_it = std::find_if(g_shell_sessions.begin(), g_shell_sessions.end(),
				[client_fd](const t_shell_session& s) { return s.client_fd == client_fd; });
			
			if (session_it != g_shell_sessions.end()){
				std::cout << "DEBUG: Sending to shell: " << msg << std::endl;  // Add this
				std::string cmd = std::string(buf, r);
				if (cmd.back() != '\n') cmd += '\n';
				size_t unused = write(session_it->pty_fd, cmd.c_str(), cmd.size());(void) unused;
			}
			else{
				std::cout << "DEBUG: No shell session found, logging as message" << std::endl;  // Add this
				g_reporter.Log(INFO,  "User input: " + msg);
			}
		}
	}
	return 1;
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

void HandleShellCommand(fd_t client_fd)
{
	int master_fd, slave_fd;
	
 	auto free_slot = std::find_if(g_shell_sessions.begin(), g_shell_sessions.end(), [](const t_shell_session& session) {
			return session.client_fd == -1 && session.pty_fd == -1 && session.shell_pid == -1;
	});
	if (free_slot == g_shell_sessions.end())
		return ; // No free shell session available

	if (openpty(&master_fd, &slave_fd, nullptr, nullptr, nullptr) < 0) {
		g_reporter.Log(ERROR, "Failed to open pseudo-terminal.");
		return;
	}

	// Configure terminal attributes to disable echo
	struct termios tty;
	if (tcgetattr(slave_fd, &tty) == 0) {
		tty.c_lflag &= ~ECHO;      // Disable echo
		tty.c_lflag &= ~ECHOE;     // Disable erase echo
		tty.c_lflag &= ~ECHOK;     // Disable kill echo
		tty.c_lflag &= ~ECHONL;    // Disable newline echo
		tcsetattr(slave_fd, TCSANOW, &tty);
	}
	
	pid_t shell_pid = fork();
	if (shell_pid < 0) {
		g_reporter.Log(ERROR, "Failed to fork shell process.");
		close(master_fd);
		close(slave_fd);
		return;
	} else if (shell_pid == 0) {
		close(master_fd);
		
		setsid();
		
		dup2(slave_fd, STDIN_FILENO);
		dup2(slave_fd, STDOUT_FILENO);
		dup2(slave_fd, STDERR_FILENO);
		close(slave_fd);

		execlp("bash", "bash", nullptr);
		exit(EXIT_FAILURE);
	}

	close(slave_fd);
	*free_slot = {client_fd, master_fd, shell_pid};
	fcntl(master_fd, F_SETFL, O_NONBLOCK);
	g_reporter.Log(INFO, "Shell session created with PID: " + std::to_string(shell_pid));
}

void CloseShellSession(t_shell_session &session)
{
	if (session.pty_fd != -1)
		close(session.pty_fd);
	if (session.shell_pid != -1)
		kill(session.shell_pid, SIGHUP);

	waitpid(session.shell_pid, nullptr, 0);
	
	const std::string msg = "Shell session with PID " + std::to_string(session.shell_pid) + " closed.";
	g_reporter.Log(INFO, msg);
	session = {-1, -1, -1};
}

void HandleShellIO(t_shell_session &session, fd_set *rset)
{
	char buffer[1024];
	
	if (FD_ISSET(session.pty_fd, rset)) {
		ssize_t bytes = read(session.pty_fd, buffer, sizeof(buffer) - 1);
		if (bytes > 0) {
			buffer[bytes] = '\0';
			send(session.client_fd, buffer, bytes, 0);
		} else if (bytes == 0 || (bytes < 0 && errno != EAGAIN)) {
			// Shell terminated
			CloseShellSession(session);
		}
	}
}
