#pragma once

#include "orwell/callbacks/InterfaceProcess.hpp"
#include <map>
#include <memory>

namespace orwell {

namespace game {
class Game;
} // namespace game

namespace com
{
class RawMessage;
} // namespace com

namespace callbacks {

class ProcessDecider
{
	typedef std::pair<std::string, std::unique_ptr<InterfaceProcess>> Couple;

public:
	ProcessDecider();
	~ProcessDecider();
	static void Process(
			com::RawMessage const & iMessage,
			game::Game & ioCtx);
	void process(
			com::RawMessage const & iMessage,
			game::Game & ioCtx);

private:
	std::map<std::string, std::unique_ptr<InterfaceProcess>> _map;
};

} // namespace callbacks
} // namespace orwell

