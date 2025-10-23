#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtype-limits"
#endif

#ifdef HAS_SSL
#include <websocketpp/config/asio_client.hpp>
#else
#include <websocketpp/config/asio_no_tls_client.hpp>
#endif

#include "zc/websocket_pool.h"

#include <websocketpp/frame.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>
#include <fmt/format.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <string>
#include <map>
#include <queue>

void scripting_log_error_with_context(std::string text);

template <typename... Args>
static void scripting_log_error_with_context(fmt::format_string<Args...> s, Args&&... args)
{
	std::string text = fmt::format(s, std::forward<Args>(args)...);
	scripting_log_error_with_context(text);
}

// Came from https://github.com/zaphoyd/websocketpp/blob/master/tutorials/utility_client/step6.cpp
// and https://github.com/barsnick/websocketpp/commit/3375a500cdee13ea8af271dd099e069c3476e9e0
// w/ minor modifications

#ifdef HAS_SSL
using client_tls = websocketpp::client<websocketpp::config::asio_tls_client>;
using context_ptr = websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context>;
using connection_ptr_tls = client_tls::connection_ptr;
using connection_type_tls = client_tls::connection_type;
using websocketpp::lib::placeholders::_1;
#endif

using client_no_tls = websocketpp::client<websocketpp::config::asio_client>;
using connection_ptr = client_no_tls::connection_ptr;
using connection_type = client_no_tls::connection_type;

using websocketpp::lib::bind;

static int next_connection_id;

template <typename T1>
class connection_metadata {
public:
	using ptr = websocketpp::lib::shared_ptr<connection_metadata>;
	using message_queue = std::queue<std::pair<websocketpp::frame::opcode::value, std::string>>;

	connection_metadata(int id, websocketpp::connection_hdl hdl, std::string uri)
		: m_id(id)
		, m_hdl(hdl)
		, m_status("Connecting")
		, m_uri(uri)
		, m_server("N/A")
	{}

	void on_open(T1 * c, websocketpp::connection_hdl hdl) {
		m_status = "Open";

		auto con = c->get_con_from_hdl(hdl);
		m_server = con->get_response_header("Server");
	}

	void on_fail(T1 * c, websocketpp::connection_hdl hdl) {
		m_status = "Failed";

		auto con = c->get_con_from_hdl(hdl);
		m_server = con->get_response_header("Server");
		m_error_reason = con->get_ec().message();
	}
	
	void on_close(T1 * c, websocketpp::connection_hdl hdl) {
		m_status = "Closed";
		auto con = c->get_con_from_hdl(hdl);
		std::stringstream s;
		s << "close code: " << con->get_remote_close_code() << " (" 
		<< websocketpp::close::status::get_string(con->get_remote_close_code()) 
		<< "), close reason: " << con->get_remote_close_reason();
		m_error_reason = s.str();
	}

	void on_message(websocketpp::connection_hdl, T1::message_ptr msg) {
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

	message_queue m_messages;
	std::string m_status;
private:
	int m_id;
	websocketpp::connection_hdl m_hdl;
	std::string m_uri;
	std::string m_server;
	std::string m_error_reason;
};

#ifdef HAS_SSL
static context_ptr on_tls_init(const char * hostname, websocketpp::connection_hdl)
{
	context_ptr ctx = websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);

	try {
		ctx->set_options(asio::ssl::context::default_workarounds |
						asio::ssl::context::no_sslv2 |
						asio::ssl::context::no_sslv3 |
						asio::ssl::context::single_dh_use);

		// https://think-async.com/Asio/asio-1.13.0/doc/asio/overview/ssl.html
		using asio::ip::tcp;
		namespace ssl = asio::ssl;
		using ssl_socket = ssl::stream<tcp::socket>;

		// Create a context that uses the default paths for
		// finding CA certificates.
		ssl::context ctx(ssl::context::sslv23);
		ctx.set_default_verify_paths();

		// Open a socket and connect it to the remote host.
		asio::io_context io_context;
		ssl_socket sock(io_context, ctx);
		tcp::resolver resolver(io_context);
		tcp::resolver::query query(hostname, "https");
		asio::connect(sock.lowest_layer(), resolver.resolve(query));
		sock.lowest_layer().set_option(tcp::no_delay(true));

		// Perform SSL handshake and verify the remote host's
		// certificate.
		sock.set_verify_mode(ssl::verify_peer);
		sock.set_verify_callback(ssl::rfc2818_verification(hostname));
		sock.handshake(ssl_socket::client);
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	return ctx;
}
#endif

template <typename T1>
class websocket_endpoint final {
public:
	websocket_endpoint () {
		m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
		m_endpoint.clear_error_channels(websocketpp::log::elevel::all);

		m_endpoint.init_asio();
		m_endpoint.start_perpetual();

		m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&T1::run, &m_endpoint);
	}

