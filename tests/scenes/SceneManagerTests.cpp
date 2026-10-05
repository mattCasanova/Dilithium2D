#include <dilithium/engine/Application.hpp>
#include <dilithium/gfx/Color.hpp>
#include <dilithium/gfx/Renderer.hpp>
#include <dilithium/math/Types.hpp>
#include <dilithium/platform/AppState.hpp>
#include <dilithium/scenes/Scene.hpp>
#include <dilithium/scenes/SceneManager.hpp>
#include <dilithium/scenes/SceneServices.hpp>
#include <dilithium/utilities/Assert.hpp>

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>
#include <vector>

using dilithium::Application;
using dilithium::AppState;
using dilithium::Color;
using dilithium::FrameOutcome;
using dilithium::QuitResponse;
using dilithium::Renderer;
using dilithium::Scene;
using dilithium::SceneManager;
using dilithium::SceneServices;
using dilithium::Vec2;

namespace {

/// Records what scenes ask of it, and carries the lifecycle log the fake scenes write to, so a test reads as one
/// sequence of events. A scene reaches it by downcasting `services.renderer`, the way a game's scene would reach
/// its own renderer.
class FakeRenderer final : public Renderer {
public:
    void setClearColor(Color color) override { clearColor = color; }
    void drawTriangle(Vec2 /*a*/, Vec2 /*b*/, Vec2 /*c*/, Color /*colorA*/, Color /*colorB*/,
                      Color /*colorC*/) override {
        ++triangles;
    }
    FrameOutcome drawFrame() override { return FrameOutcome::Presented; }

    std::vector<std::string> log;
    Color clearColor;
    int triangles = 0;
};

/// Counts quits instead of quitting.
class FakeApplication final : public Application {
public:
    void quit() override { ++quits; }
    [[nodiscard]] AppState getState() const override { return AppState::Active; }
    int quits = 0;
};

FakeRenderer& fakeRendererOf(SceneServices& services) {
    auto* renderer = dynamic_cast<FakeRenderer*>(&services.renderer);
    if (renderer == nullptr) {
        DILITHIUM_UNREACHABLE("the test scenes need a FakeRenderer");
    }
    return *renderer;
}

enum class SceneId { A, B, C };

/// One fake scene per letter; every lifecycle step appends "<letter> <step>" to the shared log.
template <char Name>
class FakeScene final : public Scene {
public:
    explicit FakeScene(SceneServices& given) : services(given) { note("built"); }
    // NOLINTNEXTLINE(bugprone-exception-escape): it logs; a bad_alloc here ends the test, which is right
    ~FakeScene() override { note("destroyed"); }

    FakeScene(const FakeScene&) = delete;
    FakeScene& operator=(const FakeScene&) = delete;
    FakeScene(FakeScene&&) = delete;
    FakeScene& operator=(FakeScene&&) = delete;

    void update(float /*dt*/) override {
        note("update");
        if (request != nullptr) {
            request(services.scenes);
            request = nullptr;
        }
    }
    void draw() override {
        note("draw");
        services.renderer.setClearColor(Color{.r = 1.0f});
    }
    void resize() override { note("resize"); }
    void resume() override { note("resume"); }
    QuitResponse quitRequested() override {
        note("quit requested");
        if (!handlesQuit) {
            return QuitResponse::Quit; // the base class's default answer, spelled out
        }
        services.app.quit(); // a real scene would prompt first and call this later
        return QuitResponse::Handled;
    }
    void handleQuit() { handlesQuit = true; }

    /// The scene's own transition requests, so a test can ask for one from inside `update` as a game would.
    void onNextUpdate(void (*next)(SceneManager&)) { request = next; }

private:
    void note(const char* step) { fakeRendererOf(services).log.push_back(std::string{Name} + " " + step); }

    SceneServices& services;
    void (*request)(SceneManager&) = nullptr;
    bool handlesQuit = false;
};

using SceneA = FakeScene<'A'>;
using SceneB = FakeScene<'B'>;
using SceneC = FakeScene<'C'>;

/// A manager with A, B and C registered and A started, plus its renderer.
struct Fixture {
    FakeRenderer renderer;
    FakeApplication app;
    SceneManager scenes{renderer, app};

    Fixture() {
        scenes.add<SceneA>(SceneId::A);
        scenes.add<SceneB>(SceneId::B);
        scenes.add<SceneC>(SceneId::C);
        scenes.start(SceneId::A);
    }

    /// One engine frame: the pending transition, then update and draw of whoever is on top.
    void frame() {
        scenes.performTransition();
        scenes.getCurrent().update(0.016f);
        scenes.getCurrent().draw();
    }

    std::vector<std::string>& log() { return renderer.log; }
};

using Log = std::vector<std::string>;

} // namespace

TEST_CASE("start builds the first scene at once; the next frame updates and draws it", "[scenes]") {
    Fixture f;
    CHECK(f.log() == Log{"A built"});
    f.frame();
    CHECK(f.log() == Log{"A built", "A update", "A draw"});
    CHECK(f.renderer.clearColor == Color{.r = 1.0f});
    CHECK(f.scenes.getDepth() == 1);
}

TEST_CASE("set waits for the next frame, and builds the new scene before destroying the old", "[scenes]") {
    Fixture f;
    f.scenes.set(SceneId::B);
    CHECK(f.log() == Log{"A built"}); // nothing yet
    f.frame();
    CHECK(f.log() == Log{"A built", "B built", "A destroyed", "B update", "B draw"});
    CHECK(f.scenes.getDepth() == 1);
}

