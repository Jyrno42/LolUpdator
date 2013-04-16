#ifndef LOGIC_HPP
#define LOGIC_HPP

/* Standard C++ headers */
#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include <boost/timer/timer.hpp>

#include "Summoner.hpp"
#include "Elophant.hpp"

typedef Summoner* Summoner_Ptr;

#define PER_THREAD 15
#define TO_INT(x) (static_cast <int> (std::floor(x)))

int get_tier(const std::string& tier);
int get_rank(const std::string& rank);

int do_one_summoner(Summoner_Ptr summoner, const std::vector<std::string >& apiKeys, int id);
void * do_one_thread(std::vector<Summoner_Ptr> s, const std::vector<std::string >& ApiKeys);
void retrieve_summoners (sql::ResultSet *rs, sql::Connection * mcon, std::string tbl_name, std::vector<std::string > ApiKeys);
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
int get_header_tag(const std::string& tag, const std::string& headers);

#endif
