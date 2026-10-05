#include <dilithium/core/Assert.hpp>
#include <dilithium/core/Log.hpp>
#include <dilithium/scenes/Scene.hpp>
#include <dilithium/scenes/SceneManager.hpp>
#include <dilithium/scenes/SceneServices.hpp>

#include <memory>
#include <utility>

namespace dilithium {

SceneManager::SceneManager(Renderer& renderer, Application& app)
    : services{.renderer = renderer, .scenes = *this, .app = app} {}

SceneManager::~SceneManager() {
    // Top down, the reverse of how they were built: a vector would destroy them front to back.
    while (!stack.empty()) {
        stack.pop_back();
    }
}

void SceneManager::add(SceneKey key, const Builder& builder) {
    const auto [position, inserted] = builders.try_emplace(key, builder);
    if (!inserted) {
        DILITHIUM_UNREACHABLE("a scene ID was registered twice");
    }
}

void SceneManager::start(SceneKey key) {
    if (!stack.empty()) {
        DILITHIUM_UNREACHABLE("SceneManager::start called twice; use set or push after the first scene");
    }
    stack.push_back(build(key));
}

void SceneManager::set(SceneKey key) {
    request(Request::Set, key);
}

void SceneManager::push(SceneKey key) {
    request(Request::Push, key);
}

void SceneManager::pop() {
    if (stack.size() < 2) {
        DILITHIUM_UNREACHABLE("SceneManager::pop with no scene underneath to return to");
    }
    request(Request::Pop, 0);
}

void SceneManager::request(Request request, SceneKey key) {
    if (stack.empty()) {
        DILITHIUM_UNREACHABLE("a scene transition was requested before SceneManager::start");
    }
    if (request != Request::Pop && !builders.contains(key)) {
        DILITHIUM_UNREACHABLE("a scene transition names an ID that was never registered");
    }
    if (pending != Request::None) {
        logWarning("a second scene transition in one frame replaces the first");
    }
    pending = request;
    pendingKey = key;
}

std::unique_ptr<Scene> SceneManager::build(SceneKey key) {
    const auto builder = builders.find(key);
    if (builder == builders.end()) {
        DILITHIUM_UNREACHABLE("a scene ID was never registered");
    }
    return builder->second(services);
}

bool SceneManager::performTransition() {
    const Request request = std::exchange(pending, Request::None);
    switch (request) {
    case Request::None:
        return false;
    case Request::Set: {
        // Build first, then destroy top down: whatever the old and new scenes share stays loaded.
        std::unique_ptr<Scene> next = build(pendingKey);
        while (!stack.empty()) {
            stack.pop_back();
        }
        stack.push_back(std::move(next));
        return true;
    }
    case Request::Push:
        stack.push_back(build(pendingKey));
        return true;
    case Request::Pop:
        stack.pop_back();
        getCurrent().resume();
        getCurrent().resize(); // it may have missed one while covered
        return true;
    }
    DILITHIUM_UNREACHABLE("unknown scene transition request");
}

Scene& SceneManager::getCurrent() {
    if (stack.empty()) {
        DILITHIUM_UNREACHABLE("no scene: SceneManager::start was never called");
    }
    return *stack.back();
}

} // namespace dilithium
