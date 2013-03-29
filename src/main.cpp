/* Custom stuff */
#include "Elophant.hpp"
#include "logic.hpp"

// TODO: Add output directory for reports.
// TODO: Log all errors into report.errors[] (FATAL: result=false, WARNING: result=true)
// TODO: Log all info logs into report.info[]
// TODO: Add verbose setting which creates a verbose log about all stuff into report.

// TODO: Better report managment. (Report file class or smth)

// TODO: Add setting to specify summonername filter.


int main(int argc, const char *argv[])
{
	// Config Vars
	std::string 
		url,
		user,
		database,
		table,
		password;

	std::vector<std::string>
		keys;

	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("k", po::value<std::vector<std::string> >(&keys), "list of keys to use")
		("host", po::value<std::string>(&url)) 
		("user", po::value<std::string>(&user)) 
		("database", po::value<std::string>(&database)) 
		("table", po::value<std::string>(&table)) 
		("password", po::value<std::string>(&password)) 
	;

	po::options_description config_file_options;
	config_file_options.add(desc);

	po::variables_map vm;
	try {
		po::store(po::parse_config_file<char>("config.cfg", config_file_options), vm);
	}
	catch(...)
	{

	}
	
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	
	if (vm.count("help") || url == "" || user == "" || database == "" || table == "" || keys.size() == 0) {
		std::cout << desc << "\n";
		return 1;
	}

	// Init elophant and test api keys.
	Elophant Api(keys, true);
	if (Api.errbit())
	{
		std::stringstream ss;
		ss << "ERROR: ElophantException:" << Api.last_error;

		std::wstring ws;
		std::string str(ss.str());
		ws.assign(str.begin(), str.end());

		JSONObject report;
		report[L"result"] = new JSONValue(false);
		report[L"msg"] = new JSONValue(ws);

		JSONValue value(report);
		std::wcout << value.Stringify() << std::endl;

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

	sql::Driver *driver;
	sql::Connection *con;
	sql::Statement *stmt;
	sql::ResultSet *res;

	curl_global_init(CURL_GLOBAL_ALL);

	try {
		driver = get_driver_instance();
		con = driver->connect(url, user, password);
		con->setAutoCommit(0);
		con->setSchema(database);
		stmt = con->createStatement();

		std::string query("SELECT * FROM ");
		query.append(table);
		//query.append(" WHERE Name = 'TH3F0X'");

		res = stmt->executeQuery(query);

		retrieve_summoners(res, con, table, keys);

		/* Clean up */
		delete res;
		delete stmt;
		con->close();
		delete con;
		curl_global_cleanup();
	} 
	catch (sql::SQLException &e) {
		std::stringstream ss;
		ss << "ERROR: SQLException in " << __FILE__ " (main) on line " << __LINE__ 
			<< " ERROR: " << e.what()<< " (MySQL error code: " << e.getErrorCode()
			<< ", SQLState: " << e.getSQLStateCStr() << ")";

		std::wstring ws;
		std::string str(ss.str());
		ws.assign(str.begin(), str.end());

		JSONObject report;
		report[L"result"] = new JSONValue(false);
		report[L"msg"] = new JSONValue(ws);

		JSONValue value(report);
		std::wcout << value.Stringify() << std::endl;
		
		curl_global_cleanup();

		return EXIT_FAILURE;
	} 
	catch (std::runtime_error &e) {
		std::stringstream ss;
		ss << "ERROR: runtime_error in " << __FILE__ << " (main) on line " << __LINE__ << "ERROR: " << e.what();

		std::wstring ws;
		std::string str(ss.str());
		ws.assign(str.begin(), str.end());

		JSONObject report;
		report[L"result"] = new JSONValue(false);
		report[L"msg"] = new JSONValue(ws);

		JSONValue value(report);
		std::wcout << value.Stringify() << std::endl;

		curl_global_cleanup();

		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}