#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <set>
#include <array>
#include <cassert>
#include <queue>
#include <bitset>
#include <algorithm> 

namespace ecs {

    using Entity = uint32_t;
    constexpr Entity MAX_ENTITIES = 5000;
    using ComponentTypeID = uint8_t;
    constexpr ComponentTypeID MAX_COMPONENTS = 32;
    using Signature = std::bitset<MAX_COMPONENTS>;


    class EntityManager {
    public:
        EntityManager() {
            for (Entity e = 0; e < MAX_ENTITIES; ++e) {
                m_AvailableEntities.push(e);
            }
            m_LivingEntityCount = 0;
        }

        Entity CreateEntity() {
            assert(m_LivingEntityCount < MAX_ENTITIES && "Max entities exceeded");

            Entity id = m_AvailableEntities.front();
            m_AvailableEntities.pop();
            ++m_LivingEntityCount;
            m_LivingEntities.insert(id);
            return id;
        }

        Entity CreateEntity(Entity id) {
            assert(id < MAX_ENTITIES && "Entity ID out of range");

            std::queue<Entity> temp;
            bool found = false;
            while (!m_AvailableEntities.empty()) {
                Entity e = m_AvailableEntities.front();
                m_AvailableEntities.pop();
                if (e == id) {
                    found = true;
                }
                else {
                    temp.push(e);
                }
            }
            m_AvailableEntities = temp;

            assert(found && "Trying to create an entity ID that was already in use");

            ++m_LivingEntityCount;
            m_LivingEntities.insert(id);
            return id;
        }

        void DestroyEntity(Entity entity) {
            assert(entity < MAX_ENTITIES && "Entity out of range");

            m_Signatures[entity].reset();
            m_AvailableEntities.push(entity);
            --m_LivingEntityCount;
            m_LivingEntities.erase(entity);
        }

        void SetSignature(Entity entity, Signature signature) {
            assert(entity < MAX_ENTITIES && "Entity out of range");
            m_Signatures[entity] = signature;
        }

        Signature GetSignature(Entity entity) const {
            assert(entity < MAX_ENTITIES && "Entity out of range");
            return m_Signatures[entity];
        }

        uint32_t GetLivingEntityCount() const {
            return m_LivingEntityCount;
        }

        const std::set<Entity>& GetLivingEntities() const {
            return m_LivingEntities;
        }

        void Reset() {
            m_LivingEntityCount = 0;
            m_LivingEntities.clear();

            std::queue<Entity> empty;
            std::swap(m_AvailableEntities, empty);
            for (Entity e = 0; e < MAX_ENTITIES; ++e) {
                m_AvailableEntities.push(e);
                m_Signatures[e].reset();
            }
        }

    private:
        std::queue<Entity> m_AvailableEntities{};
        std::array<Signature, MAX_ENTITIES> m_Signatures{};
        uint32_t m_LivingEntityCount{};
        std::set<Entity> m_LivingEntities{};
    };


    class IComponentArray {
    public:
        virtual ~IComponentArray() = default;
        virtual void EntityDestroyed(Entity entity) = 0;
        virtual void Reset() = 0;
    };

    template<typename T>
    class ComponentArray : public IComponentArray {
    public:
        void InsertData(Entity entity, T component) {
            assert(m_EntityToIndexMap.find(entity) == m_EntityToIndexMap.end() && "Component added twice");
            size_t newIndex = m_Size;
            m_EntityToIndexMap[entity] = newIndex;
            m_IndexToEntityMap[newIndex] = entity;
            m_ComponentArray[newIndex] = component;
            ++m_Size;
        }

        void RemoveData(Entity entity) {
            assert(m_EntityToIndexMap.find(entity) != m_EntityToIndexMap.end() && "Removing non-existent component");
            size_t indexOfRemoved = m_EntityToIndexMap[entity];
            size_t indexOfLast = m_Size - 1;
            m_ComponentArray[indexOfRemoved] = m_ComponentArray[indexOfLast];
            Entity entityOfLast = m_IndexToEntityMap[indexOfLast];
            m_EntityToIndexMap[entityOfLast] = indexOfRemoved;
            m_IndexToEntityMap[indexOfRemoved] = entityOfLast;
            m_EntityToIndexMap.erase(entity);
            m_IndexToEntityMap.erase(indexOfLast);
            --m_Size;
        }

