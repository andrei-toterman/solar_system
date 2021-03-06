#pragma once

#include <GL/glew.h>

#include <vector>
#include "state.hpp"
#include "render_object.hpp"

struct Planet : RenderObject {
    Planet(const char* texture_path, glm::vec3 _position, float _radius, float _rotation_speed,
           glm::vec3 _rotation_axis);

    Planet(Planet& _parent, const char* texture_path, float _orbit_radius, float _radius, float _rotation_speed,
           glm::vec3 _rotation_axis, float _orbit_speed, glm::vec3 _orbit_axis = { 0.0f, 1.0f, 0.0f });

    explicit Planet(const char* texture_path);

    [[nodiscard]] glm::vec3 absolute_position(float base_orbit_radius) const;

    [[nodiscard]] glm::mat4 model_matrix(const State& state) const override;

    void update(const State& state) override;

    void render() const override;

    Planet* parent{};

    // position is relative to parent or absolute if no parent
    glm::vec3 position{};
    float     radius{};

    // rotation around own axis
    float     rotation_angle{};
    glm::vec3 rotation_axis{};
    float     rotation_speed{};

    // orbit around parent, if it has one
    // orbiting doesn't change position relative to parent, only the orbit angle
    float     orbit_angle{};
    glm::vec3 orbit_axis{};
    float     orbit_speed{};
};
