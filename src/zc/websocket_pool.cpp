#include "websocket_pool.h"
#include "websocketpp/frame.hpp"

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

#include <iostream>
#include <string>
#include <map>
#include <queue>

// Came from https://github.com/zaphoyd/websocketpp/blob/master/tutorials/utility_client/step6.cpp
// w/ minor modifications

typedef websocketpp::client<websocketpp::config::asio_client> client;

class connection_metadata {
public:
    typedef websocketpp::lib::shared_ptr<connection_metadata> ptr;

    connection_metadata(int id, websocketpp::connection_hdl hdl, std::string uri)
      : m_id(id)
      , m_hdl(hdl)
      , m_status("Connecting")
      , m_uri(uri)
      , m_server("N/A")
    {}

    void on_open(client * c, websocketpp::connection_hdl hdl) {
        m_status = "Open";

        client::connection_ptr con = c->get_con_from_hdl(hdl);
        m_server = con->get_response_header("Server");
    }

    void on_fail(client * c, websocketpp::connection_hdl hdl) {
        m_status = "Failed";

        client::connection_ptr con = c->get_con_from_hdl(hdl);
        m_server = con->get_response_header("Server");
        m_error_reason = con->get_ec().message();
    }
    
    void on_close(client * c, websocketpp::connection_hdl hdl) {
        m_status = "Closed";
        client::connection_ptr con = c->get_con_from_hdl(hdl);
        std::stringstream s;
        s << "close code: " << con->get_remote_close_code() << " (" 
          << websocketpp::close::status::get_string(con->get_remote_close_code()) 
          << "), close reason: " << con->get_remote_close_reason();
        m_error_reason = s.str();
    }

    void on_message(websocketpp::connection_hdl, client::message_ptr msg) {
		m_messages.emplace(msg->get_opcode(), msg->get_payload());
    }

    websocketpp::connection_hdl get_hdl() const {
        return m_hdl;
    }
    
    int get_id() const {
        return m_id;
    }
    
    std::string get_status() const {
        return m_status;
    }

    std::string get_error() const {
        return m_error_reason;
    }

    friend std::ostream & operator<< (std::ostream & out, connection_metadata const & data);

    std::queue<std::pair<websocketpp::frame::opcode::value, std::string>> m_messages;
    std::string m_status;
private:
    int m_id;
    websocketpp::connection_hdl m_hdl;
    std::string m_uri;
    std::string m_server;
    std::string m_error_reason;
};
 
class websocket_endpoint {
public:
    websocket_endpoint () : m_next_id(0) {
        m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
        m_endpoint.clear_error_channels(websocketpp::log::elevel::all);

        m_endpoint.init_asio();
        m_endpoint.start_perpetual();

        m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client::run, &m_endpoint);
    }

    ~websocket_endpoint() {
        m_endpoint.stop_perpetual();
        
        for (con_list::const_iterator it = m_connection_list.begin(); it != m_connection_list.end(); ++it) {
            if (it->second->get_status() != "Open") {
                // Only close open connections
                continue;
            }
            
            std::cout << "> Closing connection " << it->second->get_id() << std::endl;
            
            websocketpp::lib::error_code ec;
            m_endpoint.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", ec);
            if (ec) {
                std::cout << "> Error closing connection " << it->second->get_id() << ": "  
                          << ec.message() << std::endl;
            }
        }
        
        m_thread->join();
    }

    int connect(std::string const & uri, std::string& err) {
        websocketpp::lib::error_code ec;

        client::connection_ptr con = m_endpoint.get_connection(uri, ec);

        if (ec) {
            err = ec.message();
            return -1;
        }

        int new_id = m_next_id++;
        connection_metadata::ptr metadata_ptr = websocketpp::lib::make_shared<connection_metadata>(new_id, con->get_handle(), uri);
        m_connection_list[new_id] = metadata_ptr;

        con->set_open_handler(websocketpp::lib::bind(
            &connection_metadata::on_open,
            metadata_ptr,
            &m_endpoint,
            websocketpp::lib::placeholders::_1
        ));
        con->set_fail_handler(websocketpp::lib::bind(
            &connection_metadata::on_fail,
            metadata_ptr,
            &m_endpoint,
            websocketpp::lib::placeholders::_1
        ));
        con->set_close_handler(websocketpp::lib::bind(
            &connection_metadata::on_close,
            metadata_ptr,
            &m_endpoint,
            websocketpp::lib::placeholders::_1
        ));
        con->set_message_handler(websocketpp::lib::bind(
            &connection_metadata::on_message,
            metadata_ptr,
            websocketpp::lib::placeholders::_1,
            websocketpp::lib::placeholders::_2
        ));

        m_endpoint.connect(con);

        return new_id;
    }

    void close(int id, websocketpp::close::status::value code, std::string reason) {
        websocketpp::lib::error_code ec;
        
        auto metadata_it = m_connection_list.find(id);
        if (metadata_it == m_connection_list.end()) {
            std::cout << "> No connection found with id " << id << std::endl;
            return;
        }
        
        metadata_it->second->m_status = "Closing";
        m_endpoint.close(metadata_it->second->get_hdl(), code, reason, ec);
        if (ec) {
            std::cout << "> Error initiating close: " << ec.message() << std::endl;
        }
    }

    void send(int id, std::string message, websocketpp::frame::opcode::value op) {
        websocketpp::lib::error_code ec;
        
        auto metadata_it = m_connection_list.find(id);
        if (metadata_it == m_connection_list.end()) {
            std::cout << "> No connection found with id " << id << std::endl;
            return;
        }
        
        m_endpoint.send(metadata_it->second->get_hdl(), message, op, ec);
        if (ec) {
            std::cout << "> Error sending message: " << ec.message() << std::endl;
            return;
        }
    }

    connection_metadata::ptr get_metadata(int id) const {
        con_list::const_iterator metadata_it = m_connection_list.find(id);
        if (metadata_it == m_connection_list.end()) {
            return connection_metadata::ptr();
        } else {
            return metadata_it->second;
        }
    }
