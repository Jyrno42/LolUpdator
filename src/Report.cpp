#include "Report.hpp"

void Report::GetReport()
{
	std::time_t timeStamp = std::time(0);

	std::stringstream fileNameStream;
	fileNameStream << reportDirectory << "report_" << timeStamp << ".json";

	rapidjson::Document doc;
	doc.SetObject();
	
	doc["result"].SetBool(result);
	doc["total"].SetInt(total);
	doc["failed"].SetInt(failed);

	doc["errors"].SetArray();
	doc["timers"].SetObject();
	doc["threads"].SetObject();

	for(std::vector<std::string >::iterator i = errors.begin(); i != errors.end(); ++i)
	{
		rapidjson::Value val((*i).c_str());
		doc["errors"].PushBack(val, doc.GetAllocator());
	}

	for(std::map<std::string, std::string >::iterator i = timers.begin(); i != timers.end(); ++i)
	{
		rapidjson::Value key((*i).first.c_str());
		rapidjson::Value val((*i).second.c_str());
		doc["timers"].AddMember(key, val, doc.GetAllocator());
	}

	for(std::map<std::string, ThreadInfo >::iterator i = threads.begin(); i != threads.end(); ++i)
	{
		rapidjson::Value key((*i).first.c_str());

		rapidjson::Value tElement;
		tElement.SetObject();
		
		tElement["started_threads"].SetInt((*i).second.started_threads);
		tElement["finished_threads"].SetInt((*i).second.finished_threads);
		tElement["summoners"].SetInt((*i).second.summoners);

		tElement["time"] = (*i).second.time.c_str();

		doc["threads"].AddMember(key, tElement, doc.GetAllocator());
	}
	
	FILE * file = fopen(fileNameStream.str().c_str(), "w");

	if (file != NULL)
	{
		rapidjson::FileStream f(file);
		rapidjson::PrettyWriter<rapidjson::FileStream> writer(f);
		doc.Accept(writer);

		fclose(file);
	}
	else
	{
		std::cout << "Could not save report to file: " << fileNameStream.str() << std::endl;
	}
}