#ifndef SUMMONER_HPP
#define SUMMONER_HPP

/* Standard C++ headers */
#include <iostream>
#include <memory>

/* MySQL Connector/C++ specific headers */
#define HAVE_STDINT_H 1
#define CPPCONN_DONT_TYPEDEF_MS_TYPES_TO_C99_TYPES 1
#include "mysql_connection.h"

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
	std::string League;
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

	int get_estimated_elo ();

	std::string Dump();
	std::string Query(const std::string& table, sql::mysql::MySQL_Connection * connection);
};

#endif
