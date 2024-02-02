#include "websocket_pool.h"

#include <string>

int websocket_pool_connect(std::string url, std::string& err)
{
    err = "Not Implemented";
    return -1;
}

void websocket_pool_send(int connection_id, WebSocketMessageType type, std::string message)
{
}

bool websocket_pool_has_message(int connection_id)
{
    return false;
}

WebSocketMessageType websocket_pool_message_type(int connection_id)
{
	return WebSocketMessageType::None;
}

std::pair<WebSocketMessageType, std::string> websocket_pool_receive(int connection_id)
{
    return {};
}

WebSocketStatus websocket_pool_status(int connection_id)
{
    return WebSocketStatus::Closed;
}

std::string websocket_pool_error(int connection_id)
{
    return "Not Implemented";
}

void websocket_pool_close(int connection_id)
{
}

void websocket_pool_destroy()
{
}
