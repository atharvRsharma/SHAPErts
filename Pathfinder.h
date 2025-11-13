#pragma once

#include <vector>
#include <queue>
#include <unordered_map>
#include <set>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp> // For hashing glm::ivec2
#include "GridSystem.h"

namespace Pathfinder {

    // A* Node
    struct Node {
        glm::ivec2 pos;
        float gCost = FLT_MAX; // Cost from start
        float hCost = 0.0f;   // Heuristic cost to end
        float fCost = FLT_MAX; // gCost + hCost
        glm::ivec2 parent;
        bool inOpenSet = false;
        bool inClosedSet = false;

        Node(glm::ivec2 p = { 0,0 }) : pos(p) {}

        // For priority queue
        bool operator>(const Node& other) const {
            return fCost > other.fCost;
        }
    };

    // Heuristic function (Manhattan distance)
    inline float CalculateHeuristic(const glm::ivec2& a, const glm::ivec2& b) {
        return static_cast<float>(std::abs(a.x - b.x) + std::abs(a.y - b.y));
    }

    // Main A* function
    inline std::vector<glm::vec3> FindPath(GridSystem* gridSystem, glm::ivec2 start, glm::ivec2 end) {
        std::vector<glm::vec3> path;

        // 1. Initialize data structures
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
        std::unordered_map<glm::ivec2, Node> allNodes;

        // 2. Create start node
        Node startNode(start);
        startNode.gCost = 0.0f;
        startNode.hCost = CalculateHeuristic(start, end);
        startNode.fCost = startNode.hCost;
        startNode.inOpenSet = true;

        openSet.push(startNode);
        allNodes[start] = startNode;

        // 3. Define 4-directional neighbors
        const std::array<glm::ivec2, 4> neighbors = {
            glm::ivec2(0, 1),  // Up
            glm::ivec2(0, -1), // Down
            glm::ivec2(1, 0),  // Right
            glm::ivec2(-1, 0)  // Left
        };

        // 4. A* main loop
        while (!openSet.empty()) {
            // Get node with lowest fCost
            Node currentNode = openSet.top();
            openSet.pop();

            currentNode.inOpenSet = false;
            currentNode.inClosedSet = true;
            allNodes[currentNode.pos] = currentNode;

            // 5. Check for goal
            if (currentNode.pos == end) {
                // --- Path Found! Reconstruct it ---
                glm::ivec2 current = end;
                while (current != start) {
                    path.push_back(gridSystem->GridToWorld(current.x, current.y));
                    current = allNodes[current].parent;
                }
                path.push_back(gridSystem->GridToWorld(start.x, start.y));
                std::reverse(path.begin(), path.end());
                return path;
                // ---
            }

            // 6. Check neighbors
            for (const auto& dir : neighbors) {
                glm::ivec2 neighborPos = currentNode.pos + dir;

                // Check if walkable and not in closed set
                if (!gridSystem->IsWalkable(neighborPos.x, neighborPos.y)) continue;

                if (allNodes.count(neighborPos) && allNodes[neighborPos].inClosedSet) continue;

                // 7. Calculate new costs
                float newGCost = currentNode.gCost + 1.0f; // Cost to move is 1

                Node& neighborNode = allNodes[neighborPos]; // Get or create

                if (newGCost < neighborNode.gCost) {
                    neighborNode.pos = neighborPos;
                    neighborNode.parent = currentNode.pos;
                    neighborNode.gCost = newGCost;
                    neighborNode.hCost = CalculateHeuristic(neighborPos, end);
                    neighborNode.fCost = neighborNode.gCost + neighborNode.hCost;

                    if (!neighborNode.inOpenSet) {
                        openSet.push(neighborNode);
                        neighborNode.inOpenSet = true;
                    }
                }
            }
        }

        // No path found
        return path;
    }

} // namespace Pathfinder