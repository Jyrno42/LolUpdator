#include "Report.hpp"

void Report::GetReport()
{
	std::time_t timeStamp = std::time(0);

	std::stringstream fileNameStream;
	fileNameStream << reportDirectory << "report_" << timeStamp << ".json";

	boost::property_tree::ptree 
		root, 
		errorElement, 
		timerElement, 
		threadElement;

	root.put<bool>("result", result);
	
	root.put<int>("total", total);
	root.put<int>("failed", failed);
	root.put<int>("errors", errors.size());

	for(std::vector<std::string >::iterator i = errors.begin(); i != errors.end(); ++i)
	{
		errorElement.put<std::string>("", *i);
	}
	root.put_child("errors", errorElement);

	for(std::map<std::string, std::string >::iterator i = timers.begin(); i != timers.end(); ++i)
	{
		timerElement.put<std::string>((*i).first, (*i).second);
	}
	root.put_child("timers", timerElement);

	for(std::map<std::string, ThreadInfo >::iterator i = threads.begin(); i != threads.end(); ++i)
	{
		boost::property_tree::ptree tElement;
		tElement.put<int>("started_threads", (*i).second.started_threads);
		tElement.put<int>("finished_threads", (*i).second.finished_threads);
		tElement.put<int>("summoners", (*i).second.summoners);
		tElement.put<std::string>("time", (*i).second.time);
		threadElement.put_child((*i).first, tElement);
	}
	root.put_child("threads", threadElement);

	boost::property_tree::write_json(fileNameStream.str(), root);
}