#include <string>

enum class WebSocketStatus
{
    Connecting,
    Open,
    Closing,
    Closed,
};

enum class WebSocketMessageType
{
    None,
    Text,
    Binary,
};

int websocket_pool_connect(std::string url, std::string& err);
void websocket_pool_send(int connection_id, WebSocketMessageType type, std::string message);
bool websocket_pool_has_message(int connection_id);
std::pair<WebSocketMessageType, std::string> websocket_pool_receive(int connection_id);
WebSocketStatus websocket_pool_status(int connection_id);
std::string websocket_pool_error(int connection_id);
void websocket_pool_close(int connection_id);
void websocket_pool_destroy();
