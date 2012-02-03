
#include "BatchQuery.h"

int main(void)
{
	BatchQuery::Servers servers;
	for (int i = 0; i < 5000/4; ++i)
	{
		servers.push_back("127.0.0.1:47201");
		servers.push_back("127.0.0.1:47202");
		servers.push_back("127.0.0.1:47203");
		servers.push_back("127.0.0.1:47204");
		//servers.push_back("127.0.0.1:47205");
	}
	BatchQuery::Responses responses;

	BatchQuery BatchQuery;
	BatchQuery.process(servers, responses);

	for (unsigned int i = 0; i < servers.size(); ++i)
	{
		printf("%s %s\n", servers[i].c_str(), responses[i].c_str());
	}

	return 0;
}