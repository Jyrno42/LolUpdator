#ifndef APIDATATYPES_HPP
#define APIDATATYPES_HPP

/* Standard C++ headers */
#include <iostream>
#include <sstream>
#include <string>

/* Boost */
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

class LeagueInfo
{
public:
	int summonerId;
	int tier;
	int rank;
	int leaguePoints;

	std::string name;
	
	bool hotStreak;
	bool freshBlood;
	bool veteran;

	bool valid;
	
	std::string last_error;

	/**
	  * Construct from the returned data.
	  */
	LeagueInfo(const std::string& data, int sId);
};

#endif
