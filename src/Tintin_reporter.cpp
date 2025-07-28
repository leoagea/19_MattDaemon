/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Tintin_reporter.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 12:11:13 by lagea             #+#    #+#             */
/*   Updated: 2025/07/28 13:27:15 by lagea            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Tintin_reporter.h"

Tintin_reporter::Tintin_reporter() : _path(fs::path(LOG_DIR) / LOG_FILE)
{
	std::cout << "Log file path: " << _path << std::endl;
	CreateLogFile();
	OpenLogFile();
}

Tintin_reporter::~Tintin_reporter()
{
	if (_logfile.is_open())
		_logfile.close();
}

void Tintin_reporter::CreateLogFile()
{
	if (!fs::exists(_path.parent_path())){
		fs::create_directories(_path.parent_path());
		std::cout << "Log file created successfully: " << _path << std::endl;
	}

}

void Tintin_reporter::OpenLogFile()
{
	if (!_logfile.is_open()) {
		_logfile.open(_path, std::ios::out | std::ios::app);
		if (!_logfile.is_open()) {
			std::cerr << "Error opening log file: " << _path << std::endl;
			exit(EXIT_FAILURE);
		}
		std::cout << "Log opened successfully: " << _path << std::endl;
	}
}

void Tintin_reporter::Log(LogLevel level,const std::string &message)
{
	if (!_logfile.is_open()) {
		std::cerr << "Log file is not open." << std::endl;
		return;
	}

	//Retrieve timestamp with hour,minutes,seconds and format it
	auto now = chrono::system_clock::now();
	auto in_time_t = chrono::system_clock::to_time_t(now);
	std::tm buf;
	localtime_r(&in_time_t, &buf);
	_logfile << std::put_time(&buf, "[%Y/%m/%d-%H:%M:%S]") << " - ";
	
	_logfile << LogLevelToString(level) << " - " << message << std::endl;
}

const char* Tintin_reporter::LogLevelToString(LogLevel level) noexcept
{
	switch (level) {
		case INFO: return "[INFO]";
		case WARNING: return "[WARNING]";
		case ERROR: return "[ERROR]";
		default: return "[UNKNOWN]";
	}
}