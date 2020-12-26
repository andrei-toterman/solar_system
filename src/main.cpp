#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#include "planet.hpp"
#include "shader.hpp"
#include "state.hpp"
#include "camera.hpp"

void mouse_button_callback(GLFWwindow* window, int button, int action, int);

void mouse_callback(GLFWwindow* window, double x, double y);

void scroll_callback(GLFWwindow* window, double, double delta_scroll);

void key_callback(GLFWwindow* window, int key, int, int action, int);

int main() {
    if (!glfwInit()) {
        std::cerr << "failed to initialize GLFW" << std::endl;
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Solar System", nullptr, nullptr);
    if (!window) {
        std::cerr << "failed to create GLFW window" << std::endl;
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cerr << "failed to initialize GLEW" << std::endl;
        return 1;
    }

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    State  state{};
    Camera camera{{ 0.0f, 0.0f, 0.0f }};
    glfwSetWindowUserPointer(window, &state);

    const glm::vec3 UP{ 0.0f, 1.0f, 0.0f };

    // orbit radius: 10^7 km
    // radius: 10^5 km
    // rotation speed: 10^3 km/h
    // orbit speed: 10 km/s
    Planet sun{ "res/sun.jpg", { 0.0f, 0.0f, 0.0f }, 69.5f * 0.2f, 1.99f, UP };
    Planet mercury{ sun, "res/mercury.jpg", 5.7f, 0.24f, 0.01f, UP, 4.74f };
    Planet venus{ sun, "res/venus.jpg", 10.8f, 0.6f, 0.006f, UP, 3.5f };
    Planet earth{ sun, "res/earth.jpg", 14.9f, 0.63f, 1.67f, UP, 2.98f };
    Planet moon{ earth, "res/moon.jpg", 0.38f, 0.17f, 1.0f, UP, 0.1f };
    Planet mars{ sun, "res/mars.jpg", 22.7f, 0.33f, 0.86f, UP, 2.41f };
    Planet jupiter{ sun, "res/jupiter.jpg", 77.8f, 6.9f, 45.58f, UP, 1.31f };
    Planet saturn{ sun, "res/saturn.jpg", 143.4f, 5.82f, 36.84f, UP, 0.97f };
    Planet uranus{ sun, "res/uranus.jpg", 287.1f, 2.53f, 14.79f, UP, 0.68f };
    Planet neptune{ sun, "res/neptune.jpg", 449.5, 2.46f, 9.71f, UP, 0.54f };

    Planet* objects[]{ &sun, &mercury, &venus, &earth, &moon, &mars, &jupiter, &saturn, &uranus, &neptune };

    Shader shader{ "shaders/vertex.glsl", "shaders/fragment.glsl" };
    glUseProgram(shader.id);

    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window)) {
        auto time = (float) glfwGetTime();
        state.delta.time = time - state.last_time;
        state.last_time  = time;

        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.update(state.camera_movement, state.delta);
        glm::mat4 proj{ glm::perspective(glm::radians(camera.fov), (float) WIDTH / HEIGHT, 0.1f, 10000.0f) };
        glm::mat4 view{ camera.view_matrix() };

        for (const auto object : objects) {
            object->update(state.delta.time, state.base_speed);
        }
        for (const auto object : objects) {
            glm::mat4 model{ 1.0f };
            model = glm::translate(model, object->absolute_position(state.base_orbit_radius));
            model = glm::rotate(model, object->rotation_angle, object->rotation_axis);
            model = glm::scale(model, glm::vec3{ object->radius * state.base_radius });
            shader.set_mvp(proj * view * model);
            object->render();
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // settings window
        ImGui::Begin("settings");
        state.mouse_in_settings = ImGui::GetIO().WantCaptureMouse;
        ImGui::SliderFloat("speed", &state.base_speed, 0.0f, 2.0f);
        ImGui::SliderFloat("radius", &state.base_radius, 0.0f, 2.0f);
        ImGui::SliderFloat("distance", &state.base_orbit_radius, 1.0f, 5.0f);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int) {
    auto state = (State*) glfwGetWindowUserPointer(window);
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (!state->mouse_in_settings) {
            if (action == GLFW_PRESS) {
                state->mouse_pressed = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                state->mouse_pressed = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
    }
}

void mouse_callback(GLFWwindow* window, double x, double y) {
    auto state = (State*) glfwGetWindowUserPointer(window);

    if (state->mouse_pressed && !state->mouse_in_settings) {
        state->delta.mouse_x = (float) (x - state->last_mouse_x);
        state->delta.mouse_y = (float) (state->last_mouse_y - y);
    }

    state->last_mouse_x = x;
    state->last_mouse_y = y;
}

void scroll_callback(GLFWwindow* window, double, double delta_scroll) {
    auto state = (State*) glfwGetWindowUserPointer(window);
    state->delta.mouse_scroll = (float) delta_scroll;
}

void key_callback(GLFWwindow* window, int key, int, int action, int) {
    auto state = (State*) glfwGetWindowUserPointer(window);

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (action == GLFW_PRESS || action == GLFW_RELEASE) {
        bool pressed = action == GLFW_PRESS;
        switch (key) {
            case GLFW_KEY_UP:
            case GLFW_KEY_W: state->camera_movement[State::FORWARD] = pressed;
                break;
            case GLFW_KEY_LEFT:
            case GLFW_KEY_A: state->camera_movement[State::LEFT] = pressed;
                break;
            case GLFW_KEY_DOWN:
            case GLFW_KEY_S: state->camera_movement[State::BACKWARD] = pressed;
                break;
            case GLFW_KEY_RIGHT:
            case GLFW_KEY_D: state->camera_movement[State::RIGHT] = pressed;
                break;
            case GLFW_KEY_RIGHT_SHIFT:
            case GLFW_KEY_LEFT_SHIFT: state->camera_movement[State::SLOW] = pressed;
                break;
            default: break;
        }
    }
}
