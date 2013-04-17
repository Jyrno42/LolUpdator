
#include "logic.hpp"
#include "Report.hpp"

#define HAVE_STDINT_H 1
#define CPPCONN_DONT_TYPEDEF_MS_TYPES_TO_C99_TYPES 1
#include "mysql_connection.h"

extern Report gReport;

int get_tier(const std::string& tier)
{
	if (tier.compare("CHALLENGER") == 0)
		return 6;
	if (tier.compare("DIAMOND") == 0)
		return 5;
	if (tier.compare("PLATINUM") == 0)
		return 4;
	if (tier.compare("GOLD") == 0)
		return 3;
	if (tier.compare("SILVER") == 0)
		return 2;
	if (tier.compare("BRONZE") == 0)
		return 1;

	return 0;
}
int get_rank(const std::string& rank)
{
	if (rank.compare("V") == 0)
		return 5;
	if (rank.compare("IV") == 0)
		return 4;
	if (rank.compare("III") == 0)
		return 3;
	if (rank.compare("II") == 0)
		return 2;
	if (rank.compare("I") == 0)
		return 1;

	return 0;
}

int get_header_tag(const std::string& tag, const std::string& headers)
{
	std::stringstream ss(headers);
	std::string line;

	while(std::getline(ss, line))
	{
		if( line != "")
		{
			if (line.substr(0, tag.length()) == tag)
			{
				int ret = atoi(line.substr(tag.length()+1).c_str());
				return ret;
			}
		}
	}
	return 0;
}



int do_one_summoner(Summoner_Ptr summoner, const std::vector<std::string >& apiKeys)
{
	Elophant m_api(apiKeys, false);
	summoner->success = false;
	
	LeagueInfo leagueInfo = m_api.getLeagues(summoner->SID, summoner->Region);
	if (leagueInfo.valid)
	{
		std::map<std::string, double> stats = m_api.getCombinedRankedStatistics(summoner->AID, summoner->Region);
		if (stats.size() > 0)
		{
			summoner->WON = TO_INT(stats["TOTAL_SESSIONS_WON"]);
			summoner->LOST = TO_INT(stats["TOTAL_SESSIONS_PLAYED"]) - TO_INT(stats["TOTAL_SESSIONS_WON"]);

			summoner->Kills = TO_INT(stats["TOTAL_CHAMPION_KILLS"]);
			summoner->Deaths = TO_INT(stats["TOTAL_DEATHS_PER_SESSION"]);
			summoner->Assists = TO_INT(stats["TOTAL_ASSISTS"]);
			
			summoner->MaxChampionKills = TO_INT(stats["MOST_CHAMPION_KILLS_PER_SESSION"]);
			summoner->MinionKills = TO_INT(stats["TOTAL_MINION_KILLS"] + stats["TOTAL_NEUTRAL_MINIONS_KILLED"]);
			summoner->QuadraKills = TO_INT(stats["TOTAL_QUADRA_KILLS"]);
			summoner->PentaKills = TO_INT(stats["TOTAL_PENTA_KILLS"]);

			// League data
			summoner->Tier = leagueInfo.tier;
			summoner->League = leagueInfo.name;
			
			summoner->Rank = leagueInfo.rank;
			summoner->LeaguePoints = leagueInfo.leaguePoints;
			summoner->Score = summoner->get_estimated_elo();
			
			summoner->HotStreak = leagueInfo.hotStreak;
			summoner->FreshBlood = leagueInfo.freshBlood;
			summoner->Veteran = leagueInfo.veteran;
			summoner->success = true;
		}
		else
		{
			summoner->error = m_api.last_error;
			return -1;
		}
	}
	else
	{
		summoner->error = leagueInfo.last_error;
		return -1;
	}
	return 0;
}

void * do_one_thread(std::vector<Summoner_Ptr> s, const std::vector<std::string >& ApiKeys, int id)
{
	//std::cout << "thread: " << id << std::endl;
	for(std::vector<Summoner_Ptr>::iterator i = s.begin(); i != s.end(); ++i)
	{
		if (do_one_summoner((*i), ApiKeys) == 0)
		{
			//std:: cout << "\ts" << id << std::endl;
		}
		else
		{
			//std:: cout << "\tf" << id << std::endl;
		}
	}
	return NULL;
}

