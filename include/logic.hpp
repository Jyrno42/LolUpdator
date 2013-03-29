
#ifndef LOGIC_HPP
#define LOGIC_HPP

#include <map>
#include <vector>

#include "Elophant.hpp"


static void * do_one_summoner(void * summoner, std::vector<std::string > apiKeys)
{
	Elophant m_api(apiKeys, false);
	Summoner * s = (Summoner *)summoner;
	s->success = false;
	
	LeagueInfo l = m_api.getLeagues(s->SID, s->Region);
	if (l.valid)
	{
		std::map<std::wstring, double> stats = m_api.getCombinedRankedStatistics(s->AID, s->Region);
		if (stats.size() > 0)
		{
			s->WON = TO_INT(stats[L"TOTAL_SESSIONS_WON"]);
			s->LOST = TO_INT(stats[L"TOTAL_SESSIONS_PLAYED"]) - TO_INT(stats[L"TOTAL_SESSIONS_WON"]);

			s->Kills = TO_INT(stats[L"TOTAL_CHAMPION_KILLS"]);
			s->Deaths = TO_INT(stats[L"TOTAL_DEATHS_PER_SESSION"]);
			s->Assists = TO_INT(stats[L"TOTAL_ASSISTS"]);
			
			s->MaxChampionKills = TO_INT(stats[L"MOST_CHAMPION_KILLS_PER_SESSION"]);
			s->MinionKills = TO_INT(stats[L"TOTAL_MINION_KILLS"] + stats[L"TOTAL_NEUTRAL_MINIONS_KILLED"]);
			s->QuadraKills = TO_INT(stats[L"TOTAL_QUADRA_KILLS"]);
			s->PentaKills = TO_INT(stats[L"TOTAL_PENTA_KILLS"]);

			// League data
			s->Tier = l.tier;
			s->League = l.name;
			
			s->Rank = l.rank;
			s->LeaguePoints = l.leaguePoints;
			s->Score = m_api.get_estimated_elo(*s);
			
			s->HotStreak = l.hotStreak;
			s->FreshBlood = l.freshBlood;
			s->Veteran = l.veteran;
			s->success = true;
		}
	}

	if (!s->success)
	{
		s->error = m_api.last_error;
	}
	return NULL;
}

static void * do_one_thread(std::vector<Summoner*> s, std::vector<std::string > ApiKeys)
{
	for(std::vector<Summoner*>::iterator i = s.begin(); i != s.end(); ++i)
	{
		do_one_summoner((*i), ApiKeys);
	}
	return NULL;
}

static void retrieve_summoners (sql::ResultSet *rs, sql::Connection * con, std::string tbl_name, std::vector<std::string > ApiKeys)
{
	clock_t total_b;
	total_b = clock();

	JSONArray errors;
	JSONArray threads;
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

	// For each region.
	for(std::map<std::string,std::map<int,Summoner> >::iterator i = summoners.begin(); i != summoners.end(); ++i)
	{
		int region_thread_count = 0;
		std::vector<boost::thread*> vec;

		std::vector<Summoner*> chunk;

		// For each summoner in region.
		for(std::map<int,Summoner>::iterator j = (*i).second.begin(); j != (*i).second.end(); ++j)
		{
			if (chunk.size() < PER_THREAD)
			{
				chunk.push_back(&((*j).second));
			}

			if (chunk.size() >= PER_THREAD || chunk.size() == (*i).second.size())
			{
				region_thread_count++;
				vec.push_back(new boost::thread(&do_one_thread, chunk, ApiKeys));
				chunk.clear();
			}
		}
		
		JSONObject c_threads;
		c_threads[L"started"] = new JSONValue((double)region_thread_count);
		
		clock_t begin, end;
		double time_spent;
		begin = clock();

		for(std::vector<boost::thread*>::iterator cthread = vec.begin(); cthread != vec.end(); cthread++)
		{
			(*cthread)->join();
		}

		end = clock();
		time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		c_threads[L"finished"] = new JSONValue((double)region_thread_count);
		c_threads[L"time"] = new JSONValue(time_spent);
		threads.push_back(new JSONValue(c_threads));
	}

	sql::mysql::MySQL_Connection* mysql_conn = dynamic_cast<sql::mysql::MySQL_Connection*>(con);
	int failed = 0;
	clock_t sql_update_b = clock();
	for(std::map<std::string,std::map<int,Summoner> >::iterator i = summoners.begin(); i != summoners.end(); ++i)
	{
		for(std::map<int,Summoner>::iterator j = (*i).second.begin(); j != (*i).second.end(); ++j)
		{
			try
			{
				if ((*j).second.success)
				{
					sql::Statement *stmt = con->createStatement();
					int ret = stmt->executeUpdate((*j).second.Query(tbl_name, mysql_conn));
					delete stmt;
				}
				else
				{
					failed++;
					std::wstring s;
					s.assign((*j).second.error.begin(), (*j).second.error.end());
					errors.push_back(new JSONValue(s));
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

				std::wstring s;
				s.assign((*j).second.error.begin(), (*j).second.error.end());
				errors.push_back(new JSONValue(s));
			}
		}
	}
	double time_sql_update = (double)(clock() - sql_update_b) / CLOCKS_PER_SEC;

	// Save
	clock_t sql_commit_b = clock();
	con->commit();
	double time_sql_commit = (double)(clock() - sql_commit_b) / CLOCKS_PER_SEC;

	JSONObject report;
	report[L"result"] = new JSONValue(true);
	report[L"total"] = new JSONValue((double)total);
	report[L"failed"] = new JSONValue((double)failed);
	report[L"errors"] = new JSONValue(errors);
	report[L"threads"] = new JSONValue(threads);
	report[L"time"] = new JSONValue((double)(clock() - total_b) / CLOCKS_PER_SEC);
	report[L"time_sql_commit"] = new JSONValue(time_sql_commit);
	report[L"time_sql_update"] = new JSONValue(time_sql_update);

	JSONValue value(report);
	std::wcout << value.Stringify() << std::endl;
}


#endif