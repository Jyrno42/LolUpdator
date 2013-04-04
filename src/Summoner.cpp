#include "Summoner.hpp"
#include "logic.hpp"

int Summoner::get_estimated_elo ()
{
	double ranges [7][2] = {
		{0, 790}, // 0
		{800, 1240}, // 800 - 1240: Bronze
		{1250, 1490}, // 1250 - 1490: Silver
		{1500, 1840}, // 1500 - 1840: Gold
		{1850, 2240}, // 1850 - 2240: Plat
		{2250, 2540}, // 2250 - 2540: Diamond
		{2550, 3000} // 2550++: Challenger
	};
	int tier_s = 0;
    
	if (Tier >= 0 && Tier < 7)
	{
		tier_s = Tier;
	}
        
	double range = ranges[tier_s][1] - ranges[tier_s][0];
	double div = range / 5;
	double rank = 5 - Rank;

	return TO_INT((ranges[tier_s][0] + (div * rank) + (div * (LeaguePoints / 100))));
}

std::string Summoner::Dump()
{
	std::stringstream dumpStream;

	dumpStream << "Summoner " << Name << ": " << WON << "-" << LOST << ", " << Kills << "/" << Deaths << "/" << Assists << std::endl <<
			
		"\tMaxChampionKills: " << MaxChampionKills << std::endl <<
		"\tMinionKills: " << MinionKills << std::endl <<
		"\tQuadraKills: " << QuadraKills << std::endl <<
		"\tPentaKills: " << PentaKills << std::endl << std::endl <<

		"\t" << "Tier: " << Tier << std::endl <<
		"\t" << "League: " << League << std::endl <<
		"\t" << "Rank: " << Rank << std::endl <<
		"\t" << "LeaguePoints: " << LeaguePoints << std::endl <<
		"\t" << "Score: " << Score << std::endl << std::endl <<
			
		"\t" << "HotStreak: " << (HotStreak ? "Y" : "N") << std::endl <<
		"\t" << "FreshBlood: " << (FreshBlood ? "Y" : "N") << std::endl <<
		"\t" << "Veteran: " << (Veteran ? "Y" : "N") << std::endl <<
		"\t" << "Tracker: " << (Tracker ? "Y" : "N") << std::endl <<
		"";
	return dumpStream.str();
}

std::string Summoner::Query(const std::string& table, sql::mysql::MySQL_Connection * connection)
{
	std::stringstream queryStream;

	std::string escaped = connection->escapeString(League);

	queryStream << "UPDATE " << table << " SET " << 
			
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
	return queryStream.str();
}