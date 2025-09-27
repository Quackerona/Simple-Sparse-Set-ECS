#ifndef WORLD_HPP
#define WORLD_HPP

#include <vector>
#include <unordered_map>
#include <optional>

class Entity
{
    private:
        std::size_t id;
        std::size_t gen;

        Entity(std::size_t id, std::size_t gen) : id(id), gen(gen) {}

        friend class World;
    public:
        bool operator==(const Entity& other) const
        {
            return (id == other.id) && (gen == other.gen);
        }

        bool operator!=(const Entity& other) const
        {
            return (id != other.id) || (gen != other.gen);
        }

        std::size_t GetID() const
        {
            return id;
        }
};

struct ISparseSet
{
    virtual ~ISparseSet() = default;

    virtual void Remove(Entity entity) = 0;
    virtual bool Contains(Entity entity) const = 0;
    virtual void Clear() = 0;
};

template <typename C>
class SparseSet : public ISparseSet
{
    private:
        std::vector<std::size_t> sparse;

        std::vector<Entity> entities;
        std::vector<C> components;

        SparseSet() = default;

        friend class World;
    public:
        void Add(Entity entity, C&& component)
        {
            std::size_t idx{entity.GetID()};

            if (idx >= sparse.size()) sparse.resize(idx + 1);

            sparse[idx] = entities.size();

            entities.push_back(entity);
            components.push_back(std::forward<C>(component));
        }

        void Remove(Entity entity) override
        {
            std::size_t idx{sparse[entity.GetID()]};

            if (idx != entities.size() - 1)
            {
                std::swap(entities[idx], entities.back());
                std::swap(components[idx], components.back());

                sparse[entities[idx].GetID()] = idx;
            }

            entities.pop_back();
            components.pop_back();
        }

        bool Contains(Entity entity) const override
        {
            std::size_t idx{entity.GetID()};

            return (idx < sparse.size()) && (entities[sparse[idx]] == entity);
        }

        void Clear() override
        {
            entities.clear();
            components.clear();
        }

        C& Get(Entity entity)
        {
            return components[sparse[entity.GetID()]];
        }

        const std::vector<Entity>* GetEntities() const // For views.
        {
            return &entities;
        }
};

template <typename... Cs>
class View
{
    private:
        std::tuple<SparseSet<Cs>*...> sparseSetTuple;
        const std::vector<Entity>* smallestEntityVector;

        View(std::tuple<SparseSet<Cs>*...>&& sparseSetTuple) : sparseSetTuple(sparseSetTuple), smallestEntityVector(nullptr)
        {
            std::apply([&smallestEntityVector = this->smallestEntityVector](auto... sparseSets){
                ([&smallestEntityVector, sparseSets]{
                    if (!smallestEntityVector) smallestEntityVector = sparseSets->GetEntities();
                    else
                    {
                        const std::vector<Entity>* entities{sparseSets->GetEntities()};
                        if (entities->size() < smallestEntityVector->size()) smallestEntityVector = entities;
                    }
                }(), ...);
            }, sparseSetTuple);
        }

        friend class World;
    public:
        template <typename C>
        requires (((std::is_same_v<C, Cs>) || ...))
        bool HasComponent(Entity entity) const
        {
            return std::get<SparseSet<C>*>(sparseSetTuple)->Contains(entity);
        }

        template <typename C>
        requires (((std::is_same_v<C, Cs>) || ...))
        C* GetComponent(Entity entity) const
        {
            if (HasComponent<C>(entity))
                return &std::get<SparseSet<C>*>(sparseSetTuple)->Get(entity);
            
            return nullptr;
        }

        template <typename F>
        requires std::is_invocable_v<F, Cs&...>
        void Each(F&& func) const
        {
            for (Entity entity : *smallestEntityVector)
            {
                if (((HasComponent<Cs>(entity)) && ...))
                    func(std::get<SparseSet<Cs>*>(sparseSetTuple)->Get(entity)...);
            }
        }

        template <typename F>
        requires std::is_invocable_v<F, Entity, Cs&...>
        void Each(F&& func) const
        {
            for (Entity entity : *smallestEntityVector)
            {
                if (((HasComponent<Cs>(entity)) && ...))
                    func(entity, std::get<SparseSet<Cs>*>(sparseSetTuple)->Get(entity)...);
            }
        }
};

std::size_t AssignUniqueComponentID()
{
    static std::size_t id{0};
    return id++;
}

template <typename C>
std::size_t GetComponentID()
{
    static std::size_t id{AssignUniqueComponentID()};
    return id;
}

class World
{
    private:
        std::vector<std::size_t> deadIDs;
        std::vector<std::size_t> generations;

        std::unordered_map<std::size_t, ISparseSet*> sparseSetMap;

        World() = default;

        friend class Engine;
    public:
        World(const World&) = delete;
        World(World&&) = delete;
        ~World()
        {
            for (auto& pair : sparseSetMap) delete pair.second;
        }

        World& operator=(const World&) = delete;
        World& operator=(World&&) = delete;

        Entity SpawnEntity()
        {
            if (!deadIDs.empty())
            {
                std::size_t id{deadIDs.back()};
                deadIDs.pop_back();

                return Entity{id, generations[id]};
            }
            else
            {
                std::size_t id{generations.size()};

                generations.emplace_back(0);

                return Entity{id, 0};
            }
        };

        void DespawnEntity(Entity entity)
        {
            if (!IsEntityValid(entity)) return;

            std::size_t id{entity.GetID()};

            deadIDs.push_back(id);
            ++generations[id];

            for (auto& pair : sparseSetMap) if (pair.second->Contains(entity)) pair.second->Remove(entity);
        }

        bool IsEntityValid(Entity entity) const
        {
            std::size_t id{entity.GetID()};

            return (id < generations.size()) && (entity.gen == generations[id]);
        }

        template <typename C>
        void AddComponent(Entity entity, C&& component)
        {
            if (!IsEntityValid(entity)) return;

            std::size_t componentID{GetComponentID<C>()};

            SparseSet<C>* sparseSet;
            auto sparseIt{sparseSetMap.find(componentID)};
            if (sparseIt != sparseSetMap.end())
                sparseSet = static_cast<SparseSet<C>*>(sparseIt->second);
            else
            {
                sparseSet = new SparseSet<C>{};
                sparseSetMap.emplace(componentID, sparseSet);
            }

            if (sparseSet->Contains(entity)) sparseSet->Get(entity) = std::forward<C>(component);
            else sparseSet->Add(entity, std::forward<C>(component));
        }

        template <typename C>
        void RemoveComponent(Entity entity)
        {
            if (!IsEntityValid(entity)) return;

            auto sparseIt{sparseSetMap.find(GetComponentID<C>())};
            if (sparseIt != sparseSetMap.end())
            {
                SparseSet<C>* sparseSet{static_cast<SparseSet<C>*>(sparseIt->second)};

                if (sparseSet->Contains(entity)) sparseSet->Remove(entity);
            }
        }

        void ClearEntities()
        {
            for (auto& pair : sparseSetMap) pair.second->Clear();
        }

        template <typename... Cs>
        std::optional<View<Cs...>> CreateView() const
        {
            auto itTuple{std::make_tuple(sparseSetMap.find(GetComponentID<Cs>())...)};

            auto itEnd{sparseSetMap.end()};

            return std::apply([itEnd](auto... its){
                if (((its != itEnd) && ...)) return std::optional<View<Cs...>>{View<Cs...>{std::make_tuple(static_cast<SparseSet<Cs>*>(its->second)...)}};
                else return std::optional<View<Cs...>>{std::nullopt};
            }, itTuple);
        }
};

#endif