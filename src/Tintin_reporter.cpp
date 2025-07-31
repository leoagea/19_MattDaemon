/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Tintin_reporter.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lagea < lagea@student.s19.be >             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 12:11:13 by lagea             #+#    #+#             */
/*   Updated: 2025/07/30 17:24:32 by lagea            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Tintin_reporter.h"

Tintin_reporter::Tintin_reporter() : _path(fs::path(LOG_DIR) / LOG_FILE)
{
	CreateLogFile();
	OpenLogFile();
}

Tintin_reporter::~Tintin_reporter()
{
	if (_logfile.is_open())
		_logfile.close();
}

Tintin_reporter::Tintin_reporter(const Tintin_reporter &other) : _path(other._path)
{
	CreateLogFile();
	OpenLogFile();
}

Tintin_reporter& Tintin_reporter::operator=(const Tintin_reporter &other)
{
	if (this != &other) {
		_path = other._path;
		if (_logfile.is_open())
			_logfile.close();
		CreateLogFile();
		OpenLogFile();
	}
	return *this;
}

void Tintin_reporter::CreateLogFile()
{
	if (!fs::exists(_path.parent_path()))
		fs::create_directories(_path.parent_path());
}

void Tintin_reporter::OpenLogFile()
{
	if (!_logfile.is_open()) {
		_logfile.open(_path, std::ios::out | std::ios::app);
		if (!_logfile.is_open()) {
			std::cerr << "Error opening log file: " << _path << std::endl;
			if (getuid() != 0)
				std::cout << "Please run this program as root." << std::endl;
			exit(EXIT_FAILURE);
		}
	}
}

void Tintin_reporter::Log(LogLevel level,const std::string &message)
{
	if (!_logfile.is_open()) {
		std::cerr << "Log file is not open." << std::endl;
		return;
	}

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
		case ERROR: return "[ERROR]";
		default: return "[UNKNOWN]";
	}
}