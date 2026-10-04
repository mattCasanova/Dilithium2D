#pragma once

namespace dilithium {

/// One screen or state of the game: a menu, the play field, a pause overlay. The game subclasses it and registers
/// the subclass with `SceneManager::add`; the manager builds it with `SceneType(SceneServices&)` and destroys it when
/// it is replaced or popped. The constructor is the setup and the destructor the shutdown, so there is no `init` or
/// `shutdown` to forget; the hooks below are for what is neither.
class Scene {
public:
    Scene() = default;
    virtual ~Scene() = default;

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&) = delete;
    Scene& operator=(Scene&&) = delete;

    /// Once per frame. `dt` is in seconds, clamped so a stall (a breakpoint, a window drag) moves the game one step.
    virtual void update(float dt) = 0;

    /// Once per frame, after `update`: record into the `Renderer` what this frame shows. The engine runs the frame
    /// around it; a scene never begins or ends a pass.
    virtual void draw() = 0;

    /// The window's pixel size changed. Also called after a pop, since the scene below may have missed one.
    virtual void resize() {}

    /// Back on top: the scene pushed over this one was popped.
    virtual void resume() {}
};

} // namespace dilithium
