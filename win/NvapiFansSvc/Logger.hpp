#ifndef NVAPIFANSSVC_LOGGER_H_
#define NVAPIFANSSVC_LOGGER_H_
#include <fstream>
#include <ctime>

class Logger {
private:
	bool opened;
	std::string	filename;
	std::ofstream stream;
	void tryOpen() {
		if (!opened) {
			stream.open(filename);
			if (stream.fail()) {
				throw std::iostream::failure("Cannot open file for logging: " + filename);
			}
			opened = true;
		}
	}
	std::string timestamp() {
		time_t result = time(NULL);
		char str[26];
		ctime_s(str, sizeof str, &result);
		printf("%s", str);
		return "[" + std::string(str) + "]";
	}

public:
	Logger(std::string fileName) {
		opened = false;
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
		stream << "ERROR: " + timestamp() + message << std::endl;
	};
	void Info(std::string message) {
		tryOpen();
		stream << "INFO: " + timestamp() + message << std::endl;
	};
	void Debug(std::string message) {
		tryOpen();
		stream << "Debug: " + timestamp() + message << std::endl;
	};
};

#endif