#include "orwell/Server.hpp"
#include "orwell/BroadcastServer.hpp"

#include <zmq.hpp>

#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <cassert>

#include <log4cxx/ndc.h>

// Yes it is a hack
#define private protected
#include "orwell/Application.hpp"
#undef private

#include "orwell/support/GlobalLogger.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/none.hpp>
#include <boost/call_traits.hpp>

#include <vector>
#include <functional>


struct Arguments
{
	char ** m_argv;
	uint8_t m_argc;

	Arguments()
		: m_argv(nullptr)
		, m_argc(0)
	{
	}

	~Arguments()
	{
		for (size_t i = 0 ; i < m_argc ; ++i)
		{
			delete [] m_argv[i];
		}
		if (nullptr != m_argv)
		{
			free(m_argv);
			m_argv = nullptr;
		}
		m_argc = 0;
	}

	Arguments(Arguments && oOld)
		: m_argv(oOld.m_argv)
		, m_argc(oOld.m_argc)
	{
		oOld.m_argv = nullptr;
		oOld.m_argc = 0;
	}

	void addArgument(char * const argument)
	{
		m_argc += 1;
		char ** tmp_argv = (char **)realloc(m_argv, m_argc * sizeof(char *));
		if (nullptr == tmp_argv)
		{
			std::cerr << "Out of memory !" << std::endl;
			throw 1;
		}
		m_argv = tmp_argv;
		m_argv[m_argc - 1] = argument;
	}
};

std::ostream & operator<<(
		std::ostream & ioOstream,
		Arguments const & iArguments)
{
	for (size_t i = 0 ; i < iArguments.m_argc ; ++i)
	{
		ioOstream << " " << iArguments.m_argv[i];
	}
	return ioOstream;
}

#define ARG_HELP "-h"
#define ARG_PUBLISHER_PORT "-P"
#define ARG_PULLER_PORT "-p"
#define ARG_AGENT_PORT "-A"
#define ARG_ORWELLRC "-r"
#define ARG_TIC_INTERVAL "-T"
#define ARG_VERSION "-v"
#define ARG_DEBUG_LOG "-d"
#define ARG_DRY_RUN "-n"


static void BuildArgument(
		char const * const iName,
		Arguments & ioArguments)
{
	size_t size = strlen(iName) + 1;
	char * str = new char(size);
	str = strncpy(str, iName, size);
	ioArguments.addArgument(str);
}


template< typename INT >
static void BuildIntArgument(
		char const * const iName,
		INT const iValue,
		Arguments & ioArguments)
{
	BuildArgument(iName, ioArguments);
	std::string aString = std::to_string(iValue);
	BuildArgument(aString.c_str(), ioArguments);
}


static void BuildStrArgument(
		char const * const iName,
		char const * const iValue,
		Arguments & ioArguments)
{
	BuildArgument(iName, ioArguments);
	BuildArgument(iValue, ioArguments);
}


static Arguments GetArugments(
		bool const iHelp = false,
		boost::optional< int32_t > const iPublisherPort = boost::none,
		boost::optional< int32_t > const iPullerPort = boost::none,
		boost::optional< int32_t > const iAgentPort = boost::none,
		boost::optional< std::string > const iOrwellRc = boost::none,
		boost::optional< int64_t > const iTicInterval = boost::none,
		bool const iVersion = false,
		bool const iDebugLog = false,
		bool const iDryRun = false)
{
	Arguments arguments;
	BuildArgument("FAKE", arguments);
	if (iHelp)
	{
		BuildArgument(ARG_HELP, arguments);
	}
	if (iPublisherPort)
	{
		BuildIntArgument(ARG_PUBLISHER_PORT, *iPublisherPort, arguments);
	}
	if (iPullerPort)
	{
		BuildIntArgument(ARG_PULLER_PORT, *iPullerPort, arguments);
	}
	if (iAgentPort)
	{
		BuildIntArgument(ARG_AGENT_PORT, *iAgentPort, arguments);
	}
	if (iOrwellRc)
	{
		BuildStrArgument(ARG_ORWELLRC, (*iOrwellRc).c_str(), arguments);
	}
	if (iTicInterval)
	{
		BuildIntArgument(ARG_TIC_INTERVAL, *iTicInterval, arguments);
	}
	if (iVersion)
	{
		BuildArgument(ARG_VERSION, arguments);
	}
	if (iDebugLog)
	{
		BuildArgument(ARG_DEBUG_LOG, arguments);
	}
	if (iDryRun)
	{
		BuildArgument(ARG_DRY_RUN, arguments);
	}
	return arguments;
}

template < typename ReturnType >
static ReturnType WrappExceptionsCall(
		std::function< ReturnType (void) > iFunction,
		ReturnType iResultOnError)
{
	try
	{
		return iFunction();
	}
	catch (std::exception const & exception)
	{
		std::cerr << exception.what() << std::endl;
		return iResultOnError;
	}
	catch (...)
	{
		std::cerr << "Unknown exception caught" << std::endl;
		return iResultOnError;
	}
}


class Application : public orwell::Application
{
public :
	Application();
	bool call_initApplication(int argc, char * argv[]);
	bool call_initConfigurationFile();
	bool call_initServer();
protected :
	static bool InitApplication(int argc, char * argv[])
	{
		return Application::m_CurrentApplication.InitApplication(argc, argv);
	}
	static Application m_CurrentApplication;
};

Application::Application()
{
}

bool Application::call_initApplication(int argc, char * argv[])
{
	return WrappExceptionsCall< bool >(
			std::bind(&Application::initApplication, this, argc, argv), false);
}

bool Application::call_initConfigurationFile()
{
	return WrappExceptionsCall< bool >(
			std::bind(&Application::initConfigurationFile, this), false);
}

bool Application::call_initServer()
{
	return WrappExceptionsCall< bool >(
			std::bind(&Application::initServer, this), false);
}


namespace test
{
enum Status
{
	kFail,
	kPass,
};
}


static void test_initApplication(
		test::Status const iTestStatus,
		Arguments iArguments)
{
	ORWELL_LOG_DEBUG("arguments:" << iArguments);
	Application application;
	bool result = application.call_initApplication(iArguments.m_argc, iArguments.m_argv);
	assert(iTestStatus == result);
}

static void test_nothing()
{
	test_initApplication(test::kFail, GetArugments());
}

static void test_wrong_port_range_publisher_1()
{
	test_initApplication(test::kFail, GetArugments(false, 0, 42, 43));
}

static void test_wrong_port_range_publisher_2()
{
	test_initApplication(test::kPass, GetArugments(false, -1024, 42, 43));
}

static void test_wrong_port_range_publisher_3()
{
	test_initApplication(test::kFail, GetArugments(false, 99999, 42, 43));
}

static void test_same_ports_agent_publisher()
{
	test_initApplication(test::kFail, GetArugments(false, 41, 42, 41));
}

static void test_same_ports_puller_publisher()
{
	test_initApplication(test::kFail, GetArugments(false, 41, 41, 43));
}

static void test_same_ports_puller_agent()
{
	test_initApplication(test::kFail, GetArugments(false, 41, 42, 42));
}

int main()
{
	orwell::support::GlobalLogger::Create("application_errors", "orwell.log", true);
	log4cxx::NDC ndc("application_errors");

	test_nothing();
	test_wrong_port_range_publisher_1();
	test_wrong_port_range_publisher_2();
	test_wrong_port_range_publisher_3();
	test_same_ports_agent_publisher();
	test_same_ports_puller_publisher();
	test_same_ports_puller_agent();

	orwell::support::GlobalLogger::Clear();
	return 0;
}
