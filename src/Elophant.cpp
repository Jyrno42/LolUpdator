#include "Elophant.hpp"

int get_header_tag(std::string tag, std::string headers)
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

int get_tier(std::wstring tier)
{
	if (tier == L"CHALLENGER")
		return 6;
	if (tier == L"DIAMOND")
		return 5;
	if (tier == L"PLATINUM")
		return 4;
	if (tier == L"GOLD")
		return 3;
	if (tier == L"SILVER")
		return 2;
	if (tier == L"BRONZE")
		return 1;

	return 0;
}
int get_rank(std::wstring rank)
{
	if (rank == L"V")
		return 5;
	if (rank == L"IV")
		return 4;
	if (rank == L"III")
		return 3;
	if (rank == L"II")
		return 2;
	if (rank.compare(L"I") == 0)
		return 1;

	return 0;
}

std::map<std::wstring, double> Elophant::getCombinedRankedStatistics (int accountId, std::string Region)
{
	std::map<std::wstring, double> ret;

	std::map<int, std::map<std::wstring, double> > value = getRankedStats(accountId, Region);
	if (value.size() > 0)
	{
		return value[0];
	}
	return ret;
}

std::map<int, std::map<std::wstring, double> > Elophant::getRankedStats (int accountId, std::string Region)
{
	std::stringstream st;
	st << Region << "/ranked_stats/" << accountId << "/CURRENT";

	std::map<int, std::map<std::wstring, double> > ret;

	JSONValue value = _makeCall(st.str());
	if (!value.IsNull())
	{
		if (value.IsObject())
		{
			JSONObject root = value.AsObject();
			if (root.find(L"data") != root.end())
			{
				if(root[L"data"]->IsObject())
				{
					JSONObject data = root[L"data"]->AsObject();
					if (data.find(L"lifetimeStatistics") != data.end())
					{
						if (data[L"lifetimeStatistics"]->IsArray())
						{
							JSONArray lifetimeStatistics = data[L"lifetimeStatistics"]->AsArray();
							for(JSONArray::iterator i = lifetimeStatistics.begin(); i != lifetimeStatistics.end(); i++)
							{
								if ((*i)->IsObject())
								{
									JSONObject o = (*i)->AsObject();
									if (o.find(L"statType") != o.end() &&
										o.find(L"value") != o.end() &&
										o.find(L"championId") != o.end())
									{
										int championId = o[L"championId"]->AsNumber();
										int value = o[L"value"]->AsNumber();
										std::wstring statType = o[L"statType"]->AsString();

										if (ret.find(championId) == ret.end())
										{
											ret[championId] = std::map<std::wstring, double>();
										}
										ret[championId][statType] = value;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return ret;
}

LeagueInfo Elophant::getLeagues (int summonerId, std::string Region)
{
	std::stringstream st;
	st << Region << "/leagues/" << summonerId << "";

	LeagueInfo ret;
	ret.valid = false;

	JSONObject value = _makeCall(st.str());
	if (value.find(L"data") != value.end())
	{
		if (value.find(L"data") != value.end())
		{
			if(value[L"data"]->IsObject())
			{
				JSONObject data = value[L"data"]->AsObject();
				if (data.find(L"summonerLeagues") != data.end())
				{
					if (data[L"summonerLeagues"]->IsArray())
					{
						JSONArray summonerLeagues = data[L"summonerLeagues"]->AsArray();
						for(JSONArray::iterator i = summonerLeagues.begin(); i != summonerLeagues.end(); ++i)
						{
							if ((*i)->IsObject())
							{
								JSONObject o = (*i)->AsObject();
								if (o.find(L"queue") != o.end())
								{
									if (o[L"queue"]->IsString())
									{
										if (o[L"queue"]->AsString() == L"RANKED_SOLO_5x5") // Found my league info.
										{
											ret.name = o[L"name"]->AsString();
											
											// Find me...
											JSONArray entries = o[L"entries"]->AsArray();
											for(JSONArray::iterator j = entries.begin(); j != entries.end(); ++j)
											{
												JSONObject p = (*j)->AsObject();
												int pid = TO_INT(_wtoi(p[L"playerOrTeamId"]->AsString().c_str()));
												if (pid == summonerId)
												{
													ret.tier = get_tier(p[L"tier"]->AsString());
													ret.rank = get_rank(p[L"rank"]->AsString());
													ret.leaguePoints = TO_INT(p[L"leaguePoints"]->AsNumber());

													ret.hotStreak = p[L"hotStreak"]->AsBool();
													ret.freshBlood = p[L"freshBlood"]->AsBool();
													ret.veteran = p[L"veteran"]->AsBool();
													
													ret.valid = true;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		//std::cout << -1 << std::endl;
	}

	return ret;
}
	
int Elophant::get_estimated_elo (Summoner s)
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
    
	if (s.Tier >= 0 && s.Tier < 7)
	{
		tier_s = s.Tier;
	}
        
	double range = ranges[tier_s][1] - ranges[tier_s][0];
	double div = range / 5;
	double rank = 5 - s.Rank;

	return static_cast <int> (std::floor((ranges[tier_s][0] + (div * rank) + (div * (s.LeaguePoints / 100)))));
}

JSONObject Elophant::_makeCall (std::string theFunction)
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
			JSONObject o = _makeCall_inner(theFunction, key);
			return o;
		}

		last_error = "_makeCall: Out of API calls.";
	}
	return JSONObject();
}


JSONObject Elophant::_makeCall_inner (std::string theFunction, std::string key)
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

			JSONValue *value = JSON::Parse(readBuffer.c_str());
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
			}
		}
	}
	return JSONObject();
}