TEST_CASE("push keeps the covered scene alive but not updated; pop resumes and resizes it", "[scenes]") {
    Fixture f;
    f.scenes.push(SceneId::B);
    f.frame();
    CHECK(f.log() == Log{"A built", "B built", "B update", "B draw"});
    CHECK(f.scenes.getDepth() == 2);

    f.log().clear();
    f.scenes.pop();
    f.frame();
    CHECK(f.log() == Log{"B destroyed", "A resume", "A resize", "A update", "A draw"});
    CHECK(f.scenes.getDepth() == 1);
}

TEST_CASE("set from a pushed scene destroys the whole stack, top down, after building the new one", "[scenes]") {
    Fixture f;
    f.scenes.push(SceneId::B);
    f.frame();
    f.log().clear();

    f.scenes.set(SceneId::C);
    f.frame();
    CHECK(f.log() == Log{"C built", "B destroyed", "A destroyed", "C update", "C draw"});
    CHECK(f.scenes.getDepth() == 1);
}

TEST_CASE("a second request in one frame replaces the first", "[scenes]") {
    Fixture f;
    f.scenes.set(SceneId::B);
    f.scenes.set(SceneId::C); // logs a warning, then wins
    f.frame();
    CHECK(f.log() == Log{"A built", "C built", "A destroyed", "C update", "C draw"});
}

TEST_CASE("performTransition says whether one happened", "[scenes]") {
    Fixture f;
    CHECK_FALSE(f.scenes.performTransition());
    f.scenes.push(SceneId::B);
    CHECK(f.scenes.performTransition());
    CHECK_FALSE(f.scenes.performTransition());
}

TEST_CASE("the stack is destroyed top down with the manager", "[scenes]") {
    FakeRenderer renderer;
    FakeApplication app;
    {
        SceneManager scenes{renderer, app};
        scenes.add<SceneA>(SceneId::A);
        scenes.add<SceneB>(SceneId::B);
        scenes.start(SceneId::A);
        scenes.push(SceneId::B);
        scenes.performTransition();
        renderer.log.clear();
    }
    CHECK(renderer.log == Log{"B destroyed", "A destroyed"});
}

TEST_CASE("a scene can ask for a transition from its own update", "[scenes]") {
    // The request is recorded during update and performed at the start of the next frame, never mid-update.
    Fixture f;
    auto& a = dynamic_cast<SceneA&>(f.scenes.getCurrent());
    a.onNextUpdate([](SceneManager& scenes) { scenes.push(SceneId::B); });
    f.frame();
    CHECK(f.log() == Log{"A built", "A update", "A draw"}); // A finished its frame intact
    f.frame();
    CHECK(f.log().back() == "B draw");
    CHECK(f.scenes.getDepth() == 2);
}

TEST_CASE("a scene answers a quit request: Quit by default, or Handled and it quits itself later", "[scenes]") {
    Fixture f;
    auto& a = dynamic_cast<SceneA&>(f.scenes.getCurrent());
    CHECK(a.quitRequested() == QuitResponse::Quit);
    CHECK(f.app.quits == 0);

    a.handleQuit();
    CHECK(a.quitRequested() == QuitResponse::Handled);
    CHECK(f.app.quits == 1); // the fake quits at once; a game would after its prompt
}

TEST_CASE("a builder lambda registers a scene with arguments of its own", "[scenes]") {
    struct Numbered final : Scene {
        Numbered(SceneServices& /*services*/, int newNumber) : number(newNumber) {}
        void update(float /*dt*/) override {}
        void draw() override {}
        int number;
    };
    enum class Id { Level }; // its own enum, so its own manager: one enum per game, and values are the keys
    FakeRenderer renderer;
    FakeApplication app;
    SceneManager scenes{renderer, app};
    const int level = 7;
    scenes.add(Id::Level, [level](SceneServices& services) -> std::unique_ptr<Scene> {
        return std::make_unique<Numbered>(services, level);
    });
    scenes.start(Id::Level);
    CHECK(dynamic_cast<Numbered&>(scenes.getCurrent()).number == level);
}

TEST_CASE("the base Scene's quitRequested quits", "[scenes]") {
    struct Bare final : Scene {
        void update(float /*dt*/) override {}
        void draw() override {}
    } bare;
    CHECK(bare.quitRequested() == QuitResponse::Quit);
    bare.appStateChanged(AppState::Background); // the default does nothing
}

#ifndef DILITHIUM_DEBUG
#include <stdexcept>

TEST_CASE("programmer errors stop at the request, in release as logic_error", "[scenes][assert]") {
    FakeRenderer renderer;
    FakeApplication app;
    SceneManager scenes{renderer, app};
    scenes.add<SceneA>(SceneId::A);

    CHECK_THROWS_AS(scenes.add<SceneB>(SceneId::A), std::logic_error); // registered twice
    CHECK_THROWS_AS(scenes.getCurrent(), std::logic_error);            // no scene yet
    CHECK_THROWS_AS(scenes.set(SceneId::A), std::logic_error);         // before start
    CHECK_THROWS_AS(scenes.start(SceneId::B), std::logic_error);       // never registered
    scenes.start(SceneId::A);
    CHECK_THROWS_AS(scenes.start(SceneId::A), std::logic_error); // started twice
    CHECK_THROWS_AS(scenes.set(SceneId::C), std::logic_error);   // never registered
    CHECK_THROWS_AS(scenes.pop(), std::logic_error);             // nothing underneath
}
#endif
