
#ifndef ELOPHANT_H
#define ELOPHANT_H

/* Standard C++ headers */
#include <iostream>
#include <map>
#include <sstream>
#include <memory>
#include <string>
#include <stdexcept>

/* Curl */
#include <curl/curl.h>

/* JSON */
#include "JSON.h"

/* Boost */
#include <boost/thread.hpp>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

/* MySQL Connector/C++ specific headers */
#define HAVE_STDINT_H 1
#define CPPCONN_DONT_TYPEDEF_MS_TYPES_TO_C99_TYPES 1
#include "mysql_connection.h"

#include <driver.h>
#include <connection.h>
#include <statement.h>
#include <resultset.h>
#include <exception.h>
#include <warning.h>

#define PER_THREAD 15

#define TO_INT(x) (static_cast <int> (std::floor(x)))

using namespace sql::mysql;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

class Summoner
{
public:
	int AID;
	int SID;

	std::string Region;
	std::string Name;

	int WON;
	int LOST;

	int Kills;
	int Deaths;
	int Assists;
	
	int MaxChampionKills;
	int MinionKills;
	int QuadraKills;
	int PentaKills;
	
	int Tier;
	std::wstring League;
	int Rank;
	int LeaguePoints;
	int Score;

	bool HotStreak;
	bool FreshBlood;

	bool Veteran;
	int Modified;
	bool Tracker;

	bool success;
	std::string error;

	std::string Dump()
	{
		std::stringstream st;

		std::string s;
		s.assign(League.begin(), League.end());

		st << "Summoner " << Name << ": " << WON << "-" << LOST << ", " << Kills << "/" << Deaths << "/" << Assists << std::endl <<
			
			"\tMaxChampionKills: " << MaxChampionKills << std::endl <<
			"\tMinionKills: " << MinionKills << std::endl <<
			"\tQuadraKills: " << QuadraKills << std::endl <<
			"\tPentaKills: " << PentaKills << std::endl << std::endl <<

			"\t" << "Tier: " << Tier << std::endl <<
			"\t" << "League: " << s << std::endl <<
			"\t" << "Rank: " << Rank << std::endl <<
			"\t" << "LeaguePoints: " << LeaguePoints << std::endl <<
			"\t" << "Score: " << Score << std::endl << std::endl <<
			
			"\t" << "HotStreak: " << (HotStreak ? "Y" : "N") << std::endl <<
			"\t" << "FreshBlood: " << (FreshBlood ? "Y" : "N") << std::endl <<
			"\t" << "Veteran: " << (Veteran ? "Y" : "N") << std::endl <<
			"\t" << "Tracker: " << (Tracker ? "Y" : "N") << std::endl <<
			
			"";

		return st.str();
	}

	std::string Query(std::string table, sql::mysql::MySQL_Connection* connection)
	{
		std::stringstream st;

		std::string s;
		s.assign(League.begin(), League.end());

		std::string escaped = connection->escapeString(s);

		st << "UPDATE " << table << " SET " << 
			
				"WON = '" << WON << "', "
				"LOST = '" << LOST << "', "
				
				"Kills = '" << Kills << "', "
				"Deaths = '" << Deaths << "', "
				"Assists = '" << Assists << "', "
				
				"MaxChampionKills = '" << MaxChampionKills << "', "
				"MinionKills = '" << MinionKills << "', "
				"QuadraKills = '" << QuadraKills << "', "
				"PentaKills = '" << PentaKills << "', "
				
				"Tier = '" << Tier << "', "
				"League = '" << escaped << "', "
				"Rank = '" << Rank << "', "
				"LeaguePoints = '" << LeaguePoints << "', "
				"Score = '" << Score << "', "
				
				"HotStreak = '" << (HotStreak ? "1" : "0") << "', "
				"FreshBlood = '" << (FreshBlood ? "1" : "0") << "', "
				"Veteran = '" << (Veteran ? "1" : "0") << "'"

			" WHERE SID = '" << SID << "'";
		return st.str();
	}
};

class LeagueInfo
{
public:
	int tier;
	int rank;
	int leaguePoints;

	std::wstring name;
	
	bool hotStreak;
	bool freshBlood;
	bool veteran;

	bool valid;
};

class Elophant
{
public:
	Elophant (std::vector<std::string > apiKeys, bool check)
	{
		curl = curl_easy_init();
	
		has_api = true;
		if (curl)
		{
			for (std::vector<std::string >::iterator i = apiKeys.begin(); i != apiKeys.end(); ++i) 
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

	std::map<std::wstring, double> getCombinedRankedStatistics (int accountId, std::string Region);
	std::map<int, std::map<std::wstring, double> > getRankedStats (int accountId, std::string Region);
	LeagueInfo getLeagues (int accountId, std::string Region);
	
	int get_estimated_elo (Summoner s);

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
	
	JSONObject _makeCall (std::string theFunction);
	JSONObject _makeCall_inner (std::string theFunction, std::string key);

	int test_api_key(std::string key, bool check)
	{
		if (check)
		{
			ApiKeys[key] = -1;

			JSONObject ret = _makeCall_inner("champions", key);
			if (ret.find(L"data") == ret.end())
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
