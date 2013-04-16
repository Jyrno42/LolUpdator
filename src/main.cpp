#include <iostream>
#include <stdexcept>

/* Boost */
#include <boost/program_options.hpp>

/* Custom stuff */
#include "Elophant.hpp"
#include "logic.hpp"
#include "Report.hpp"

Report gReport;

// TODO: Add Tracker.

// Later: Add verbose setting which creates a verbose log about all stuff into report.

int main(int argc, const char *argv[])
{
	// Config Vars
	std::string 
		url,
		user,
		database,
		table,
		password,
		reportDir,
		qExtra;

	std::vector<std::string>
		keys;

	// Declare the supported options.
	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("k", boost::program_options::value<std::vector<std::string> >(&keys), "list of keys to use")
		("host", boost::program_options::value<std::string>(&url)) 
		("user", boost::program_options::value<std::string>(&user)) 
		("database", boost::program_options::value<std::string>(&database)) 
		("table", boost::program_options::value<std::string>(&table)) 
		("password", boost::program_options::value<std::string>(&password)) 
		("report-dir", boost::program_options::value<std::string>(&reportDir))  
		("query-extra", boost::program_options::value<std::string>(&qExtra)) 
	;

	boost::program_options::options_description config_file_options;
	config_file_options.add(desc);

	boost::program_options::variables_map vm;
	try {
		boost::program_options::store(boost::program_options::parse_config_file<char>("config.cfg", config_file_options), vm);
	}
	catch(...)
	{

	}
	
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);
	
	if (vm.count("help") || url == "" || user == "" || database == "" || table == "" || reportDir == "" || keys.size() == 0) {
		std::cout << desc << "\n";
		return 1;
	}

	gReport.reportDirectory = reportDir;

	// Init elophant and test api keys.
	Elophant Api(keys, true);
	if (Api.errbit())
	{
		std::stringstream ss;
		ss << "ERROR: ElophantException:" << Api.last_error;

		gReport.errors.push_back(ss.str());
		gReport.GetReport();

		return EXIT_FAILURE;
	}
	else
	{
		keys.clear();
		for(std::map<std::string, int>::iterator i = Api.ApiKeys.begin(); i != Api.ApiKeys.end(); ++i)
		{
			keys.push_back((*i).first);
		}
	}

	sql::Connection * con;
	sql::Driver *driver;
	sql::Statement *stmt;
	sql::ResultSet *res;

	curl_global_init(CURL_GLOBAL_ALL);

	try {
		driver = get_driver_instance();
		
		con = driver->connect(url, user, password); 

		con->setAutoCommit(false);
		con->setSchema(database);

		stmt = con->createStatement();

		std::string query("SELECT * FROM ");
		query.append(table);
		query.append(" ");
		//query.append(" WHERE Name = 'TH3F0X'");
		//query.append(" LIMIT 15");
		query.append(qExtra);

		res = stmt->executeQuery(query);

		retrieve_summoners(res, con, table, keys);
		gReport.GetReport();

		/* Clean up */
		delete res;
		delete stmt;
		con->close();
		curl_global_cleanup();
	} 
	catch (sql::SQLException &e) {
		std::stringstream ss;
		ss << "ERROR: SQLException in " << __FILE__ " (main) on line " << __LINE__ 
			<< " ERROR: " << e.what()<< " (MySQL error code: " << e.getErrorCode()
			<< ", SQLState: " << e.getSQLStateCStr() << ")";
		
		gReport.errors.push_back(ss.str());
		gReport.GetReport();
		
		curl_global_cleanup();

		return EXIT_FAILURE;
	} 
	catch (std::runtime_error &e) {
		std::stringstream ss;
		ss << "ERROR: runtime_error in " << __FILE__ << " (main) on line " << __LINE__ << "ERROR: " << e.what();
		
		gReport.errors.push_back(ss.str());
		gReport.GetReport();

		curl_global_cleanup();

		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
