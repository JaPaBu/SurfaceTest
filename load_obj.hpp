#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <map>
#include <sstream>
#include <set>

class model {
public:
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> textureCoords;
    std::vector<glm::vec3> normals;
    std::vector<uint32_t> indices;
    std::map<uint32_t, std::set<uint32_t>> neighbors;
    std::map<std::pair<uint32_t, uint32_t>, std::set<uint32_t>> edgeOpposites;
};

std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

model load_obj(std::string filename) {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> textureCoords;
    std::vector<glm::vec3> normals;
    std::vector<uint32_t> indices;

    std::map<std::string, uint32_t> indexMap;

    model m;

    auto parse_vertex = [&](std::string text) {
//        auto index = indexMap.find(text);
//        if (index != indexMap.end()) {
//            return index->second;
//        }

        auto splits = split(text, '/');

        auto index = indexMap.find(splits[0]);
        if (index != indexMap.end()) {
            return index->second;
        }

        auto vi = std::stoi(splits[0]) - 1;
        auto ti = std::stoi(splits[1]) - 1;
        auto ni = std::stoi(splits[2]) - 1;

        uint32_t i = m.vertices.size();

        m.vertices.emplace_back(vertices[vi]);
        m.textureCoords.emplace_back(textureCoords[ti]);
        m.normals.emplace_back(normals[ni]);

        //indexMap.insert({text, i});
        indexMap.insert({splits[0], i});

        return i;
    };

    std::ifstream t(filename);

    std::string line;
    while (std::getline(t, line)) {
        std::istringstream ss(line);
        std::string c;
        ss >> c;

        if (c == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            vertices.emplace_back(x, y, z);
        } else if (c == "vt") {
            float x, y, z;
            ss >> x >> y >> z;
            textureCoords.emplace_back(x, y);
        } else if (c == "vn") {
            float x, y, z;
            ss >> x >> y >> z;
            normals.emplace_back(x, y, z);
        } else if (c == "f") {
            std::string a, b, c;
            ss >> a >> b >> c;

            auto ai = parse_vertex(a);
            auto bi = parse_vertex(b);
            auto ci = parse_vertex(c);

            m.neighbors.insert({ai, std::set<uint32_t>()}).first->second.insert({bi, ci});
            m.neighbors.insert({bi, std::set<uint32_t>()}).first->second.insert({ai, ci});
            m.neighbors.insert({ci, std::set<uint32_t>()}).first->second.insert({ai, bi});

            m.indices.emplace_back(ai);
            m.indices.emplace_back(bi);
            m.indices.emplace_back(ci);

            m.edgeOpposites.insert({{ai, bi}, std::set<uint32_t>()}).first->second.insert(ci);
            m.edgeOpposites.insert({{bi, ci}, std::set<uint32_t>()}).first->second.insert(ai);
            m.edgeOpposites.insert({{ci, ai}, std::set<uint32_t>()}).first->second.insert(bi);

            m.edgeOpposites.insert({{bi, ai}, std::set<uint32_t>()}).first->second.insert(ci);
            m.edgeOpposites.insert({{ci, bi}, std::set<uint32_t>()}).first->second.insert(ai);
            m.edgeOpposites.insert({{ai, ci}, std::set<uint32_t>()}).first->second.insert(bi);
        }
    }


    return m;
}