	~websocket_endpoint() {
		m_endpoint.stop_perpetual();
		
		for (typename con_list::const_iterator it = m_connection_list.begin(); it != m_connection_list.end(); ++it) {
			if (it->second->get_status() != "Open") {
				// Only close open connections
				continue;
			}
			
			std::cout << "> Closing connection " << it->second->get_id() << std::endl;
			
			websocketpp::lib::error_code ec;
			m_endpoint.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", ec);
			if (ec) {
				scripting_log_error_with_context("Error closing connection: {}", ec.message());
			}
		}
		
		m_thread->join();
	}

	virtual void secure_handler(std::string host);

	int connect(std::string const & uri, int new_id, std::string& err) {
		websocketpp::lib::error_code ec;

		websocketpp::uri_ptr location = websocketpp::lib::make_shared<websocketpp::uri>(uri);
		if (!location->get_valid())
		{
			err = "Invalid uri";
			return -1;
		}

		secure_handler(location->get_host());

		auto con = m_endpoint.get_connection(location, ec);
		if (ec) {
			err = ec.message();
			return -1;
		}

		auto metadata_ptr = websocketpp::lib::make_shared<connection_metadata<T1>>(new_id, con->get_handle(), uri);
		m_connection_list[new_id] = metadata_ptr;

		con->set_open_handler(websocketpp::lib::bind(
			&connection_metadata<T1>::on_open,
			metadata_ptr,
			&m_endpoint,
			websocketpp::lib::placeholders::_1
		));
		con->set_fail_handler(websocketpp::lib::bind(
			&connection_metadata<T1>::on_fail,
			metadata_ptr,
			&m_endpoint,
			websocketpp::lib::placeholders::_1
		));
		con->set_close_handler(websocketpp::lib::bind(
			&connection_metadata<T1>::on_close,
			metadata_ptr,
			&m_endpoint,
			websocketpp::lib::placeholders::_1
		));
		con->set_message_handler(websocketpp::lib::bind(
			&connection_metadata<T1>::on_message,
			metadata_ptr,
			websocketpp::lib::placeholders::_1,
			websocketpp::lib::placeholders::_2
		));

		m_endpoint.connect(con);

		return new_id;
	}

	connection_metadata<T1>::ptr find_connection(int id)
	{
		auto metadata_it = m_connection_list.find(id);
		return metadata_it != m_connection_list.end() ? metadata_it->second : nullptr;
	}

	void close(connection_metadata<T1>::ptr connection, websocketpp::close::status::value code, std::string reason) {
		websocketpp::lib::error_code ec;
		
		connection->m_status = "Closing";
		m_endpoint.close(connection->get_hdl(), code, reason, ec);
		if (ec) {
			scripting_log_error_with_context("Error initiating close: {}", ec.message());
		}
	}

	void send(connection_metadata<T1>::ptr connection, std::string message, websocketpp::frame::opcode::value op) {
		websocketpp::lib::error_code ec;

		m_endpoint.send(connection->get_hdl(), message, op, ec);
		if (ec) {
			scripting_log_error_with_context("Error sending message: {}", ec.message());
		}
	}
private:
	using con_list = std::map<int, typename connection_metadata<T1>::ptr>;

	T1 m_endpoint;
	websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

	con_list m_connection_list;
};

#ifdef HAS_SSL
template<>
void websocket_endpoint<client_tls>::secure_handler(std::string host)
{
	m_endpoint.set_tls_init_handler(bind(&on_tls_init, host.c_str(), ::_1));
}
#endif

template<>
void websocket_endpoint<client_no_tls>::secure_handler(std::string host)
{
}

// end copy/pasted code

#ifdef HAS_SSL
using websocket_endpoint_tls = websocket_endpoint<client_tls>;
static websocket_endpoint_tls* pool_tls;
#endif

using websocket_endpoint_no_tls = websocket_endpoint<client_no_tls>;
static websocket_endpoint_no_tls* pool_no_tls;

int websocket_pool_connect(std::string url, std::string& err)
{
	websocketpp::uri_ptr location = websocketpp::lib::make_shared<websocketpp::uri>(url);
	if (!location->get_valid())
	{
		err = "Invalid url";
		return -1;
	}

	if (location->get_secure())
	{
#ifdef HAS_SSL
		if (!pool_tls)
			pool_tls = new websocket_endpoint_tls();
		return pool_tls->connect(url, next_connection_id++, err);
#else
		err = "Cannot open secure connection (no SSL in this build)";
		return -1;
#endif
	}

	if (!pool_no_tls)
		pool_no_tls = new websocket_endpoint_no_tls();
	return pool_no_tls->connect(url, next_connection_id++, err);
}

