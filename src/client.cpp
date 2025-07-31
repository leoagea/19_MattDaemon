/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/31 14:34:38 by lagea             #+#    #+#             */
/*   Updated: 2025/07/31 14:35:13 by lagea            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/MattDaemon.h"


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
				std::string cmd = std::string(buf, r);
				if (cmd.back() != '\n') cmd += '\n';
				size_t unused = write(session_it->pty_fd, cmd.c_str(), cmd.size());(void) unused;
			}
			else{
				g_reporter.Log(INFO,  "User input: " + msg);
			}
		}
	}
	return 1;
}
