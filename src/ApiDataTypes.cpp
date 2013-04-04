#include "ApiDataTypes.hpp"
#include "logic.hpp"

LeagueInfo::LeagueInfo(const std::string& data, int sId)
{
	summonerId = sId;

	std::stringstream dataStream;
	dataStream << data;

	try {
		boost::property_tree::ptree pt;
		boost::property_tree::read_json(dataStream, pt);

		if(pt.get<bool>("success"))
		{
			BOOST_FOREACH(boost::property_tree::ptree::value_type &leagues, pt.get_child("data.summonerLeagues")){
				if(leagues.second.get<std::string>("queue") == "RANKED_SOLO_5x5")
				{
					name = leagues.second.get<std::string>("name");

					BOOST_FOREACH(boost::property_tree::ptree::value_type &entries, leagues.second.get_child("entries")){
						int pId = entries.second.get<int>("playerOrTeamId");
						if(pId == summonerId)
						{
							tier = get_tier(entries.second.get<std::string>("tier"));
							rank = get_rank(entries.second.get<std::string>("rank"));
							leaguePoints = entries.second.get<int>("leaguePoints");

							hotStreak = entries.second.get<bool>("hotStreak");
							freshBlood = entries.second.get<bool>("freshBlood");
							veteran = entries.second.get<bool>("veteran");					
							valid = true;
						}
					}
				}
			}
		}
		else
		{
			throw std::exception(std::string("Failed Call: ").append(data).c_str());
		}
	}
	catch(...)
	{
		valid = false;
	}
}