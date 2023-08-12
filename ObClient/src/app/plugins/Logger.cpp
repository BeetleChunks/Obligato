#include "pch.h"
#include "Logger.h"
#include "AutoCriticalSection.h"

/*
	CONSTRUCTORS
*/
Obligato::Logger::Logger() {
	_colors = {
			{ Obligato::LogColor::Prefix,    LOG_COLOR_PREFIX    },
			{ Obligato::LogColor::Debug,     LOG_COLOR_DEBUG     },
			{ Obligato::LogColor::Trace,     LOG_COLOR_TRACE     },
			{ Obligato::LogColor::Error,     LOG_COLOR_ERROR     },
			{ Obligato::LogColor::Exception, LOG_COLOR_EXCEPTION },
			{ Obligato::LogColor::Warn,      LOG_COLOR_WARN      },
			{ Obligato::LogColor::Info,      LOG_COLOR_INFO      }
	};
}


/*
	PUBLIC METHODS - Display
*/
void Obligato::Logger::GetLogSnapshot(std::vector<Obligato::LogLine>& LogSnapshot) {
	AutoCriticalSection locker(_lock);
	LogSnapshot = _logs;
}

void Obligato::Logger::GetHistorySnapshot(std::vector<std::string>& HistorySnapshot) {
	AutoCriticalSection locker(_lock);
	HistorySnapshot = _history;
}

void Obligato::Logger::UpdateLogs(const Obligato::LogLine& line) {
	AutoCriticalSection locker(_lock);
	_logs.push_back(line);
}

void Obligato::Logger::ClearLogs() {
	AutoCriticalSection locker(_lock);
	_logs.clear();
}

void Obligato::Logger::LookupColor(const Obligato::LogColor id, ImVec4& color) {
	AutoCriticalSection locker(_lock);

	color.x = _colors[id][0];
	color.y = _colors[id][1];
	color.z = _colors[id][2];
	color.w = _colors[id][3];
}

/*
	PUBLIC METHODS - Add logs
*/
void Obligato::Logger::Debug(const std::string& message) {
	AutoCriticalSection locker(_lock);

	Obligato::LogLine Output;
	Output.PrefixColor = Obligato::LogColor::Prefix;
	Output.TextColor = Obligato::LogColor::Debug;
	Output.Prefix = "[Debug]";
	Output.Text = message.c_str();

	UpdateLogs(Output);
}

void Obligato::Logger::Trace(const std::string& message) {
	AutoCriticalSection locker(_lock);

	Obligato::LogLine Output;
	Output.PrefixColor = Obligato::LogColor::Prefix;
	Output.TextColor = Obligato::LogColor::Trace;
	Output.Prefix = "[Trace]";
	Output.Text = message.c_str();

	UpdateLogs(Output);
}

void Obligato::Logger::Error(const std::string& message) {
	AutoCriticalSection locker(_lock);

	Obligato::LogLine Output;
	Output.PrefixColor = Obligato::LogColor::Prefix;
	Output.TextColor = Obligato::LogColor::Error;
	Output.Prefix = "[Error]";
	Output.Text = message.c_str();

	UpdateLogs(Output);
}

void Obligato::Logger::Exception(const std::string& message) {
	AutoCriticalSection locker(_lock);

	Obligato::LogLine Output;
	Output.PrefixColor = Obligato::LogColor::Prefix;
	Output.TextColor = Obligato::LogColor::Exception;
	Output.Prefix = "[Exception]";
	Output.Text = message.c_str();

	UpdateLogs(Output);
}

void Obligato::Logger::Warn(const std::string& message) {
	AutoCriticalSection locker(_lock);

	Obligato::LogLine Output;
	Output.PrefixColor = Obligato::LogColor::Prefix;
	Output.TextColor = Obligato::LogColor::Warn;
	Output.Prefix = "[Warn]";
	Output.Text = message.c_str();

	UpdateLogs(Output);
}

void Obligato::Logger::Info(const std::string& message) {
	AutoCriticalSection locker(_lock);

	Obligato::LogLine Output;
	Output.PrefixColor = Obligato::LogColor::Prefix;
	Output.TextColor = Obligato::LogColor::Info;
	Output.Prefix = "[Info]";
	Output.Text = message.c_str();

	UpdateLogs(Output);
}