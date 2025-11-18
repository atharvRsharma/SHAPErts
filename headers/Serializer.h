#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>     
#include <nlohmann/json.hpp>
#include "Components.h"

using json = nlohmann::json;




namespace glm {
    void to_json(json& j, const vec3& v) { j = json{ v.x, v.y, v.z }; }
    void from_json(const json& j, vec3& v) { j.at(0).get_to(v.x); j.at(1).get_to(v.y); j.at(2).get_to(v.z); }
    void to_json(json& j, const ivec2& v) { j = json{ v.x, v.y }; }
    void from_json(const json& j, ivec2& v) { j.at(0).get_to(v.x); j.at(1).get_to(v.y); }
}

void to_json(json& j, const TransformComponent& p) {
    j = json{ {"pos", p.position}, {"scale", p.scale}, {"rot", p.rotation} };
}
void from_json(const json& j, TransformComponent& p) {
    j.at("pos").get_to(p.position);
    j.at("scale").get_to(p.scale);
    j.at("rot").get_to(p.rotation);
}

void to_json(json& j, const RenderComponent& p) {
    j = json{ p.color.r, p.color.g, p.color.b, p.color.a };
}
void from_json(const json& j, RenderComponent& p) {
    j.at(0).get_to(p.color.r); j.at(1).get_to(p.color.g);
    j.at(2).get_to(p.color.b); j.at(3).get_to(p.color.a);
}

NLOHMANN_JSON_SERIALIZE_ENUM(MeshType, {
    {MeshType::None, "none"}, {MeshType::Quad, "quad"}, {MeshType::Cube, "cube"},
    {MeshType::Base, "base"}, {MeshType::Turret, "turret"}, {MeshType::Sphere, "sphere"}
    })
    NLOHMANN_JSON_SERIALIZE_ENUM(BuildingType, {
        {BuildingType::None, "none"}, {BuildingType::Base, "base"}, {BuildingType::Turret, "turret"},
        {BuildingType::ResourceNode, "node"}, {BuildingType::Bomb, "bomb"}
        })

    void to_json(json& j, const MeshComponent& p) { j = json{ {"type", p.type} }; }
void from_json(const json& j, MeshComponent& p) { j.at("type").get_to(p.type); }

void to_json(json& j, const BuildingComponent& p) { j = json{ {"type", p.type} }; }
void from_json(const json& j, BuildingComponent& p) { j.at("type").get_to(p.type); }

void to_json(json& j, const HealthComponent& p) {
    j = json{ {"hp", p.currentHP}, {"max", p.maxHP} };
}
void from_json(const json& j, HealthComponent& p) {
    j.at("hp").get_to(p.currentHP); j.at("max").get_to(p.maxHP);
}

void to_json(json& j, const ResourceGeneratorComponent& p) {
    j = json{ {"rate", p.resourcesPerSecond} };
}
void from_json(const json& j, ResourceGeneratorComponent& p) {
    j.at("rate").get_to(p.resourcesPerSecond);
}

void to_json(json& j, const TurretAIComponent& p) {
    j = json{ {"target", p.currentTarget}, {"ammo", p.currentAmmo}, {"cd", p.fireCooldown} };
}
void from_json(const json& j, TurretAIComponent& p) {
    j.at("target").get_to(p.currentTarget);
    j.at("ammo").get_to(p.currentAmmo);
    j.at("cd").get_to(p.fireCooldown);
}

void to_json(json& j, const BombComponent& p) { j = json{ {"dmg", p.damage} }; }
void from_json(const json& j, BombComponent& p) { j.at("dmg").get_to(p.damage); }

void to_json(json& j, const CollisionComponent& p) { j = json{ {"r", p.radius} }; }
void from_json(const json& j, CollisionComponent& p) { j.at("r").get_to(p.radius); }

void to_json(json& j, const MovementComponent& p) {
    j = json{ {"target", p.targetEntity}, {"attacking", p.isAttacking} };
}
void from_json(const json& j, MovementComponent& p) {
    j.at("target").get_to(p.targetEntity);
    j.at("attacking").get_to(p.isAttacking);
}