private:
    typedef std::map<int,connection_metadata::ptr> con_list;

    client m_endpoint;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

    con_list m_connection_list;
    int m_next_id;
};

// end copy/pasted code

static websocket_endpoint* pool;

int websocket_pool_connect(std::string url, std::string& err)
{
    if (!pool)
        pool = new websocket_endpoint();

    return pool->connect(url, err);
}

void websocket_pool_send(int connection_id, WebSocketMessageType type, std::string message)
{
    assert(pool);
    pool->send(connection_id, message, (websocketpp::frame::opcode::value)type);
}

bool websocket_pool_has_message(int connection_id)
{
    assert(pool);
    return !pool->get_metadata(connection_id)->m_messages.empty();
}

WebSocketMessageType websocket_pool_message_type(int connection_id)
{
	assert(pool);
    auto& messages = pool->get_metadata(connection_id)->m_messages;
	if (!messages.empty())
	{
		if (messages.back().first == websocketpp::frame::opcode::text)
			return WebSocketMessageType::Text;
		if (messages.back().first == websocketpp::frame::opcode::binary)
			return WebSocketMessageType::Binary;
	}
	return WebSocketMessageType::None;
}

std::pair<WebSocketMessageType, std::string> websocket_pool_receive(int connection_id)
{
    assert(pool);
    auto& messages = pool->get_metadata(connection_id)->m_messages;
    auto result = messages.front();
    messages.pop();

	WebSocketMessageType type = WebSocketMessageType::None;
	auto op = result.first;
	if (op == websocketpp::frame::opcode::text)
		type = WebSocketMessageType::Text;
	if (op == websocketpp::frame::opcode::binary)
		type = WebSocketMessageType::Binary;
    return {type, result.second};
}

WebSocketStatus websocket_pool_status(int connection_id)
{
    assert(pool);
    std::string status = pool->get_metadata(connection_id)->get_status();

    if (status == "Open") return WebSocketStatus::Open;
    if (status == "Failed") return WebSocketStatus::Closed;
    if (status == "Closing") return WebSocketStatus::Closing;
    if (status == "Closed") return WebSocketStatus::Closed;
    if (status == "Connecting") return WebSocketStatus::Connecting;
    return WebSocketStatus::Connecting;
}

std::string websocket_pool_error(int connection_id)
{
    assert(pool);
    return pool->get_metadata(connection_id)->get_error();
}

void websocket_pool_close(int connection_id)
{
    pool->close(connection_id, websocketpp::close::status::normal, "");
}

void websocket_pool_destroy()
{
    if (pool)
    {
        delete pool;
        pool = nullptr;
    }
}
