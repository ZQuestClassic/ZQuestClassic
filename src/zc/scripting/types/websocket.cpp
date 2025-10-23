#include "zc/scripting/types/websocket.h"
#include "base/zdefs.h"
#include "zasm/defines.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"
#include "zc/scripting/script_object.h"
#include "zc/websocket_pool.h"
#include "zscriptversion.h"
#include <optional>

extern refInfo *ri;

#define MAX_USER_WEBSOCKETS 3

user_websocket::~user_websocket()
{
	if (connection_id != -1)
		websocket_pool_close(connection_id);
	if (message_array_id)
	{
		if (ZScriptVersion::gc_arrays())
			script_object_autorelease_pool.push_back(message_array_id);
		else
			FFScript::deallocateArray(message_array_id);
	}
}

void user_websocket::get_retained_ids(std::vector<uint32_t>& ids)
{
	if (ZScriptVersion::gc_arrays())
		ids.push_back(message_array_id);
}

bool user_websocket::connect(std::string url)
{
	connection_id = websocket_pool_connect(url, err);
	this->url = url;
	return connection_id != -1;
}

void user_websocket::send(WebSocketMessageType type, std::string message)
{
	if (connection_id == -1 || type == WebSocketMessageType::None) return;
	websocket_pool_send(connection_id, type, message);
}

bool user_websocket::has_message()
{
	if (connection_id == -1) return false;
	return websocket_pool_has_message(connection_id);
}

std::string user_websocket::receive_message()
{
	if (connection_id == -1) return "";
	auto [type, message] = websocket_pool_receive(connection_id);
	last_message_type = type;
	return message;
}

WebSocketStatus user_websocket::get_state() const
{
	if (connection_id == -1) return WebSocketStatus::Closed;
	return websocket_pool_status(connection_id);
}

std::string user_websocket::get_error() const
{
	if (connection_id == -1) return err;
	return websocket_pool_error(connection_id);
}

static UserDataContainer<user_websocket, MAX_USER_WEBSOCKETS> user_websockets = {script_object_type::websocket, "websocket"};

user_websocket* checkWebsocket(int32_t ref, bool skipError)
{
	return user_websockets.check(ref, skipError);
}

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
			auto ws = user_websockets.check(GET_REF(websocketref));
			if (!ws) break;

			ret = (int)ws->get_state();
			break;
		}
		case WEBSOCKET_HAS_MESSAGE:
		{
			auto ws = user_websockets.check(GET_REF(websocketref));
			if (!ws) break;

			ret = ws->has_message() * 10000;
			break;
		}
		case WEBSOCKET_MESSAGE_TYPE:
		{
			auto ws = user_websockets.check(GET_REF(websocketref));
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
			if (auto ws = user_websockets.check(GET_REF(websocketref)))
			{
				own_script_object(ws, type, i);
			}
			break;
		}
		case WEBSOCKET_LOAD:
		{
			int arrayptr = SH::get_arg(sarg1, false);
			std::string url;
			ArrayH::getString(arrayptr, url, 512);

			ri->websocketref = 0;
			SET_D(rEXP1, 0);

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
			SET_D(rEXP1, ws->id);
			break;
		}
		case WEBSOCKET_FREE:
		{
			if (auto ws = user_websockets.check(GET_REF(websocketref), true))
			{
				free_script_object(ws->id);
			}
			break;
		}
		case WEBSOCKET_ERROR:
		{
			int32_t arrayptr = get_register(sarg1);
			if (auto ws = user_websockets.check(GET_REF(websocketref)))
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
			int32_t arrayptr = get_register(sarg2);

			current_zasm_extra_context = "WebsocketType";
			if (BC::checkBounds(type, 1, 2) != SH::_NoError)
				break;
			current_zasm_extra_context = "";

			std::string message;
			ArrayH::getString(arrayptr, message);
			if (auto ws = user_websockets.check(GET_REF(websocketref)))
			{
				ws->send((WebSocketMessageType)type, message);
			}
			break;
		}
		case WEBSOCKET_RECEIVE:
		{
			if (auto ws = user_websockets.check(GET_REF(websocketref)))
			{
				if (!ws->has_message())
				{
					set_register(sarg1, 0);
					break;
				}
				std::string message = ws->receive_message();
				auto message_type = ws->last_message_type;

				if(!(ws->message_array_id && is_valid_array(ws->message_array_id)))
				{
					ws->message_array_id = allocatemem(message.size() + 1, true, ScriptType::None, -1);
					if (ZScriptVersion::gc_arrays())
						script_object_ref_inc(ws->message_array_id);
				}

				if (message_type == WebSocketMessageType::Text)
					ArrayH::setArray(ws->message_array_id, message, true);
				else
					ArrayH::setArray(ws->message_array_id, message.size(), message.data(), false, true);

				if (ZScriptVersion::gc_arrays())
					set_register(sarg1, ws->message_array_id);
				else
					set_register(sarg1, ws->message_array_id * 10000);
			}
			break;
		}
		default: return std::nullopt;
	}

	return RUNSCRIPT_OK;
}

static ArrayRegistrar WEBSOCKET_URL_registrar(WEBSOCKET_URL, []{
	static ScriptingArray_ObjectComputed<user_websocket, char> impl(
		[](user_websocket* ws){ return ws->url.size() + 1; },
		[](user_websocket* ws, int index) -> char { return ws->url[index]; },
		[](user_websocket* ws, int index, char value){}
	);
	impl.setMul10000(true);
	impl.setReadOnly();
	return &impl;
}());
