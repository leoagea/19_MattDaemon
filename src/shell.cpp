/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shell.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/31 14:33:00 by lagea             #+#    #+#             */
/*   Updated: 2025/07/31 14:33:35 by lagea            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/MattDaemon.h"

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
