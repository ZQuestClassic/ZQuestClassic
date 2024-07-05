#include "zc/replay_upload.h"
#include "base/http.h"
#include "zc/replay.h"
#include "zconfig.h"
#include "zsyssimple.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <map>
#include <chrono>
#include <filesystem>
#include <optional>

static const int TIME_ONE_DAY = 86400;
static const int TIME_ONE_WEEK = TIME_ONE_DAY * 7;

namespace fs = std::filesystem;
using json = nlohmann::json;

template <typename T>
struct api_response
{
	int status_code;
	T data;
};

struct api_error
{
	int status_code;
	std::string message;

	bool server_error() const
	{
		return status_code >= 500 && status_code < 600;
	}
};

template <typename T>
static std::optional<std::string> try_deserialize(T& data, const json& json)
{
	try
	{
		data = json;
		return std::nullopt;
	}
	catch (json::exception& ex)
	{
		return ex.what();
	}
}

enum class state {
	untracked,
	ignored,
	try_later,
	tracked,
};
NLOHMANN_JSON_SERIALIZE_ENUM(state, {
    {state::untracked, nullptr},
    {state::untracked, "untracked"},
    {state::ignored, "ignored"},
	{state::try_later, "try_later"},
    {state::tracked, "tracked"},
})

struct status_entry_t
{
	std::string key;
	state state;
	int64_t time;
	std::string error;

	void try_later(int when)
	{
		state = state::try_later;
		time = when;
	}

	void ignore()
	{
		state = state::ignored;
	}
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(status_entry_t, key, state, time, error)

struct replay_upload_state
{
	std::map<std::string, status_entry_t> entries;
	int time_last_run;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(replay_upload_state, entries, time_last_run)

static int64_t get_last_write_time(fs::path path)
{
	auto file_time = fs::last_write_time(path);
	return std::chrono::duration_cast<std::chrono::seconds>(
		file_time.time_since_epoch()).count();
}

namespace api_client
{
	static std::string api_endpoint;

	template <typename T>
	static auto _parse_response(const expected<http::http_response, std::string>& response) -> expected<api_response<T>, api_error>
	{
		if (!response)
			return make_unexpected(api_error{0, response.error()});

		json json = json::parse(response->body, nullptr, false);
		if (json.is_discarded())
			return make_unexpected(api_error{response->status_code, "invalid json"});

		if (!response->success())
		{
			std::string error;
			if (json.contains("error") && json["error"].is_string())
				error = json["error"].template get<std::string>();
			else
				error = "server error";
			return make_unexpected(api_error{response->status_code, error});
		}

		T data;
		if (auto error = try_deserialize(data, json))
			return make_unexpected(api_error{response->status_code, *error});

		return api_response<T>{response->status_code, data};
	}

	struct quest {
		std::string hash;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(quest, hash)
	using quests_result = std::vector<quest>;

	static auto quests()
	{
		std::string url = fmt::format("{}/api/v1/quests", api_endpoint);
		return _parse_response<quests_result>(http::get(url));
	}

	struct upload_result {
		std::string key;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(upload_result, key)

