#include <iostream>

#include "glad/glad.h"

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtc/reciprocal.hpp>

#include <string>
#include <fstream>
#include <streambuf>
#include <exception>
#include <thread>

#include "load_obj.hpp"

static void error_callback(int error, const char *description) {
    std::cerr << "Error: " << description << std::endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

std::string read_file(std::string filename) {
    std::ifstream t(filename);
    return std::string((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
}

bool check_shader(GLuint shader, std::string name) {
    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

        std::string s(infoLog.begin(), infoLog.end());
        std::cerr << name << " compilation failed" << std::endl << s << std::endl;

        glDeleteShader(shader);
        return false;
    }

    return true;
}

bool check_program(GLuint program, std::string name) {
    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

        std::string s(infoLog.begin(), infoLog.end());
        std::cerr << name << " linking failed" << std::endl << s << std::endl;

        glDeleteProgram(program);
        return false;
    }

    return true;
}

void calculate_cot_sums_matrix(const model &m, std::map<std::pair<uint32_t, uint32_t>, float> &cot_sums_matrix) {
    for (uint32_t vi = 0; vi < m.vertices.size(); vi++) {
        const auto &v = m.vertices.at(vi);

        for (const auto &ni : m.neighbors.at(vi)) {
            const auto &n = m.vertices[ni];

            auto nnis = std::set<uint32_t>();

            const auto &vi_to_ni = m.edgeOpposites.at({vi, ni});
            const auto &ni_to_vi = m.edgeOpposites.at({ni, vi});
            nnis.insert(vi_to_ni.begin(), vi_to_ni.end());
            nnis.insert(ni_to_vi.begin(), ni_to_vi.end());

            float cot_sum = 0;
            for (const auto &nni : nnis) {
                const auto &nn = m.vertices[nni];
                const auto &nn_to_v = v - nn;
                const auto &nn_to_n = n - nn;
                const auto theta = glm::angle(glm::normalize(nn_to_v), glm::normalize(nn_to_n));
                cot_sum += glm::cot(theta);
            }

            cot_sums_matrix.insert({{ni, vi}, cot_sum});
            cot_sums_matrix.insert({{vi, ni}, cot_sum});
        }
    }
}

void calculate_mass_matrix(const model &m, std::map<uint32_t, float> &mass_matrix) {
    for (uint32_t vi = 0; vi < m.vertices.size(); vi++) {
        const auto &v = m.vertices.at(vi);

        float two_Ai = 0;
        auto edges_done = std::set<std::pair<uint32_t, uint32_t>>();

        for (const auto &ni : m.neighbors.at(vi)) {
            const auto &n = m.vertices.at(ni);

            auto nnis = std::set<uint32_t>();

            const auto &vi_to_ni = m.edgeOpposites.at({vi, ni});
            const auto &ni_to_vi = m.edgeOpposites.at({ni, vi});
            nnis.insert(vi_to_ni.begin(), vi_to_ni.end());
            nnis.insert(ni_to_vi.begin(), ni_to_vi.end());

            for (const auto &nni : nnis) {
                if (edges_done.count({ni, nni}) > 0) {
                    continue;
                }

                const auto &nn = m.vertices[nni];
                const auto &v_to_n = n - v;
                const auto &v_to_nn = nn - v;
                two_Ai += glm::length(glm::cross(v_to_n, v_to_nn)) / 3;
                edges_done.emplace(ni, nni);
                edges_done.emplace(nni, ni);
            }
        }

        mass_matrix.insert({vi, two_Ai});
    }
}

void update_simulation_part(const std::vector<float> &old_us, std::vector<float> &us, std::vector<float> &vels,
                            const uint32_t &start, const uint32_t &end,
                            const float &dt, const model &m,
                            const std::map<std::pair<uint32_t, uint32_t>, float> &cot_sums_matrix,
                            const std::map<uint32_t, float> &mass_matrix) {
//    for (uint32_t vi = start; vi < end; vi++) {
//        const auto &v = m.vertices.at(vi);
//
//        float sum = 0;
//        for (const auto &ni : m.neighbors.at(vi)) {
//            sum += cot_sums_matrix.at({ni, vi}) * (us[ni] - us[vi]);
//        }
//
//        const auto &L = sum / mass_matrix.at(vi);
//        us[vi] += L * dt;
//    }
}

void abc() {

}

void update_simulation(std::vector<float> &us, std::vector<float> &vels, const float &dt, const model &m,
                       const std::map<std::pair<uint32_t, uint32_t>, float> &cot_sums_matrix,
                       const std::map<uint32_t, float> &mass_matrix) {

    const auto old_us = us;

    auto n = std::thread::hardware_concurrency();
    n = 1;
    std::vector<std::thread> threads(n);
    for (auto i = 0; i < n; i++) {
        uint32_t batch_size = m.vertices.size() / n;

        auto start = i * batch_size;
        auto end = (i + 1) * batch_size;

        if (i == n - 1) {
            end += batch_size % n;
        }

//        auto t = std::thread(update_simulation_part, std::cref(old_us), std::ref(us), std::ref(vels), std::cref(start),
//                    std::cref(end), std::cref(dt), std::cref(m), std::cref(cot_sums_matrix),
//                    std::cref(mass_matrix));




        threads.push_back(std::thread(abc));

//        threads.push_back(std::thread(update_simulation_part, std::cref(old_us), std::ref(us), std::ref(vels), std::cref(start),
//                             std::cref(end), std::cref(dt), std::cref(m), std::cref(cot_sums_matrix),
//                             std::cref(mass_matrix)));
        //update_simulation_part(old_us, us, vels, i * batch_size, (i + 1) * batch_size, dt, m, cot_sums_matrix, mass_matrix);
    }

    for (auto &t : threads) {
        t.join();
    }
}

int main() {
    auto model = load_obj("teapot2.obj");

    auto vertices = model.vertices;
    auto normals = model.normals;
    auto indices = model.indices;

    auto u = std::vector<float>(vertices.size());
    auto vels = std::vector<float>(vertices.size());
//    for (unsigned i = 0; i < model.vertices.size(); i++) {
//        u[i] = (i / 2) % 50 == 0 ? 10 : 0;
//        vels[i] = 0;
//    }

    float nearestValue = -INFINITY;
    uint32_t nearestIndex = 0;
    for (size_t i = 0; i < model.vertices.size(); i++) {
        const auto &v = model.vertices.at(i);
        if (v[2] > nearestValue) {
            nearestValue = v[2];
            nearestIndex = i;
        }
    }

    u[nearestIndex] = 200;

    std::map<std::pair<uint32_t, uint32_t>, float> cot_sums_matrix;
    std::map<uint32_t, float> mass_matrix;

    calculate_cot_sums_matrix(model, cot_sums_matrix);
    calculate_mass_matrix(model, mass_matrix);

    if (!glfwInit()) {
        std::cerr << "glfwInit failed!" << std::endl;
        std::cin.sync();
        std::cin.get();
        return EXIT_FAILURE;
    }

    GLFWwindow *window;
    GLuint vertex_buffer, normal_buffer, index_buffer, u_buffer;
    GLuint vertex_shader, fragment_shader, program;
    GLint mvp_location, pos_location, normal_location, u_location;

    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(1920, 1080, "Simple example", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(0);

    auto vertex_shader_text = read_file("shader.vert");
    auto fragment_shader_text = read_file("shader.frag");

    auto vertex_shader_text_p = vertex_shader_text.c_str();
    auto fragment_shader_text_p = fragment_shader_text.c_str();

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text_p, nullptr);
    glCompileShader(vertex_shader);
    if (!check_shader(vertex_shader, "vertex shader")) return EXIT_FAILURE;

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text_p, nullptr);
    glCompileShader(fragment_shader);
    if (!check_shader(fragment_shader, "fragment shader")) return EXIT_FAILURE;

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    if (!check_program(program, "program")) return EXIT_FAILURE;

    mvp_location = glGetUniformLocation(program, "MVP");
    pos_location = glGetAttribLocation(program, "inPos");
    normal_location = glGetAttribLocation(program, "inNormal");
    u_location = glGetAttribLocation(program, "inU");


    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(pos_location);
    glVertexAttribPointer(pos_location, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glGenBuffers(1, &normal_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(normals[0]), normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(normal_location);
    glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &u_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, u_buffer);
    //glBufferData(GL_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(u_location);
    glVertexAttribPointer(u_location, 1, GL_FLOAT, GL_FALSE, 0, nullptr);

    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        float ratio;
        int width, height;
        glm::mat4 m, v, p, mvp;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = (float) width / (float) height;

        update_simulation(u, vels, 0.0005f, model, cot_sums_matrix, mass_matrix);

        glBindBuffer(GL_ARRAY_BUFFER, u_buffer);
        glBufferData(GL_ARRAY_BUFFER, u.size() * sizeof(u[0]), u.data(), GL_DYNAMIC_DRAW);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m = glm::identity<glm::mat4>();
        //m = glm::scale(m, glm::vec3(glm::sin(glfwGetTime()), -2*glm::sin(glfwGetTime()*0.3), glm::cos(glfwGetTime())*0.551));
        //m = glm::rotate(m, (float) glfwGetTime(), glm::vec3(1, 1, 1));
        //m = glm::rotate(m, (float) glfwGetTime() / 20, glm::vec3(0, 1, 0));

        v = glm::identity<glm::mat4>();
        v = glm::translate(v, glm::vec3(0, 0, -30));

        p = glm::perspective(glm::pi<float>() / 2, ratio, 0.1f, 100.0f);

        mvp = p * v * m;

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
