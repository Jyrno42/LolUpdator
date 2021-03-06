#include "ApiDataTypes.hpp"
#include "logic.hpp"

LeagueInfo::LeagueInfo(const std::string& data, int sId)
{
	summonerId = sId;

	std::stringstream dataStream;
	dataStream << data;
	valid = false;
	
	try {
		if (data.length() > 0)
		{
			boost::property_tree::ptree pt;
			boost::property_tree::read_json(dataStream, pt);

			last_error = "No LeagueInfo";
			
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
								last_error = "";
							}
						}
					}
				}
			}
			else
			{
				std::stringstream errStream;
				errStream << "Success False: " << data;
				last_error = errStream.str();
			}
		}
		else
		{
			last_error = "Empty Result";
		}
	}
	catch (boost::exception &e)
	{
		last_error = "Invalid JSON";
	}
	catch(...)
	{
		last_error = "Unknown Error";
	}
}