	static auto replays_upload(std::string uuid, fs::path path)
	{
		std::string url = fmt::format("{}/api/v1/replays/{}", api_endpoint, uuid);
		return _parse_response<upload_result>(http::upload(url, path));
	}
}

static std::optional<api_client::quests_result> quests;

static fs::path get_state_path()
{
	fs::path replay_file_dir = zc_get_config("zeldadx", "replay_file_dir", "replays/");
	return replay_file_dir / "state.json";
}

static replay_upload_state load_state()
{
	replay_upload_state state = {};

	fs::path state_path = get_state_path();
	if (fs::exists(state_path))
	{
		std::ifstream f(state_path);
		json j = json::parse(f, nullptr, false);
		if (j.is_discarded())
		{
			Z_message("invalid json: %s\n", state_path.string().c_str());
		}
		if (auto error = try_deserialize(state, j))
		{
			Z_message("invalid json: %s\n", error.value().c_str());
		}
	}

	return state;
}

bool replay_upload_auto_enabled()
{
	// TODO: get rid if we ever improve our config story.
	if (std::getenv("TEST_ZC_API_SERVER") != nullptr)
		return true;

	return zc_get_config("zeldadx", "replay_upload", false);
}

static bool should_process_replay(status_entry_t& status_entry, fs::path path, int64_t now_time)
{
	if (status_entry.state == state::ignored)
		return false;

	switch (status_entry.state)
	{
		case state::untracked:
		{
			return true;
		}
		break;

		case state::try_later:
		{
			return now_time > status_entry.time;
		}
		break;

		case state::tracked:
		{
			int last_write_time = get_last_write_time(path);
			return last_write_time > status_entry.time;
		}
		break;
	}

	return false;
}

static bool process_replay(status_entry_t& status_entry, fs::path path, std::string rel_fname, int64_t now_time)
{
	status_entry = {};
	status_entry.time = now_time;

	auto meta_map = replay_load_meta(path);
	std::string uuid = meta_map["uuid"];
	std::string qst_hash = meta_map["qst_hash"];
	if (uuid.empty() || qst_hash.empty() || !meta_map.contains("length"))
	{
		status_entry.ignore();
		status_entry.error = "replay is too old";
		return false;
	}

	status_entry.key = fmt::format("{}/{}.zplay", qst_hash, uuid);

	// Check if the server has this qst.
	bool known_qst = false;
	for (auto& quest : *quests)
	{
		if (quest.hash == qst_hash)
		{
			known_qst = true;
			break;
		}
	}

	if (!known_qst)
	{
		status_entry.try_later(now_time + TIME_ONE_WEEK * 2);
		status_entry.error = "qst is not in database";
		return false;
	}

	if (auto r = api_client::replays_upload(uuid, path); !r)
	{
		auto& error = r.error();
		status_entry.error = error.message;
		if (error.server_error() || error.status_code == 0)
			status_entry.try_later(now_time + TIME_ONE_DAY);
		else
			status_entry.ignore();
		return false;
	}

	status_entry.state = state::tracked;
	return true;
}

static int replay_upload_impl(replay_upload_state& state)
{
	Z_message("Checking for replays to upload ...\n");

	api_client::api_endpoint = zc_get_config("zeldadx", "api_endpoint", "");
	// TODO: get rid if we ever improve our config story.
	if (std::getenv("TEST_ZC_API_SERVER") != nullptr)
		api_client::api_endpoint = std::getenv("TEST_ZC_API_SERVER");
	if (api_client::api_endpoint.empty())
	{
		Z_message("api_endpoint not set\n");
		return 0;
	}

	Z_message("api_endpoint set: %s\n", api_client::api_endpoint.c_str());

	fs::path replay_file_dir = zc_get_config("zeldadx", "replay_file_dir", "replays/");
	if (!fs::exists(replay_file_dir))
	{
		Z_message("No replays found\n");
		return 0;
	}

	if (!quests)
	{
		if (auto r = api_client::quests(); !r)
		{
			Z_message("Error fetching quests: %s\n", r.error().message.c_str());
			return 0;
		}
		else
		{
			quests = std::move(r->data);
		}
	}

	int replays_uploaded = 0;
	int64_t now_time = std::chrono::duration_cast<std::chrono::seconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();
	state.time_last_run = now_time;

	for (const auto& entry : fs::recursive_directory_iterator(replay_file_dir))
	{
		if (!entry.is_regular_file() || entry.path().extension() != ".zplay")
			continue;

		std::string rel_fname = fs::relative(entry.path(), replay_file_dir).make_preferred().string();
		auto& status_entry = state.entries[rel_fname];
		if (!should_process_replay(status_entry, entry.path(), now_time))
			continue;

		if (process_replay(status_entry, entry.path(), rel_fname, now_time))
		{
			replays_uploaded++;
			Z_message("[%s] Success\n", rel_fname.c_str());
		}
		else
		{
			Z_message("[%s] Failed - %s\n", rel_fname.c_str(), status_entry.error.c_str());
		}
	}

	fs::path state_path = get_state_path();
	std::ofstream out(state_path, std::ios::binary);
	json j = state;
	out << j.dump(2);

	Z_message("Uploaded %d replays.\n", replays_uploaded);
	return replays_uploaded;
}

int replay_upload()
{
	auto state = load_state();
	return replay_upload_impl(state);
}

int replay_upload_auto()
{
	if (replay_upload_auto_enabled())
		return 0;

	auto state = load_state();
	int64_t now_time = std::chrono::duration_cast<std::chrono::seconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();
	if (now_time - state.time_last_run <= TIME_ONE_WEEK)
		return 0;

	return replay_upload_impl(state);
}

void replay_upload_clear_cache()
{
	fs::path state_path = get_state_path();
	std::error_code ec;
	fs::remove(state_path, ec);
	if (ec)
	{
		std::string error = fmt::format("Failed to clear cache: {}", std::strerror(ec.value()));
		Z_message("%s\n", error.c_str());
	}
}
