#include "Report.hpp"

#include "rapidjson/stringbuffer.h"

void Report::GetReport()
{
	std::time_t timeStamp = std::time(0);

	std::stringstream fileNameStream;
	fileNameStream << reportDirectory << "report_" << timeStamp << ".json";
	
	FILE * file = fopen(fileNameStream.str().c_str(), "w");

	if (file != NULL)
	{
		rapidjson::FileStream f(file);
		
		rapidjson::Document doc;
		doc.Parse<0>("{}");

		rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
			
		rapidjson::Value resVal(result);
		rapidjson::Value totVal(total);
		rapidjson::Value failVal(failed);

		doc.AddMember("result", resVal, allocator);
		doc.AddMember("total", totVal, allocator);
		doc.AddMember("failed", failVal, allocator);

		rapidjson::Value arr(rapidjson::kArrayType);
		for(std::vector<std::string >::iterator i = errors.begin(); i != errors.end(); ++i)
		{
			rapidjson::Value val((*i).c_str(), allocator);
			arr.PushBack(val, allocator);
		}
		doc.AddMember("errors", arr, allocator);

		rapidjson::Value tObj(rapidjson::kObjectType);
		for(std::map<std::string, std::string >::iterator i = timers.begin(); i != timers.end(); ++i)
		{
			rapidjson::Value key((*i).first.c_str());
			rapidjson::Value val((*i).second.c_str());
			tObj.AddMember(key, val, allocator);
		}
		doc.AddMember("timers", tObj, allocator);

		rapidjson::Value thObj(rapidjson::kObjectType);
		for(std::map<std::string, ThreadInfo >::iterator i = threads.begin(); i != threads.end(); ++i)
		{
			rapidjson::Value tElement(rapidjson::kObjectType);
			
			rapidjson::Value started_threads((*i).second.started_threads);
			rapidjson::Value finished_threads((*i).second.finished_threads);
			rapidjson::Value summoners((*i).second.summoners);
			rapidjson::Value time((*i).second.time.c_str());
			
			tElement.AddMember("started_threads", started_threads, allocator);
			tElement.AddMember("finished_threads", finished_threads, allocator);
			tElement.AddMember("summoners", summoners, allocator);
			tElement.AddMember("time", time, allocator);

			thObj.AddMember((*i).first.c_str(), tElement, allocator);
		}
		doc.AddMember("threads", thObj, allocator);

		// Convert JSON document to string
		rapidjson::StringBuffer strBuf;
		rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
		doc.Accept(writer);

		std::string str = strBuf.GetString();
		printf("--\n%s\n--\n", strBuf.GetString());
		
		fwrite(str.c_str(), str.length(), 1, file);

		fclose(file);
	}
	else
	{
		std::cout << "Could not save report to file: " << fileNameStream.str() << std::endl;
	}
}