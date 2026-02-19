#include "player_stats.hpp"

#include <cmath>

#include <include/cef_process_message.h>
#include <nlohmann/json.hpp>

#include "utf8.hpp"

#include "browser/manager.hpp"

static inline bool NearlyEqual(float a, float b, float eps = 0.001f) noexcept
{
    return std::fabs(a - b) < eps;
}

bool PlayerStats::Equal(const Snapshot& a, const Snapshot& b) noexcept
{
    return a.hp == b.hp
        && a.max_hp == b.max_hp
        && a.arm == b.arm
        && a.breath == b.breath
        && a.wanted == b.wanted
        && a.weapon == b.weapon
        && a.ammo == b.ammo
        && a.max_ammo == b.max_ammo
        && a.money == b.money
        && NearlyEqual(a.speed, b.speed)
        && NearlyEqual(a.x, b.x)
        && NearlyEqual(a.y, b.y)
        && NearlyEqual(a.z, b.z)
        && NearlyEqual(a.heading, b.heading)
        && NearlyEqual(a.camera_heading, b.camera_heading)
        && a.in_vehicle == b.in_vehicle
        && a.aiming == b.aiming
        && a.vehicle_model == b.vehicle_model
        && NearlyEqual(a.vehicle_health, b.vehicle_health);
}

std::string PlayerStats::ToJson(const Snapshot& snapshot)
{
    nlohmann::json json;

    json["hp"] = snapshot.hp;
    json["max_hp"] = snapshot.max_hp;
    json["arm"] = snapshot.arm;
    json["breath"] = snapshot.breath;
    json["wanted"] = snapshot.wanted;
    json["weapon"] = snapshot.weapon;
    json["ammo"] = snapshot.ammo;
    json["max_ammo"] = snapshot.max_ammo;
    json["money"] = snapshot.money;
    json["speed"] = snapshot.speed;

    json["flags"] = {
        {"in_vehicle", snapshot.in_vehicle},
        {"aiming", snapshot.aiming}
    };

    json["vehicle"] = {
        {"model", snapshot.vehicle_model},
        {"health", snapshot.vehicle_health}
    };

    json["pos"] = {
        {"x", snapshot.x},
        {"y", snapshot.y},
        {"z", snapshot.z}
    };

    json["heading"] = snapshot.heading;
    json["camera_heading"] = snapshot.camera_heading;
    json["in_vehicle"] = snapshot.in_vehicle;

    return json.dump();
}

void PlayerStats::EmitJson(BrowserInstance* instance, std::string_view eventName, std::string_view jsonPayload)
{
    if (!instance || !instance->browser)
        return;

    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("emit_event");
    CefRefPtr<CefListValue> list = msg->GetArgumentList();

    list->SetString(0, EnsureUtf8ForCef(eventName));
    list->SetString(1, EnsureUtf8ForCef(jsonPayload));

    instance->browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, msg);
}