#include "Elophant.hpp"
#include "logic.hpp"

std::map<std::string, double> Elophant::getCombinedRankedStatistics (int accountId, const std::string& Region)
{
	std::map<std::string, double> ret;

	try {
		std::map<int, std::map<std::string, double> > value = getRankedStats(accountId, Region);
		if (value.size() > 0)
		{
			return value[0];
		}
	}
	catch(...)
	{
	}
	return ret;
}

std::map<int, std::map<std::string, double> > Elophant::getRankedStats (int accountId, const std::string& Region)
{
	std::stringstream st;
	st << Region << "/ranked_stats/" << accountId << "/CURRENT";

	std::map<int, std::map<std::string, double> > ret;

	try {
		std::string data = _makeCall(st.str());

		if (data.length() > 0)
		{
			std::stringstream dataStream;
			dataStream << data;

			boost::property_tree::ptree pt;
			boost::property_tree::read_json(dataStream, pt);

			if(pt.get<bool>("success"))
			{
				BOOST_FOREACH(boost::property_tree::ptree::value_type &stats, pt.get_child("data.lifetimeStatistics")){
					int championId = stats.second.get<int>("championId");
					int value = stats.second.get<int>("value");
					std::string statType = stats.second.get<std::string>("statType");
				
					if (ret.find(championId) == ret.end())
					{
						ret[championId] = std::map<std::string, double>();
					}
					ret[championId][statType] = value;

				}
			}
			else
			{
				std::stringstream errStream;
				errStream << "Success False: " << data;
				last_error = errStream.str();
				valid = false;
			}
		}
		else
		{
			last_error = "Empty Result";
			valid = false;
		}
	}
	catch (boost::exception &e)
	{
		last_error = "Invalid JSON";
		valid = false;
	}
	catch(...)
	{
		last_error = "Unknown Error";
		valid = false;
	}
	return ret;
}

LeagueInfo Elophant::getLeagues (int summonerId, const std::string& Region)
{
	std::stringstream st;
	st << Region << "/leagues/" << summonerId << "";
	
	std::string result = _makeCall(st.str());

	return LeagueInfo(result, summonerId);
}

std::string Elophant::_makeCall (const std::string& theFunction)
{
	if (has_api)
	{
		std::string key;
		int remaining = INT_MIN;

		for(std::map<std::string, int>::iterator i = ApiKeys.begin(); i != ApiKeys.end(); ++i)
		{
			if ((*i).second > remaining || key.length() == 0)
			{
				remaining = (*i).second;
				key = (*i).first;
			}
		}

		if (key.length() != 0)
		{
			return _makeCall_inner(theFunction, key);
		}

		last_error = "_makeCall: Out of API calls.";
	}
	return "";
}


std::string Elophant::_makeCall_inner (const std::string& theFunction, const std::string& key)
{
	if (curl)
	{
		std::string url("http://api.elophant.com/v2/");
		url.append(theFunction);
		url.append("?key=");
		url.append(key);

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		
		std::string readBuffer;
		std::string headBuffer;

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &headBuffer);


		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
		{
			std::stringstream ss;
			ss << "curl_easy_perform() failed: " << res << curl_easy_strerror(res);
			last_error = ss.str();
		}
		else
		{
			ApiKeys[key] = get_header_tag("Developer-Remaining", headBuffer);

			return readBuffer;
			/*JSONValue *value = JSON::Parse(readBuffer.c_str());
			if (value != NULL)
			{
				if (value->IsObject())
				{
					JSONObject root = value->AsObject();
					if (root.find(L"success") != root.end())
					{
						if (root[L"success"]->AsBool())
						{
							return root;
						}
						else
						{
							std::stringstream ss;
							ss << "GET " << url.c_str() << " - obj->success == false. (" << readBuffer << ")" << std::endl;
							last_error = ss.str();
						}
					}
					else
					{
						std::stringstream ss;
						ss << "GET " << url.c_str() << " - No obj->success element. (" << readBuffer << ")" << std::endl;
						last_error = ss.str();
					}
				}
				else
				{
					std::stringstream ss;
					ss << "GET " << url.c_str() << " - Not JSON obj. (" << readBuffer << ")" << std::endl;
					last_error = ss.str();
				}
			}
			else
			{
				std::stringstream ss;
				ss << "GET " << url.c_str() << " - Not JSON data. (" << readBuffer << ")" << std::endl;
				last_error = ss.str();
			}*/
		}
	}
	return "";
}