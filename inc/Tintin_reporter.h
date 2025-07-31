/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Tintin_reporter.h                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 12:01:09 by lagea             #+#    #+#             */
/*   Updated: 2025/07/30 17:25:08 by lagea            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TINTIN_REPORTER_H
# define TINTIN_REPORTER_H

#include <filesystem> // for std::filesystem
#include <fstream> // for std::ofstream
#include <iostream> // for std::cerr

#include <unistd.h> // for getuid()

#define LOG_DIR "/var/log/matt_daemon"
#define LOG_FILE "matt_daemon.log"

namespace fs = std::filesystem;
namespace chrono = std::chrono;

enum LogLevel {
	INFO,
	WARNING,
	ERROR
};

class Tintin_reporter
{
	private:
		fs::path _path;
		std::ofstream _logfile;
		
		void CreateLogFile();
		void OpenLogFile();
		static const char* LogLevelToString(LogLevel level) noexcept;

	public:
		Tintin_reporter();
		~Tintin_reporter();
		Tintin_reporter(const Tintin_reporter &);
		Tintin_reporter& operator=(const Tintin_reporter &);

		void Log(LogLevel ,const std::string &);
};

#endif 