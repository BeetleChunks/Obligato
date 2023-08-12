#pragma once

#include "CriticalSection.h"
#include "Helpers.h"

#define LOG_COLOR_PREFIX    { 255.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f, 1.0f }
#define LOG_COLOR_DEBUG     {   0.0f/255.0f, 144.0f/255.0f, 250.0f/255.0f, 1.0f }
#define LOG_COLOR_TRACE     { 244.0f/255.0f,  10.0f/255.0f, 244.0f/255.0f, 1.0f }
#define LOG_COLOR_ERROR     { 255.0f/255.0f, 102.0f/255.0f, 102.0f/255.0f, 1.0f }
#define LOG_COLOR_EXCEPTION { 255.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f, 1.0f }
#define LOG_COLOR_WARN      { 214.0f/255.0f, 127.0f/255.0f,  40.0f/255.0f, 1.0f }
#define LOG_COLOR_INFO      {  24.0f/255.0f, 252.0f/255.0f, 140.0f/255.0f, 1.0f }

namespace Obligato {
	enum class LogColor : short {
		None,
		Prefix,
		Debug,
		Trace,
		Error,
		Exception,
		Warn,
		Info
	};

	struct LogLine {
		LogColor PrefixColor = LogColor::None;
		LogColor TextColor = LogColor::None;
		std::string Prefix;
		std::string Text;
	};
	
	class Logger {
	private:
		mutable CriticalSection _lock;
		std::vector<std::string> _history;
		std::vector<LogLine> _logs;
		std::map<LogColor, std::vector<float>> _colors;

	public:
		Logger();

		/*
			Display
		*/
		void GetLogSnapshot(std::vector<LogLine>& LogSnapshot);
		void GetHistorySnapshot(std::vector<std::string>& HistorySnapshot);
		void UpdateLogs(const LogLine& line);
		void ClearLogs();
		void LookupColor(const LogColor id, ImVec4& color);

		/*
			Add Logs
		*/
		void Debug(const std::string& message);
		void Trace(const std::string& message);
		void Error(const std::string& message);
		void Exception(const std::string& message);
		void Warn(const std::string& message);
		void Info(const std::string& message);
	};
}