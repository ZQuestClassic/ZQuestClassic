#include "base/zdefs.h"
#include "zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/scripting/script_object.h"
#include "zc/websocket_pool.h"
#include <optional>

extern refInfo *ri;

#define MAX_USER_WEBSOCKETS 3

struct user_websocket : public user_abstract_obj
{
	~user_websocket()
	{
		if (connection_id != -1)
			websocket_pool_close(connection_id);
		if (message_arrayptr)
			FFScript::deallocateArray(message_arrayptr);
	}

	bool connect(std::string url)
	{
		connection_id = websocket_pool_connect(url, err);
		this->url = url;
		return connection_id != -1;
	}

	void send(WebSocketMessageType type, std::string message)
	{
		if (connection_id == -1 || type == WebSocketMessageType::None) return;
		websocket_pool_send(connection_id, type, message);
	}

	bool has_message()
	{
		if (connection_id == -1) return false;
		return websocket_pool_has_message(connection_id);
	}

	std::string receive_message()
	{
		if (connection_id == -1) return "";
		auto [type, message] = websocket_pool_receive(connection_id);
		last_message_type = type;
		return message;
	}

	WebSocketStatus get_state() const
	{
		if (connection_id == -1) return WebSocketStatus::Closed;
		return websocket_pool_status(connection_id);
	}

	std::string get_error() const
	{
		if (connection_id == -1) return err;
		return websocket_pool_error(connection_id);
	}

	int connection_id = -1;
	std::string url;
	std::string err;
	int message_arrayptr;
	WebSocketMessageType last_message_type;
};

static UserDataContainer<user_websocket, MAX_USER_WEBSOCKETS> user_websockets = {script_object_type::websocket, "websocket"};

void websocket_init()
{
	user_websockets.clear();
	websocket_pool_destroy();
}

std::optional<int32_t> websocket_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case WEBSOCKET_STATE:
		{
			ret = 0;
			auto ws = user_websockets.check(ri->websocketref);
			if (!ws) break;

			ret = (int)ws->get_state();
			break;
		}
		case WEBSOCKET_HAS_MESSAGE:
		{
			ret = 0;
			auto ws = user_websockets.check(ri->websocketref);
			if (!ws) break;

			ret = ws->has_message() * 10000;
			break;
		}
		case WEBSOCKET_MESSAGE_TYPE:
		{
			ret = 0;
			auto ws = user_websockets.check(ri->websocketref);
			if (!ws) break;

			ret = (int)ws->last_message_type;
			break;
		}
		default: return std::nullopt;
	}

	return ret;
}

std::optional<int32_t> websocket_run_command(word command)
{
	extern int32_t sarg1;
	extern int32_t sarg2;
	extern int32_t sarg3;
	extern ScriptType curScriptType;
	extern word curScriptNum;
	extern int32_t curScriptIndex;

	ScriptType type = curScriptType;
	int32_t i = curScriptIndex;

	switch (command)
	{
		case WEBSOCKET_OWN:
		{
			if (auto ws = user_websockets.check(ri->websocketref))
			{
				own_script_object(ws, type, i);
			}
			break;
		}
		case WEBSOCKET_LOAD:
		{
			int arrayptr = SH::get_arg(sarg1, false) / 10000;
			std::string url;
			ArrayH::getString(arrayptr, url, 512);

			ri->websocketref = 0;
			ri->d[rEXP1] = 0;

			if (url.size() == 0 || !url.starts_with("ws"))
			{
				break;
			}

			auto ws = user_websockets.create();
			if (!ws)
			{
				break;
			}

			ws->connect(url);

			ri->websocketref = ws->id;
			ri->d[rEXP1] = ws->id;
			break;
		}
		case WEBSOCKET_FREE:
		{
			if (auto ws = user_websockets.check(ri->websocketref, true))
			{
				free_script_object(ws->id);
			}
			break;
		}
		case WEBSOCKET_ERROR:
		{
			int32_t arrayptr = get_register(sarg1) / 10000;
			if (auto ws = user_websockets.check(ri->websocketref))
			{
				ArrayH::setArray(arrayptr, ws->get_error(), true);
			}
			else
			{
				ArrayH::setArray(arrayptr, "Invalid pointer", true);
			}
			break;
		}
		case WEBSOCKET_SEND:
		{
			int32_t type = get_register(sarg1);
			int32_t arrayptr = get_register(sarg2) / 10000;

			current_zasm_extra_context = "WebsocketType";
			if (BC::checkBounds(type, 1, 2) != SH::_NoError)
				break;
			current_zasm_extra_context = "";

			std::string message;
			ArrayH::getString(arrayptr, message);
			if (auto ws = user_websockets.check(ri->websocketref))
			{
				ws->send((WebSocketMessageType)type, message);
			}
			break;
		}
		case WEBSOCKET_RECEIVE:
		{
			if (auto ws = user_websockets.check(ri->websocketref))
			{
				if (!ws->has_message())
				{
					set_register(sarg1, 0);
					break;
				}
				std::string message = ws->receive_message();
				auto message_type = ws->last_message_type;

				if(!(ws->message_arrayptr && is_valid_array(ws->message_arrayptr)))
					ws->message_arrayptr = allocatemem(message.size() + 1, true, ScriptType::None, -1);

				if (message_type == WebSocketMessageType::Text)
					ArrayH::setArray(ws->message_arrayptr, message, true);
				else
					ArrayH::setArray(ws->message_arrayptr, message.size(), message.data(), false, true);

				set_register(sarg1, ws->message_arrayptr * 10000);
			}
			break;
		}
		default: return std::nullopt;
	}

	return RUNSCRIPT_OK;
}
