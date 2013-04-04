#ifndef ELOPHANT_HPP
#define ELOPHANT_HPP

/* Standard C++ headers */
#include <iostream>
#include <map>
#include <sstream>
#include <memory>
#include <string>

/* Curl */
#include <curl/curl.h>

/* Boost */
#include <boost/thread.hpp>

#include <driver.h>
#include <connection.h>
#include <statement.h>
#include <resultset.h>
#include <exception.h>
#include <warning.h>

/* Custom */
#include "ApiDataTypes.hpp"

class Elophant
{
public:
	Elophant (const std::vector<std::string > apiKeys, bool check)
	{
		last_error = "";

		curl = curl_easy_init();
	
		has_api = true;
		if (curl)
		{
			for (std::vector<std::string >::const_iterator i = apiKeys.begin(); i != apiKeys.end(); ++i) 
			{
				int ret = test_api_key(*i, check);
				if (ret < 10) {
					ApiKeys.erase(*i);
				} 
			}

			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

			if (ApiKeys.size() < 1)
			{
				last_error = "Elophant:: Out of API keys.";
				has_api = false;
			}
			else
			{
				has_api = true;
			}
		}
		else
		{
			last_error = "Elophant:: Curl init failed.";
			has_api = false;
		}
	}
	~Elophant() 
	{
		ApiKeys.clear();
		if (curl)
		{
			curl_easy_cleanup(curl);
		}
	}

	std::map<std::string, double> getCombinedRankedStatistics (int accountId, const std::string& Region);
	std::map<int, std::map<std::string, double> > getRankedStats (int accountId, const std::string& Region);
	LeagueInfo getLeagues (int accountId, const std::string& Region);
	
	std::string last_error;

	bool errbit()
	{
		return !(has_api && curl != NULL);
	}
	std::map<std::string, int> ApiKeys;

private:
	CURL *curl;
	CURLcode res;

	bool has_api;
	
	std::string _makeCall (const std::string& theFunction);
	std::string _makeCall_inner (const std::string& theFunction, const std::string& key);

	int test_api_key(const std::string& key, bool check)
	{
		if (check)
		{
			ApiKeys[key] = -1;

			std::string ret = _makeCall_inner("champions", key);
			try {
				std::stringstream dataStream;
				dataStream << ret;

				boost::property_tree::ptree pt;
				boost::property_tree::read_json(dataStream, pt);
				if(!pt.get<bool>("success"))
				{
					throw std::exception();
				}
			}
			catch(...)
			{
				last_error = "";
				last_error.append("ApiKey ").append(key).append(" is invalid.");
			}
			return ApiKeys[key];
		}
		else
		{
			ApiKeys[key] = 999;
			return ApiKeys[key];
		}
	}
};

#endif
