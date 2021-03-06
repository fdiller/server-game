
#pragma once

#include <string>
#include <stdint.h>

#include "orwell/IAgentProxy.hpp"

namespace orwell
{
class Application;

/// Class that gives access to commands to be run from an agent.
class AgentProxy : public IAgentProxy
{
public :
	/// \param ioApplication
	///  The application the agent will control.
	AgentProxy(orwell::Application & ioApplication);

	/// \return
	///  True if and only if the command was successfully parsed.
	bool step(
			std::string const & iCommand,
			std::string & ioReply) override;

	/// stop application
	void stopApplication() override;

	/// List all the robots present.
	///
	void listRobot(std::string & ioReply) override;

	/// add robot <name>
	void addRobot(std::string const & iRobotName) override;

	/// remove robot <name>
	void removeRobot(std::string const & iRobotName) override;

	/// register robot <name>
	void registerRobot(std::string const & iRobotName) override;

	/// unregister robot <name>
	void unregisterRobot(std::string const & iRobotName) override;

	/// set property <property> of robot <name>
	void setRobot(
			std::string const & iRobotName,
			std::string const & iProperty,
			std::string const & iValue) override;

	/// get property <property> of robot <name>
	void getRobot(
			std::string const & iRobotName,
			std::string const & iProperty,
			std::string & oValue) override;

	/// List all the players present.
	///
	void listPlayer(std::string & ioReply) override;

	/// add player <name>
	void addPlayer(std::string const & iPlayerName) override;

	/// remove player <name>
	void removePlayer(std::string const & iPlayerName) override;

	/// start game
	void startGame() override;

	/// stop game
	void stopGame() override;
protected :

private :
	AgentProxy(AgentProxy const & iRight) = delete;
	AgentProxy & operator=(AgentProxy const & iRight) = delete;

	orwell::Application & m_application;
};
}

