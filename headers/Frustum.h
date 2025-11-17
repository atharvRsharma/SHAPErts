#pragma once

#include <glm/glm.hpp>

// Defines one of the 6 planes of the frustum
struct FrustumPlane {
    glm::vec3 normal = { 0.0f, 1.0f, 0.0f };
    float distance = 0.0f; // Distance from origin

    // Get signed distance from a point to the plane
    float getSignedDistance(const glm::vec3& point) const {
        return glm::dot(normal, point) + distance;
    }
};

class Frustum {
public:
    FrustumPlane top, bottom, left, right, near, far;

    // This is the magic: it extracts the 6 planes from the combined View-Projection matrix
    void ExtractPlanes(const glm::mat4& vpMatrix) {
        // Left plane
        left.normal.x = vpMatrix[0][3] + vpMatrix[0][0];
        left.normal.y = vpMatrix[1][3] + vpMatrix[1][0];
        left.normal.z = vpMatrix[2][3] + vpMatrix[2][0];
        left.distance = vpMatrix[3][3] + vpMatrix[3][0];

        // Right plane
        right.normal.x = vpMatrix[0][3] - vpMatrix[0][0];
        right.normal.y = vpMatrix[1][3] - vpMatrix[1][0];
        right.normal.z = vpMatrix[2][3] - vpMatrix[2][0];
        right.distance = vpMatrix[3][3] - vpMatrix[3][0];

        // Bottom plane
        bottom.normal.x = vpMatrix[0][3] + vpMatrix[0][1];
        bottom.normal.y = vpMatrix[1][3] + vpMatrix[1][1];
        bottom.normal.z = vpMatrix[2][3] + vpMatrix[2][1];
        bottom.distance = vpMatrix[3][3] + vpMatrix[3][1];

        // Top plane
        top.normal.x = vpMatrix[0][3] - vpMatrix[0][1];
        top.normal.y = vpMatrix[1][3] - vpMatrix[1][1];
        top.normal.z = vpMatrix[2][3] - vpMatrix[2][1];
        top.distance = vpMatrix[3][3] - vpMatrix[3][1];

        // Near plane
        near.normal.x = vpMatrix[0][3] + vpMatrix[0][2];
        near.normal.y = vpMatrix[1][3] + vpMatrix[1][2];
        near.normal.z = vpMatrix[2][3] + vpMatrix[2][2];
        near.distance = vpMatrix[3][3] + vpMatrix[3][2];

        // Far plane
        far.normal.x = vpMatrix[0][3] - vpMatrix[0][2];
        far.normal.y = vpMatrix[1][3] - vpMatrix[1][2];
        far.normal.z = vpMatrix[2][3] - vpMatrix[2][2];
        far.distance = vpMatrix[3][3] - vpMatrix[3][2];

        // Normalize all planes
        auto normalize = [](FrustumPlane& p) {
            float len = glm::length(p.normal);
            p.normal /= len;
            p.distance /= len;
            };
        normalize(left);
        normalize(right);
        normalize(bottom);
        normalize(top);
        normalize(near);
        normalize(far);
    }

    // The core culling function
    bool IsSphereInFrustum(const glm::vec3& center, float radius) const {
        // If the sphere is *fully behind* any single plane, it's outside
        if (left.getSignedDistance(center) < -radius) return false;
        if (right.getSignedDistance(center) < -radius) return false;
        if (bottom.getSignedDistance(center) < -radius) return false;
        if (top.getSignedDistance(center) < -radius) return false;
        if (near.getSignedDistance(center) < -radius) return false;
        if (far.getSignedDistance(center) < -radius) return false;

        // Otherwise, it's at least partially inside
        return true;
    }
};