void websocket_pool_send(int connection_id, WebSocketMessageType type, const std::string& message)
{
#ifdef HAS_SSL
	if (auto c = pool_tls ? pool_tls->find_connection(connection_id) : nullptr)
		return pool_tls->send(c, message, (websocketpp::frame::opcode::value)type);
#endif
	if (auto c = pool_no_tls ? pool_no_tls->find_connection(connection_id) : nullptr)
		return pool_no_tls->send(c, message, (websocketpp::frame::opcode::value)type);

	scripting_log_error_with_context("No connection found with id {}", connection_id);
}

bool websocket_pool_has_message(int connection_id)
{
#ifdef HAS_SSL
	if (auto c = pool_tls ? pool_tls->find_connection(connection_id) : nullptr)
		return !c->m_messages.empty();
#endif
	if (auto c = pool_no_tls ? pool_no_tls->find_connection(connection_id) : nullptr)
		return !c->m_messages.empty();

	scripting_log_error_with_context("No connection found with id {}", connection_id);
	return false;
}

WebSocketMessageType websocket_pool_message_type(int connection_id)
{
	connection_metadata<client_no_tls>::message_queue* messages = nullptr;

	if (false)
		;
#ifdef HAS_SSL
	else if (auto c = pool_tls ? pool_tls->find_connection(connection_id) : nullptr)
		messages = &c->m_messages;
#endif
	else if (auto c = pool_no_tls ? pool_no_tls->find_connection(connection_id) : nullptr)
		messages = &c->m_messages;
	else
	{
		scripting_log_error_with_context("No connection found with id {}", connection_id);
		return WebSocketMessageType::None;
	}

	if (!messages->empty())
	{
		if (messages->back().first == websocketpp::frame::opcode::text)
			return WebSocketMessageType::Text;
		if (messages->back().first == websocketpp::frame::opcode::binary)
			return WebSocketMessageType::Binary;
	}

	return WebSocketMessageType::None;
}

std::pair<WebSocketMessageType, std::string> websocket_pool_receive(int connection_id)
{
	connection_metadata<client_no_tls>::message_queue* messages = nullptr;
	if (false)
		;
#ifdef HAS_SSL
	if (auto c = pool_tls ? pool_tls->find_connection(connection_id) : nullptr)
		messages = &c->m_messages;
#endif
	else if (auto c = pool_no_tls ? pool_no_tls->find_connection(connection_id) : nullptr)
		messages = &c->m_messages;
	else
	{
		scripting_log_error_with_context("No connection found with id {}", connection_id);
		return {};
	}

	auto result = messages->front();
	messages->pop();

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
	std::string status;
	if (false)
		;
#ifdef HAS_SSL
	if (auto c = pool_tls ? pool_tls->find_connection(connection_id) : nullptr)
		status = c->get_status();
#endif
	else if (auto c = pool_no_tls ? pool_no_tls->find_connection(connection_id) : nullptr)
		status = c->get_status();
	else
	{
		scripting_log_error_with_context("No connection found with id {}", connection_id);
		return WebSocketStatus::Connecting;
	}

	if (status == "Open") return WebSocketStatus::Open;
	if (status == "Failed") return WebSocketStatus::Closed;
	if (status == "Closing") return WebSocketStatus::Closing;
	if (status == "Closed") return WebSocketStatus::Closed;
	if (status == "Connecting") return WebSocketStatus::Connecting;
	return WebSocketStatus::Connecting;
}

std::string websocket_pool_error(int connection_id)
{
#ifdef HAS_SSL
	if (auto c = pool_tls ? pool_tls->find_connection(connection_id) : nullptr)
		return c->get_error();
#endif
	if (auto c = pool_no_tls ? pool_no_tls->find_connection(connection_id) : nullptr)
		return c->get_error();

	scripting_log_error_with_context("No connection found with id {}", connection_id);
	return "";
}

void websocket_pool_close(int connection_id)
{
#ifdef HAS_SSL
	if (auto c = pool_tls ? pool_tls->find_connection(connection_id) : nullptr)
		return pool_tls->close(c, websocketpp::close::status::normal, "");
#endif
	if (auto c = pool_no_tls ? pool_no_tls->find_connection(connection_id) : nullptr)
		return pool_no_tls->close(c, websocketpp::close::status::normal, "");

	scripting_log_error_with_context("No connection found with id {}", connection_id);
}

void websocket_pool_destroy()
{
#ifdef HAS_SSL
	if (pool_tls)
	{
		delete pool_tls;
		pool_tls = nullptr;
	}
#endif

	if (pool_no_tls)
	{
		delete pool_no_tls;
		pool_no_tls = nullptr;
	}
}
