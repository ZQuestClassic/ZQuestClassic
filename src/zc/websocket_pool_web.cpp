#include "zc/websocket_pool.h"
#include "base/util.h"

#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>
#include <fmt/format.h>

#include <map>
#include <queue>
#include <string>
#include <utility>

void scripting_log_error_with_context(std::string text);

template <typename... Args>
static void scripting_log_error_with_context(fmt::format_string<Args...> s, Args&&... args)
{
	std::string text = fmt::format(s, std::forward<Args>(args)...);
	scripting_log_error_with_context(text);
}

struct websocket_endpoint
{
	WebSocketStatus status;
	std::string error;
	std::queue<std::pair<WebSocketMessageType, std::string>> messages;
};

static std::map<int, websocket_endpoint> endpoints;

static EM_BOOL onopen(int eventType, const EmscriptenWebSocketOpenEvent *websocketEvent, void *userData)
{
	auto* endpoint = (websocket_endpoint*)userData;
	endpoint->status = WebSocketStatus::Open;
	return EM_TRUE;
}

static EM_BOOL onerror(int eventType, const EmscriptenWebSocketErrorEvent *websocketEvent, void *userData)
{
	auto* endpoint = (websocket_endpoint*)userData;
	endpoint->error = "Unknown error";
	return EM_TRUE;
}

static EM_BOOL onclose(int eventType, const EmscriptenWebSocketCloseEvent *websocketEvent, void *userData)
{
	auto* endpoint = (websocket_endpoint*)userData;
	endpoint->status = WebSocketStatus::Closed;
	return EM_TRUE;
}

static EM_BOOL onmessage(int eventType, const EmscriptenWebSocketMessageEvent *websocketEvent, void *userData)
{
	auto* endpoint = (websocket_endpoint*)userData;
	std::string str(reinterpret_cast<const char*>(websocketEvent->data), websocketEvent->numBytes);
	if (websocketEvent->isText)
		endpoint->messages.emplace(WebSocketMessageType::Text, str);
	else
		endpoint->messages.emplace(WebSocketMessageType::Binary, str);
	return EM_TRUE;
}

int websocket_pool_connect(std::string url, std::string& err)
{
	if (!emscripten_websocket_is_supported())
	{
		err = "WebSockets not supported in this browser";
		return -1;
	}

	EmscriptenWebSocketCreateAttributes ws_attrs = {
		url.c_str(),
		NULL, // protocols
		EM_TRUE // createOnMainThread
	};

	EMSCRIPTEN_WEBSOCKET_T ws = emscripten_websocket_new(&ws_attrs);

	if (ws <= 0)
	{
		err = fmt::format("emscripten error: {}", ws);
		return -1;
	}

	auto& endpoint = endpoints[ws] = {};
	emscripten_websocket_set_onopen_callback(ws, &endpoint, onopen);
	emscripten_websocket_set_onerror_callback(ws, &endpoint, onerror);
	emscripten_websocket_set_onclose_callback(ws, &endpoint, onclose);
	emscripten_websocket_set_onmessage_callback(ws, &endpoint, onmessage);

	return ws;
}

void websocket_pool_send(int connection_id, WebSocketMessageType type, const std::string& message)
{
	EMSCRIPTEN_RESULT result;
	if (type == WebSocketMessageType::Text)
		result = emscripten_websocket_send_utf8_text((EMSCRIPTEN_WEBSOCKET_T)connection_id, message.c_str());
	else
		result = emscripten_websocket_send_binary((EMSCRIPTEN_WEBSOCKET_T)connection_id, (void *)message.data(), message.size());

	if (result < 0)
		scripting_log_error_with_context("emscripten error {}", result);
}

bool websocket_pool_has_message(int connection_id)
{
	if (auto endpoint = util::find(endpoints, connection_id))
		return !endpoint->messages.empty();

	scripting_log_error_with_context("No connection found with id {}", connection_id);
	return false;
}

WebSocketMessageType websocket_pool_message_type(int connection_id)
{
	if (auto endpoint = util::find(endpoints, connection_id))
		return endpoint->messages.front().first;

	scripting_log_error_with_context("No connection found with id {}", connection_id);
	return WebSocketMessageType::None;
}

std::pair<WebSocketMessageType, std::string> websocket_pool_receive(int connection_id)
{
	if (auto endpoint = util::find(endpoints, connection_id))
	{
		auto result = endpoint->messages.front();
		endpoint->messages.pop();
		return result;
	}

	scripting_log_error_with_context("No connection found with id {}", connection_id);
	return {};
}

WebSocketStatus websocket_pool_status(int connection_id)
{
	if (auto endpoint = util::find(endpoints, connection_id))
		return endpoint->status;

	scripting_log_error_with_context("No connection found with id {}", connection_id);
	return WebSocketStatus::Connecting;
}

std::string websocket_pool_error(int connection_id)
{
	if (auto endpoint = util::find(endpoints, connection_id))
		return endpoint->error;

	scripting_log_error_with_context("No connection found with id {}", connection_id);
}

void websocket_pool_close(int connection_id)
{
	if (auto endpoint = util::find(endpoints, connection_id))
	{
		endpoint->status = WebSocketStatus::Closing;

		EMSCRIPTEN_RESULT result = emscripten_websocket_close((EMSCRIPTEN_WEBSOCKET_T)connection_id, 1000, "");
		if (result < 0)
		{
			scripting_log_error_with_context("emscripten error {}", result);
			return;
		}

		result = emscripten_websocket_delete((EMSCRIPTEN_WEBSOCKET_T)connection_id);
		if (result < 0)
		{
			scripting_log_error_with_context("emscripten error {}", result);
			return;
		}

		return;
	}

	scripting_log_error_with_context("No connection found with id {}", connection_id);
}

void websocket_pool_destroy()
{
	for (auto& it : endpoints)
	{
		if (it.second.status == WebSocketStatus::Connecting || it.second.status == WebSocketStatus::Open)
			emscripten_websocket_close((EMSCRIPTEN_WEBSOCKET_T)it.first, 1000, "");
		emscripten_websocket_delete((EMSCRIPTEN_WEBSOCKET_T)it.first);
	}

	endpoints.clear();
}
