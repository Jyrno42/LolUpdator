#ifndef REPORT_HPP
#define REPORT_HPP

#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include "rapidjson/document.h"

#include "rapidjson/prettywriter.h"
#include "rapidjson/filestream.h"

/* Boost */
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

class ThreadInfo
{
public:
	int started_threads;
	int finished_threads;
	int summoners;
	std::string time;
};

class Report
{
public:
	Report()
	{
		total = 0;
		failed = 0;
		result = false;

		reportDirectory = "";
	}

	std::vector<std::string > errors;
	std::map<std::string, std::string> timers;
	std::map<std::string, ThreadInfo> threads;

	int total;
	int failed;

	bool result;

	void GetReport();

	std::string reportDirectory;
};

#endif
