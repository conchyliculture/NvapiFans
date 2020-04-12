#ifndef NVAPIFANSSVC_LOGGER_H_
#define NVAPIFANSSVC_LOGGER_H_
#include <fstream>
#include <ctime>

namespace SillyLogger {
	enum LogLevel { DEBUG, INFO, QUIET };

	class Logger {
		private:
			bool opened = false;
			bool level = QUIET;
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
			Logger(std::string fileName, int level) {
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
				if (level > INFO)
					return
				tryOpen();
				stream << timestamp() + " INFO: " + message << std::endl;
			};
			void Debug(std::string message) {
				if (level > DEBUG)
					return
				tryOpen();
				stream << timestamp() + " Debug: " + message << std::endl;
			};
	};
}
#endif