#ifndef NVAPIFANSSVC_LOGGER_H_
#define NVAPIFANSSVC_LOGGER_H_
#include <fstream>
#include <ctime>

class Logger {
private:
	bool opened=false;
	std::string	filename;
	std::ofstream stream;
	void tryOpen() {
		if (!opened) {
			stream.open(filename, std::ofstream::out | std::ofstream::app);
			if (stream.fail()) {
				throw std::iostream::failure("Cannot open file for logging: " + filename);
			}
			opened = true;
		}
	}
	std::string timestamp() const {
		struct tm timeinfo;
		time_t t = std::time(nullptr);
		localtime_s(&timeinfo, &t);

		char mbstr[32];
		std::strftime(mbstr, sizeof(mbstr), "[%Y-%m-%dT%H:%M:%S%z]", &timeinfo);
		return std::string(mbstr);
	}

public:
	Logger(std::string fileName) {
		filename = fileName;
	};
	~Logger() {
		if (opened)
			stream.close();
	};
	void Flush() {
		if (opened)
			stream.flush();
	};
	void Error(std::string message) {
		tryOpen();
		stream << timestamp() + " ERROR: " + message << std::endl;
	};
	void Info(std::string message) {
		tryOpen();
		stream << timestamp() + " INFO: " + message << std::endl;
	};
	void Debug(std::string message) {
		tryOpen();
		stream << timestamp() + " Debug: " + message << std::endl;
	};
};

#endif