        T& GetData(Entity entity) {
            assert(m_EntityToIndexMap.find(entity) != m_EntityToIndexMap.end() && "Retrieving non-existent component");
            return m_ComponentArray[m_EntityToIndexMap[entity]];
        }

        bool HasData(Entity entity) {
            return m_EntityToIndexMap.find(entity) != m_EntityToIndexMap.end();
        }

        void EntityDestroyed(Entity entity) override {
            if (HasData(entity)) {
                RemoveData(entity);
            }
        }

        void Reset() override {
            m_EntityToIndexMap.clear();
            m_IndexToEntityMap.clear();
            m_Size = 0;
        }

    private:
        std::array<T, MAX_ENTITIES> m_ComponentArray;
        std::unordered_map<Entity, size_t> m_EntityToIndexMap;
        std::unordered_map<size_t, Entity> m_IndexToEntityMap;
        size_t m_Size{};
    };


    class ComponentManager {
    public:
        template<typename T>
        void RegisterComponent() {
            const char* typeName = typeid(T).name();
            assert(m_ComponentTypes.find(typeName) == m_ComponentTypes.end() && "Registering component type more than once");
            m_ComponentTypes.insert({ typeName, m_NextComponentTypeID });
            m_ComponentArrays.insert({ typeName, std::make_shared<ComponentArray<T>>() });
            ++m_NextComponentTypeID;
        }

        template<typename T>
        ComponentTypeID GetComponentTypeID() const {
            const char* typeName = typeid(T).name();
            assert(m_ComponentTypes.find(typeName) != m_ComponentTypes.end() && "Component not registered");
            return m_ComponentTypes.at(typeName);
        }

        template<typename T>
        void AddComponent(Entity entity, T component) {
            GetComponentArray<T>()->InsertData(entity, component);
        }

        template<typename T>
        void RemoveComponent(Entity entity) {
            GetComponentArray<T>()->RemoveData(entity);
        }

        template<typename T>
        T& GetComponent(Entity entity) {
            return GetComponentArray<T>()->GetData(entity);
        }

        template<typename T>
        bool HasComponent(Entity entity) {
            return GetComponentArray<T>()->HasData(entity);
        }

        void EntityDestroyed(Entity entity) {
            for (auto const& pair : m_ComponentArrays) {
                pair.second->EntityDestroyed(entity);
            }
        }

        void Reset() {
            for (auto const& pair : m_ComponentArrays) {
                pair.second->Reset();
            }
        }
    private:
        std::unordered_map<const char*, ComponentTypeID> m_ComponentTypes{};
        std::unordered_map<const char*, std::shared_ptr<IComponentArray>> m_ComponentArrays{};
        ComponentTypeID m_NextComponentTypeID{};

        template<typename T>
        std::shared_ptr<ComponentArray<T>> GetComponentArray() {
            const char* typeName = typeid(T).name();
            assert(m_ComponentTypes.find(typeName) != m_ComponentTypes.end() && "Component not registered");
            return std::static_pointer_cast<ComponentArray<T>>(m_ComponentArrays.at(typeName));
        }
    };


    class Registry; // Forward declare

    class System {
    public:
        virtual ~System() = default;
        std::set<Entity> m_Entities;
        Registry* m_Registry = nullptr;
    };


    class SystemManager {
    public:
        template<typename T>
        std::shared_ptr<T> RegisterSystem(Registry* registry) {
            const char* typeName = typeid(T).name();
            assert(m_Systems.find(typeName) == m_Systems.end() && "Registering system more than once");
            auto system = std::make_shared<T>();
            system->m_Registry = registry;
            m_Systems.insert({ typeName, system });
            return system;
        }

        template<typename T>
        std::shared_ptr<T> GetSystem() const {
            const char* typeName = typeid(T).name();
            assert(m_Systems.find(typeName) != m_Systems.end() && "System not registered");
            return std::static_pointer_cast<T>(m_Systems.at(typeName));
        }

