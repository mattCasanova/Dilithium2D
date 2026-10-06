#pragma once

#include <dilithium/scenes/Scene.hpp>
#include <dilithium/scenes/SceneServices.hpp>
#include <dilithium/utilities/NonMovable.hpp>

#include <concepts>
#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace dilithium {

class AppServices;
class Renderer;

/// A scene ID is any enum the game declares: `enum class SceneId { Menu, Play }`.
template <typename Id>
concept SceneId = std::is_enum_v<Id>;

/// Registers the game's scenes, builds the first, and runs transitions between them. LiquidMetal2D's `SceneManager`
/// and `SceneFactory` in one. Scene IDs are a game enum (`enum class SceneId { Menu, Play }`); any enum works.
///
/// A transition asked for during a frame happens at the start of the next one, so a scene never destroys itself in
/// the middle of its own `update`. The new scene is then updated and drawn in that same frame. A second request in
/// one frame replaces the first, with a warning. Programmer errors (an ID never registered, `pop` with one scene
/// left, a frame with no scene) stop the program at the request through `DILITHIUM_UNREACHABLE`.
class SceneManager : NonMovable {
public:
    SceneManager(Renderer& renderer, AppServices& app);
    ~SceneManager(); ///< destroys the stack top down

    /// Builds a scene from the services. For a scene whose constructor takes more than the services.
    using Builder = std::function<std::unique_ptr<Scene>(SceneServices&)>;

    // --- what the game calls

    /// Registers `SceneType`, built with `SceneType(SceneServices&)` whenever `id` is started, set or pushed.
    template <std::derived_from<Scene> SceneType, SceneId Id>
    void add(Id id) {
        add(key(id), [](SceneServices& given) -> std::unique_ptr<Scene> { return std::make_unique<SceneType>(given); });
    }

    /// Registers `builder` to build the scene whenever `id` is started, set or pushed: for a scene with constructor
    /// arguments of its own (a level number, a window to drive), which the lambda captures.
    template <SceneId Id>
    void add(Id id, const Builder& builder) {
        add(key(id), builder);
    }

    /// Builds the first scene now. Once, before the first frame.
    template <SceneId Id>
    void start(Id id) {
        start(key(id));
    }

    /// Next frame: replace the current scene, and everything under it, with a new `id`. The new scene is built
    /// before the old ones are destroyed, so anything they share stays loaded.
    template <SceneId Id>
    void set(Id id) {
        set(key(id));
    }

    /// Next frame: cover the current scene with a new `id`. The covered scene stays alive, neither updated nor
    /// drawn, until `pop`.
    template <SceneId Id>
    void push(Id id) {
        push(key(id));
    }

    /// Next frame: destroy the top scene; the one below gets `resume()`, then `resize()`.
    void pop();

    // --- what the engine calls, once per frame

    /// Performs the transition asked for last frame, if any. Returns whether one happened.
    bool performTransition();

    /// The scene on top. A programmer error if `start` was never called.
    [[nodiscard]] Scene& getCurrent();

    /// How many scenes are alive: the current one plus those under it.
    [[nodiscard]] int getDepth() const { return static_cast<int>(stack.size()); }

private:
    using SceneKey = std::int64_t; ///< a game enum's value, whatever its underlying type
    enum class Request { None, Set, Push, Pop };

    template <SceneId Id>
    static SceneKey key(Id id) {
        return static_cast<SceneKey>(static_cast<std::underlying_type_t<Id>>(id));
    }

    void add(SceneKey key, const Builder& builder);
    void start(SceneKey key);
    void set(SceneKey key);
    void push(SceneKey key);
    void request(Request request, SceneKey key);
    [[nodiscard]] std::unique_ptr<Scene> build(SceneKey key);

    std::unordered_map<SceneKey, Builder> builders;
    std::vector<std::unique_ptr<Scene>> stack; ///< the current scene is last
    SceneServices services;
    Request pending = Request::None;
    SceneKey pendingKey = 0;
};

} // namespace dilithium
