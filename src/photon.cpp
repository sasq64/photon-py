#include <thread>

#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include <LoadBalancing-cpp/inc/Client.h>
#include <Photon-cpp/inc/PhotonPeer.h>

#include <variant>

namespace fs = std::filesystem;
namespace py = pybind11;
using namespace ExitGames;

struct Player
{
    std::string name;
    std::string user_id;
};

namespace events {
// clang-format off
struct Connected { int error_code; };
struct PlayerJoined
{
    int no;
    Player player;
};
struct PlayerLeft{};
struct Data { std::string payload; };
struct NoEvent{};
using PhotonEvent =
    std::variant<NoEvent, Connected, PlayerJoined, PlayerLeft, Data>;
// clang-format on
} // namespace events

class PhotonClient : LoadBalancing::Listener
{
    static inline const std::string appId =
        "5569cb7b-b875-41ab-9155-a44de7c4c227";
    LoadBalancing::Client client;

    std::deque<events::PhotonEvent> events;

public:
    PhotonClient(std::string const& app_id, std::string const& app_version)
        : client{*this, app_id.c_str(), app_version.c_str()}
    {
    }

    void connect() { client.connect(); }

    events::PhotonEvent update()
    {
        client.service();
        if (!events.empty()) {
            auto result = events.front();
            events.pop_front();
            return result;
        }
        return events::NoEvent{};
    }

    void debugReturn(int debugLevel, const Common::JString& string) override {}
    void connectionErrorReturn(int errorCode) override {}
    void clientErrorReturn(int errorCode) override {}
    void warningReturn(int warningCode) override {}
    void serverErrorReturn(int errorCode) override {}
    void joinRoomEventAction(int playerNr,
                             const Common::JVector<int>& playernrs,
                             const LoadBalancing::Player& player) override
    {
        Player p{player.getName().UTF8Representation().cstr(),
                 player.getUserID().UTF8Representation().cstr()};
        events.emplace_back(events::PlayerJoined{playerNr, p});
        printf("Joined as player %d\n", playerNr);
        // std::vector<unsigned char> data(50);
        // data[0] = 0x54;
        // client.opRaiseEvent(true, data.data(), data.size(), 1);
        // client.opRaiseEvent()
    }

    void send_data(int evt_no, std::string_view data, bool reliable)
    {
        auto const* ptr = reinterpret_cast<const unsigned char*>(data.data());
        printf("Sending %lu bytes", data.size());
        client.opRaiseEvent(reliable, ptr, data.size(), evt_no);
    }

    void join_room(std::string const& room_name)
    {
        client.opJoinOrCreateRoom(room_name.c_str());
    }

    void leaveRoomEventAction(int playerNr, bool isInactive) override {}

    void customEventAction(int playerNr, nByte eventCode,
                           const Common::Object& eventContent) override
    {
        auto vo = Common::ValueObject<unsigned char*>(eventContent);
        auto** ptr = vo.getDataAddress();
        auto const* len = vo.getSizes();
        printf("Got event %d from player %d, len, %d, data %x\n", eventCode,
               playerNr, *len, (*ptr)[0]);
        events.emplace_back(events::Data{std::string((const char*)ptr[0], *len)});
    }

    // callbacks for operations on the server
    void connectReturn(int errorCode, const Common::JString& errorString,
                       const Common::JString& region,
                       const Common::JString& cluster) override
    {
        printf("Connect %d %ls\n", errorCode, errorString.cstr());
        events.emplace_back(events::Connected{errorCode});
        // client.opJoinOrCreateRoom("game");
    }
    void disconnectReturn() override {}
    void leaveRoomReturn(int errorCode,
                         const Common::JString& errorString) override
    {
    }
};

void send_data_tuple(PhotonClient& thiz, int evt_no, py::tuple data, bool reliable)
{
    for(auto&& item : data)
    {
        py::handle h = item.get_type();
    }
}


PYBIND11_MODULE(photon, mod)
{
    mod.doc() = "photon";

    auto events = mod.def_submodule("event");
    py::class_<events::Connected>(events, "Connected")
        .def_readonly("error_code", &events::Connected::error_code)
        .attr("__match_args__") = std::make_tuple("error_code");
    py::class_<events::PlayerJoined>(events, "PlayerJoined")
        .def_readonly("player_no", &events::PlayerJoined::no)
        .def_readonly("player", &events::PlayerJoined::player)
        .attr("__match_args__") = std::make_tuple("player_no", "player");
    py::class_<events::Data>(events, "Data")
        .def_readonly("payload", &events::Data::payload)
        .attr("__match_args__") = std::make_tuple("payload");
    py::class_<events::PlayerLeft>(events, "PlayerLeft");
    py::class_<events::NoEvent>(events, "NoEvent");

    py::class_<Player>(mod, "Player")
        .def_readonly("name", &Player::name)
        .def_readonly("id", &Player::user_id);

    py::class_<PhotonClient, std::shared_ptr<PhotonClient>>(mod, "Client")
        .def("join_room", &PhotonClient::join_room, py::arg("name"))
        .def("send_event", &PhotonClient::send_data, py::arg("event"),
             py::arg("payload"), py::arg("reliable") = false)
        .def("send_event", &send_data_tuple, py::arg("event"),
             py::arg("payload"), py::arg("reliable") = false)
        .def("update", &PhotonClient::update)
        .def("connect", &PhotonClient::connect);

    mod.def(
        "create_client",
        [](std::string const& app_id, std::string const& app_version) {
            return std::make_shared<PhotonClient>(app_id, app_version);
        },
        py::arg("app_id"), py::arg("app_version") = "1.0");
}
