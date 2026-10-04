#include <dilithium/core/Assert.hpp>
#include <dilithium/core/Log.hpp>
#include <dilithium/scenes/Scene.hpp>
#include <dilithium/scenes/SceneManager.hpp>
#include <dilithium/scenes/SceneServices.hpp>

#include <memory>
#include <utility>

namespace dilithium {

SceneManager::SceneManager(Renderer& renderer) : m_services{.renderer = renderer, .scenes = *this} {}

SceneManager::~SceneManager() {
    // Top down, the reverse of how they were built: a vector would destroy them front to back.
    while (!m_stack.empty()) {
        m_stack.pop_back();
    }
}

void SceneManager::add(SceneKey key, const Builder& builder) {
    const auto [position, inserted] = m_builders.try_emplace(key, builder);
    if (!inserted) {
        DILITHIUM_UNREACHABLE("a scene ID was registered twice");
    }
}

void SceneManager::start(SceneKey key) {
    if (!m_stack.empty()) {
        DILITHIUM_UNREACHABLE("SceneManager::start called twice; use set or push after the first scene");
    }
    m_stack.push_back(build(key));
}

void SceneManager::set(SceneKey key) {
    request(Request::Set, key);
}

void SceneManager::push(SceneKey key) {
    request(Request::Push, key);
}

void SceneManager::pop() {
    if (m_stack.size() < 2) {
        DILITHIUM_UNREACHABLE("SceneManager::pop with no scene underneath to return to");
    }
    request(Request::Pop, 0);
}

void SceneManager::request(Request request, SceneKey key) {
    if (m_stack.empty()) {
        DILITHIUM_UNREACHABLE("a scene transition was requested before SceneManager::start");
    }
    if (request != Request::Pop && !m_builders.contains(key)) {
        DILITHIUM_UNREACHABLE("a scene transition names an ID that was never registered");
    }
    if (m_pending != Request::None) {
        logWarning("a second scene transition in one frame replaces the first");
    }
    m_pending = request;
    m_pendingKey = key;
}

std::unique_ptr<Scene> SceneManager::build(SceneKey key) {
    const auto builder = m_builders.find(key);
    if (builder == m_builders.end()) {
        DILITHIUM_UNREACHABLE("a scene ID was never registered");
    }
    return builder->second(m_services);
}

bool SceneManager::performTransition() {
    const Request request = std::exchange(m_pending, Request::None);
    switch (request) {
    case Request::None:
        return false;
    case Request::Set: {
        // Build first, then destroy top down: whatever the old and new scenes share stays loaded.
        std::unique_ptr<Scene> next = build(m_pendingKey);
        while (!m_stack.empty()) {
            m_stack.pop_back();
        }
        m_stack.push_back(std::move(next));
        return true;
    }
    case Request::Push:
        m_stack.push_back(build(m_pendingKey));
        return true;
    case Request::Pop:
        m_stack.pop_back();
        current().resume();
        current().resize(); // it may have missed one while covered
        return true;
    }
    DILITHIUM_UNREACHABLE("unknown scene transition request");
}

Scene& SceneManager::current() {
    if (m_stack.empty()) {
        DILITHIUM_UNREACHABLE("no scene: SceneManager::start was never called");
    }
    return *m_stack.back();
}

} // namespace dilithium
