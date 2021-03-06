#include <zmq.hpp>

#include <sys/types.h>
#include <cassert>

#include <boost/lexical_cast.hpp>

#include <log4cxx/ndc.h>

#include <gtest/gtest.h>

#include "orwell/Application.hpp"
#include "orwell/AgentProxy.hpp"
#include "orwell/Server.hpp"
#include "orwell/com/Receiver.hpp"
#include "orwell/com/Url.hpp"

#include "orwell/support/GlobalLogger.hpp"

#include "Common.hpp"

bool gOK;

#define ORWELL_ASSERT(Expected, Received, Message) \
{\
	if (Expected != Received)\
	{\
		ORWELL_LOG_ERROR("expected: " << Expected);\
		ORWELL_LOG_ERROR(Message);\
		return;\
	}\
}\

static void test_1(orwell::Application & ioApplication)
{
	TestAgent aTestAgent(9003);
	gOK = false;
	ORWELL_LOG_DEBUG("test_1");
	orwell::AgentProxy aAgentProxy(ioApplication);
	std::string aAgentReply;
	std::string aPlayerList;
	std::string aRobotList;
	assert(aAgentProxy.step("add player Player1", aAgentReply));
	assert(aAgentProxy.step("add robot Robot1", aAgentReply));
	// list player {
	assert(aAgentProxy.step("list player", aPlayerList));
	ORWELL_LOG_DEBUG("aPlayerList = " << aPlayerList);
	std::string aExpectedPlayerList(R"(Players:
	Player1 -> name = Player1 ; robot = 
)");
	ORWELL_ASSERT(aExpectedPlayerList, aPlayerList, "list player KO");
	// } list player
	// list robot {
	assert(aAgentProxy.step("list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	std::string aExpectedRobotList(R"(Robots:
	Robot1 -> name = Robot1 ; not registered ; video_url =  ; player = 
)");
	ORWELL_ASSERT(aExpectedRobotList, aRobotList, "list robot KO");
	// } list robot
	// register robot {
	assert(aAgentProxy.step("register robot Robot1", aAgentReply));
	// make sure that Robot1 is now registered
	assert(aAgentProxy.step("list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	aExpectedRobotList = (R"(Robots:
	Robot1 -> name = Robot1 ; registered ; video_url =  ; player = 
)");
	ORWELL_ASSERT(aExpectedRobotList, aRobotList, "register KO");
	// } register robot
	// set robot {
	assert(aAgentProxy.step("set robot Robot1 video_url titi", aAgentReply));
	// } set robot
	// unregister robot {
	assert(aAgentProxy.step("unregister robot Robot1", aAgentReply));
	// make sure that Robot1 is now unregistered
	assert(aAgentProxy.step("list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	aExpectedRobotList = (R"(Robots:
	Robot1 -> name = Robot1 ; not registered ; video_url = titi ; player = 
)");
	ORWELL_ASSERT(aExpectedRobotList, aRobotList, "unregister KO");
	// } unregister robot
	assert(aAgentProxy.step("start game", aAgentReply));
	assert(aAgentProxy.step("stop game", aAgentReply));
	assert(aAgentProxy.step("remove robot Robot1", aAgentReply));
	assert(aAgentProxy.step("remove player Player1", aAgentReply));
	assert(aAgentProxy.step("list player", aPlayerList));
	ORWELL_LOG_DEBUG("aPlayerList = " << aPlayerList);
	aExpectedPlayerList = (R"(Players:
)");
	ORWELL_ASSERT(aExpectedPlayerList, aPlayerList, "empty player KO");
	assert(aAgentProxy.step("list robot", aRobotList));
	ORWELL_LOG_DEBUG("aRobotList = " << aRobotList);
	aExpectedRobotList = (R"(Robots:
)");
	ORWELL_ASSERT(aExpectedRobotList, aRobotList, "empty robot KO");
	assert(aAgentProxy.step("stop application", aAgentReply));
	gOK = true;
}


int main()
{
	orwell::support::GlobalLogger::Create("test_agent_proxy", "test_agent_proxy.log", true);
	log4cxx::NDC ndc("test_agent_proxy");
	ORWELL_LOG_INFO("Test starts\n");
	{
		orwell::Application & aApplication = orwell::Application::GetInstance();

		Arguments aArguments = Common::GetArguments(
				false, 9001, 9000, 9003,
				boost::none, boost::none, 500, 300,
				false, true, true, true);
		orwell::Application::Parameters aParameters;
		orwell::Application::ReadParameters(
				aArguments.m_argc,
				aArguments.m_argv,
				aParameters);
		aApplication.run(aParameters);
		//usleep(40 * 1000); // sleep for 0.040 s
		test_1(aApplication);
		assert(gOK);
	}
	orwell::support::GlobalLogger::Clear();
	return 0;
}