void retrieve_summoners (sql::ResultSet *rs, sql::Connection * mcon, std::string tbl_name, std::vector<std::string > ApiKeys)
{
	boost::timer::cpu_timer 
		totalTimer;
	
	sql::mysql::MySQL_Connection * con = (dynamic_cast<sql::mysql::MySQL_Connection*>(mcon));

	boost::timer::cpu_timer 
		initTimer;
	int total = rs->rowsCount();

	std::map<std::string,std::map<int,Summoner> > summoners;

	summoners["euw"] = std::map<int,Summoner>();
	summoners["eune"] = std::map<int,Summoner>();
	summoners["na"] = std::map<int,Summoner>();

	while (rs->next()) {
		Summoner s;

		s.AID = rs->getInt("AID");
		s.SID = rs->getInt("SID");
		
		s.Region = rs->getString("Region");
		s.Name = rs->getString("Name");

		std::transform(s.Region.begin(), s.Region.end(), s.Region.begin(), ::tolower);

		s.WON = rs->getInt("WON");
		s.LOST = rs->getInt("LOST");
		
		s.Kills = rs->getInt("Kills");
		s.Deaths = rs->getInt("Deaths");
		s.Assists = rs->getInt("Assists");
		
		s.MaxChampionKills = rs->getInt("MaxChampionKills");
		s.MinionKills = rs->getInt("MinionKills");
		s.QuadraKills = rs->getInt("QuadraKills");
		s.PentaKills = rs->getInt("PentaKills");
		
		s.Tier = rs->getInt("Tier");
		
		std::string str(rs->getString("League"));
		s.League.assign(str.begin(), str.end());
		
		s.Rank = rs->getInt("Rank");
		s.LeaguePoints = rs->getInt("LeaguePoints");
		s.Score = rs->getInt("Score");
		s.Modified = rs->getInt("Modified");
		
		s.HotStreak = rs->getBoolean("HotStreak");
		s.FreshBlood = rs->getBoolean("FreshBlood");
		s.Veteran = rs->getBoolean("Veteran");
		s.Tracker = rs->getBoolean("Tracker");

		summoners[s.Region.c_str()][s.AID] = s;
	}
	initTimer.stop();

	boost::timer::cpu_timer 
		chunkTimer;
	// For each region.
	for(std::map<std::string,std::map<int,Summoner> >::iterator i = summoners.begin(); i != summoners.end(); ++i)
	{
		chunkTimer.resume();
		int region_thread_count = 0;
		int num_summoners = 0;
		std::vector<boost::thread * > vec;
		std::vector<Summoner_Ptr > chunk;

		int n = 0;
		// For each summoner in region.
		for(std::map<int,Summoner>::iterator j = (*i).second.begin(); j != (*i).second.end(); ++j)
		{
			num_summoners++;
			if (chunk.size() < PER_THREAD)
			{
				chunk.push_back(&((*j).second));
			}

			if (chunk.size() >= PER_THREAD || chunk.size() == (*i).second.size())
			{
				region_thread_count++;
				vec.push_back(new boost::thread(boost::bind(&do_one_thread, chunk, ApiKeys, n)));
				chunk.clear();
				n++;
			}
		}
		chunkTimer.stop();

		boost::timer::cpu_timer 
			threadTimer;
		ThreadInfo t;
		t.started_threads = vec.size();
		t.finished_threads = 0;
		t.summoners = num_summoners;
		for(std::vector<boost::thread *>::iterator cthread = vec.begin(); cthread != vec.end(); ++cthread)
		{
			(*cthread)->join();
			t.finished_threads++;
		}

		threadTimer.stop();
		t.time = threadTimer.format();
		gReport.threads[(*i).first] = t;
	}

	int failed = 0;
	boost::timer::cpu_timer 
		sqlUpdateTimer;
	for(std::map<std::string,std::map<int,Summoner> >::iterator i = summoners.begin(); i != summoners.end(); ++i)
	{
		for(std::map<int,Summoner>::iterator j = (*i).second.begin(); j != (*i).second.end(); ++j)
		{
			try
			{
				if ((*j).second.success)
				{
					sql::Statement *stmt = con->createStatement();
					int ret = stmt->executeUpdate((*j).second.Query(tbl_name, con));
					delete stmt;
				}
				else
				{
					failed++;
					gReport.errors.push_back((*j).second.error);
				}
			}
			catch(sql::SQLException &e) {
				failed++;

				std::stringstream ss;
				ss << "ERROR: SQLException in " << __FILE__ " (main) on line " << __LINE__ 
				   << " ERROR: " << e.what()<< " (MySQL error code: " << e.getErrorCode()
				   << ", SQLState: " << e.getSQLStateCStr() << ")";
				
				(*j).second.success = false;
				(*j).second.error = ss.str();
				
				gReport.errors.push_back((*j).second.error);
			}
		}
	}
	sqlUpdateTimer.stop();

	// Save
	boost::timer::cpu_timer
		sqlCommitTimer;
	con->commit();
	sqlCommitTimer.stop();

	// Finish totalTimer.
	totalTimer.stop();

	// Generate report
	gReport.failed = failed;
	gReport.total = total;

	gReport.result = true;
	
	gReport.timers["total"] = totalTimer.format();
	gReport.timers["init"] = initTimer.format();
	gReport.timers["chunk"] = chunkTimer.format();
	gReport.timers["sql_update"] = sqlUpdateTimer.format();
	gReport.timers["sql_commit"] = sqlCommitTimer.format();
}

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}