        template<typename T>
        void SetSignature(Signature signature) {
            const char* typeName = typeid(T).name();
            assert(m_Systems.find(typeName) != m_Systems.end() && "System not registered");
            m_Signatures.insert({ typeName, signature });
        }

        void EntityDestroyed(Entity entity) {
            for (auto const& pair : m_Systems) {
                pair.second->m_Entities.erase(entity);
            }
        }

        void EntitySignatureChanged(Entity entity, Signature entitySignature) {
            for (auto const& pair : m_Systems) {
                auto const& type = pair.first;
                auto const& system = pair.second;
                if (m_Signatures.find(type) == m_Signatures.end()) continue;
                auto const& systemSignature = m_Signatures.at(type);
                if ((entitySignature & systemSignature) == systemSignature) {
                    system->m_Entities.insert(entity);
                }
                else {
                    system->m_Entities.erase(entity);
                }
            }
        }

        void Reset() {
            for (auto const& pair : m_Systems) {
                pair.second->m_Entities.clear();
            }
        }

    private:
        std::unordered_map<const char*, Signature> m_Signatures{};
        std::unordered_map<const char*, std::shared_ptr<System>> m_Systems{};
    };


    class Registry {
    public:
        Registry() {
            m_EntityManager = std::make_unique<EntityManager>();
            m_ComponentManager = std::make_unique<ComponentManager>();
            m_SystemManager = std::make_unique<SystemManager>();
        }

        Entity CreateEntity() { return m_EntityManager->CreateEntity(); }
        Entity CreateEntity(Entity id) { return m_EntityManager->CreateEntity(id); } // <-- NEW
        void DestroyEntity(Entity entity) {
            m_EntityManager->DestroyEntity(entity);
            m_ComponentManager->EntityDestroyed(entity);
            m_SystemManager->EntityDestroyed(entity);
        }
        uint32_t GetLivingEntityCount() const { return m_EntityManager->GetLivingEntityCount(); }
        const std::set<Entity>& GetLivingEntities() const { return m_EntityManager->GetLivingEntities(); } // <-- NEW

        void Reset() {
            m_EntityManager->Reset();
            m_ComponentManager->Reset();
            m_SystemManager->Reset();
        }

        template<typename T>
        void RegisterComponent() { m_ComponentManager->RegisterComponent<T>(); }
        template<typename T>
        void AddComponent(Entity entity, T component) {
            m_ComponentManager->AddComponent<T>(entity, component);
            auto signature = m_EntityManager->GetSignature(entity);
            signature.set(m_ComponentManager->GetComponentTypeID<T>(), true);
            m_EntityManager->SetSignature(entity, signature);
            m_SystemManager->EntitySignatureChanged(entity, signature);
        }
        template<typename T>
        void RemoveComponent(Entity entity) {
            m_ComponentManager->RemoveComponent<T>(entity);
            auto signature = m_EntityManager->GetSignature(entity);
            signature.set(m_ComponentManager->GetComponentTypeID<T>(), false);
            m_EntityManager->SetSignature(entity, signature);
            m_SystemManager->EntitySignatureChanged(entity, signature);
        }
        template<typename T>
        T& GetComponent(Entity entity) { return m_ComponentManager->GetComponent<T>(entity); }
        template<typename T>
        bool HasComponent(Entity entity) { return m_ComponentManager->HasComponent<T>(entity); }
        template<typename T>
        ComponentTypeID GetComponentTypeID() const { return m_ComponentManager->GetComponentTypeID<T>(); }

        template<typename T>
        std::shared_ptr<T> RegisterSystem() { return m_SystemManager->RegisterSystem<T>(this); }
        template<typename T>
        std::shared_ptr<T> GetSystem() const { return m_SystemManager->GetSystem<T>(); }
        template<typename T>
        void SetSystemSignature(Signature signature) { m_SystemManager->SetSignature<T>(signature); }

    private:
        std::unique_ptr<EntityManager> m_EntityManager;
        std::unique_ptr<ComponentManager> m_ComponentManager;
        std::unique_ptr<SystemManager> m_SystemManager;
    };

}