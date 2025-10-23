#ifndef ZC_SCRIPTING_WEBSOCKET_H_
#define ZC_SCRIPTING_WEBSOCKET_H_

#include "../../user_object.h"
#include "base/ints.h"
#include "zc/websocket_pool.h"
#include <optional>

struct user_websocket : public user_abstract_obj
{
	~user_websocket();

	void get_retained_ids(std::vector<uint32_t>& ids);
	bool connect(std::string url);
	void send(WebSocketMessageType type, std::string message);
	bool has_message();
	std::string receive_message();
	WebSocketStatus get_state() const;
	std::string get_error() const;

	int connection_id = -1;
	std::string url;
	std::string err;
	uint32_t message_array_id;
	WebSocketMessageType last_message_type;
};

user_websocket* checkWebsocket(int32_t ref, bool skipError = false);

void websocket_init();
std::optional<int32_t> websocket_get_register(int32_t reg);
std::optional<int32_t> websocket_run_command(word command);

#endif
