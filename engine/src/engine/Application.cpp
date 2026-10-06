#include <dilithium/engine/Application.hpp>
#include <dilithium/engine/DefaultEngine.hpp>
#include <dilithium/engine/Engine.hpp>
#include <dilithium/gfx/Renderer.hpp>
#include <dilithium/gfx/renderers/DefaultRenderer.hpp>
#include <dilithium/platform/AppState.hpp>
#include <dilithium/platform/Window.hpp>
#include <dilithium/scenes/SceneManager.hpp>
#include <dilithium/utilities/Assert.hpp>
#include <dilithium/utilities/CommandLine.hpp>
#include <dilithium/utilities/Log.hpp>

#include <memory>
#include <utility>

namespace dilithium {

Application::Application(CommandLine newCommandLine) : commandLine(std::move(newCommandLine)) {}

// The members go in reverse order: the engine (its scenes, then its renderer), then the window.
Application::~Application() = default;

void Application::start() {
    if (engine) {
        DILITHIUM_UNREACHABLE("Application::start() called twice");
    }
    window = std::make_unique<Window>(windowConfig());
    std::unique_ptr<Renderer> renderer = createRenderer(*window);
    if (!renderer) {
        DILITHIUM_UNREACHABLE("createRenderer returned null");
    }
    engine = createEngine(std::move(renderer));
    if (!engine) {
        DILITHIUM_UNREACHABLE("createEngine returned null");
    }
    engine->setPausesWhenInactive(pausesWhenInactive);
    engine->addAppStateObserver([this](AppState state) { appStateChanged(state); });
    registerScenes(engine->getScenes());
}

FrameResult Application::frame() {
    return getEngine().frame();
}

void Application::windowChanged() {
    getEngine().appStateReported(getWindow().getAppState());
}

void Application::resized() {
    const PixelSize pixels = getWindow().getPixelSize();
    DILITHIUM_LOG_DEBUG("window is {}x{} pixels", pixels.width, pixels.height);
    getEngine().resized();
}

void Application::quitRequested() {
    getEngine().quitRequested();
}

void Application::quit() {
    getEngine().quit();
}

AppState Application::getState() const {
    if (!engine) {
        DILITHIUM_UNREACHABLE("getState() before start()");
    }
    return engine->getState();
}

void Application::setPausesWhenInactive(bool pauses) {
    pausesWhenInactive = pauses;
    if (engine) {
        engine->setPausesWhenInactive(pauses);
    }
}

Window& Application::getWindow() {
    if (!window) {
        DILITHIUM_UNREACHABLE("the window exists only after start()");
    }
    return *window;
}

Engine& Application::getEngine() {
    if (!engine) {
        DILITHIUM_UNREACHABLE("the engine exists only after start()");
    }
    return *engine;
}

SceneManager& Application::getScenes() {
    return getEngine().getScenes();
}

WindowConfig Application::windowConfig() const {
    return {};
}

std::unique_ptr<Renderer> Application::createRenderer(const Window& forWindow) {
    return std::make_unique<DefaultRenderer>(forWindow);
}

std::unique_ptr<Engine> Application::createEngine(std::unique_ptr<Renderer> renderer) {
    return std::make_unique<DefaultEngine>(std::move(renderer), *this, commandLine);
}

void Application::appStateChanged(AppState /*state*/) {}

} // namespace dilithium
