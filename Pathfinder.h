#pragma once

#include <vector>
#include <queue>
#include <unordered_map>
#include <set>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp> 
#include "GridSystem.h"

namespace Pathfinder {

    struct Node {
        glm::ivec2 pos;
        float gCost = FLT_MAX;
        float hCost = 0.0f;
        float fCost = FLT_MAX;
        glm::ivec2 parent;
        bool inOpenSet = false;
        bool inClosedSet = false;

        Node(glm::ivec2 p = { 0,0 }) : pos(p) {}
        bool operator>(const Node& other) const { return fCost > other.fCost; }
    };

    inline float CalculateHeuristic(const glm::ivec2& a, const glm::ivec2& b) {
        return static_cast<float>(std::abs(a.x - b.x) + std::abs(a.y - b.y));
    }

    inline std::vector<glm::vec3> FindPath(GridSystem* gridSystem, glm::ivec2 start, glm::ivec2 end) {
        std::vector<glm::vec3> path;
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
        std::unordered_map<glm::ivec2, Node> allNodes;

        Node startNode(start);
        startNode.gCost = 0.0f;
        startNode.hCost = CalculateHeuristic(start, end);
        startNode.fCost = startNode.hCost;
        startNode.inOpenSet = true;

        openSet.push(startNode);
        allNodes[start] = startNode;

        const std::array<glm::ivec2, 4> neighbors = {
            glm::ivec2(0, 1), glm::ivec2(0, -1), glm::ivec2(1, 0), glm::ivec2(-1, 0)
        };

        while (!openSet.empty()) {
            Node currentNode = openSet.top();
            openSet.pop();

            currentNode.inOpenSet = false;
            currentNode.inClosedSet = true;
            allNodes[currentNode.pos] = currentNode;

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
            }

            for (const auto& dir : neighbors) {
                glm::ivec2 neighborPos = currentNode.pos + dir;

                if (neighborPos != end && !gridSystem->IsWalkable(neighborPos.x, neighborPos.y)) {
                    continue;
                }

                if (allNodes.count(neighborPos) && allNodes[neighborPos].inClosedSet) continue;

                float newGCost = currentNode.gCost + 1.0f;
                Node& neighborNode = allNodes[neighborPos];

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

        return path; //if no path fpund
    }

}