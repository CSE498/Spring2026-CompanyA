#ifdef __EMSCRIPTEN__

#include "../../../source/Agents/PacingAgent.hpp"
#include "../../../source/Interfaces/WebUI/interface/MockWorld.hpp"
#include "../../../source/Interfaces/WebUI/interface/WebInterface.hpp"
#include "../../../third-party/Catch/single_include/catch2/catch.hpp"

#include <iostream>

using namespace cse498;

/// @brief Helper to create a keyboard event
/// @param key The key to simulate
/// @param type The type of keyboard event e.g. keydown or keyup
EM_JS(void, keyboardEvent, (const char* key, const char* type), {
    const keyStr = UTF8ToString(key);
    const typeStr = UTF8ToString(type);
    const event = new KeyboardEvent(typeStr, {
        key : keyStr,
        code : keyStr,
    });
    window.dispatchEvent(event);
});

class TestingMockWorld : MockWorld {
public:
    WorldPosition interfaceStartingPosition = WorldPosition{1, 1};
    MockWorld::ActionType lastAction{MockWorld::ActionType::REMAIN_STILL};
    bool userQuit{false};

    TestingMockWorld() {
        AddAgent<PacingAgent>("Pacer 1").SetLocation(WorldPosition{3, 1});
        AddAgent<PacingAgent>("Pacer 2").SetLocation(WorldPosition{6, 1});
        AddAgent<PacingAgent>("Guard 1").SetHorizontal().SetLocation(WorldPosition{7, 7});
        AddAgent<PacingAgent>("Guard 2").SetHorizontal().ToggleDirection().SetLocation(WorldPosition{8, 8});
        AddInterface<WebInterface>("Web UI").SetSymbol('@').SetLocation(interfaceStartingPosition);
    }

    WorldPosition GetInterfacePosition() { return mInterface->GetLocation().AsWorldPosition(); }

    bool IsPaused() { return mInterface->IsPaused(); }

    /// @brief Runs the main game loop iteration.
    void Run() override {
        size_t action_id = mInterface->SelectAction(main_grid);
        int result = DoAction(*mInterface, action_id);
        if (action_id == QUIT)
            Teardown();
        lastAction = static_cast<MockWorld::ActionType>(action_id);
        mInterface->SetActionResult(result);
        if (!mInterface->IsPaused()) {
            RunAgents();
        }
        mInterface->RenderFrame();
    }

    void Teardown() override { userQuit = true; }
};

TEST_CASE("WebInterface Basic Movement Test") {
    TestingMockWorld world{};

    CHECK(world.GetInterfacePosition() == world.interfaceStartingPosition);

    keyboardEvent("s", "keydown");
    world.Run();
    keyboardEvent("s", "keyup");

    CHECK(world.GetInterfacePosition() == world.interfaceStartingPosition.Down());

    keyboardEvent("W", "keydown");
    world.Run();
    keyboardEvent("w", "keyup");

    CHECK(world.GetInterfacePosition() == world.interfaceStartingPosition);

    keyboardEvent("d", "keydown");
    world.Run();
    keyboardEvent("D", "keyup");

    CHECK(world.GetInterfacePosition() == world.interfaceStartingPosition.Right());

    keyboardEvent("A", "keydown");
    world.Run();
    keyboardEvent("a", "keyup");

    CHECK(world.GetInterfacePosition() == world.interfaceStartingPosition);
}

TEST_CASE("WebInterface Multiple Keys Pressed and Released") {
    TestingMockWorld world{};

    CHECK(world.GetInterfacePosition() == world.interfaceStartingPosition);

    keyboardEvent("S", "keydown");
    world.Run();

    CHECK(world.GetInterfacePosition() == world.interfaceStartingPosition.Down());

    keyboardEvent("w", "keydown");
    world.Run();

    CHECK(world.GetInterfacePosition() == world.interfaceStartingPosition);

    keyboardEvent("D", "keydown");
    world.Run();

    CHECK(world.GetInterfacePosition() == world.interfaceStartingPosition.Right());

    keyboardEvent("a", "keydown");
    world.Run();

    CHECK(world.GetInterfacePosition() == world.interfaceStartingPosition);

    keyboardEvent("a", "keyup");
    keyboardEvent("w", "keyup");
    world.Run();

    CHECK(world.GetInterfacePosition() == world.interfaceStartingPosition.Right());

    keyboardEvent("d", "keyup");
    world.Run();

    CHECK(world.GetInterfacePosition() == world.interfaceStartingPosition.Right().Down());
}

TEST_CASE("WebInterface Interaction Action") {
    TestingMockWorld world{};

    keyboardEvent("e", "keydown");
    world.Run();
    keyboardEvent("e", "keyup");

    CHECK(world.lastAction == MockWorld::ActionType::INTERACT);

    world.Run();

    CHECK(world.lastAction == MockWorld::ActionType::REMAIN_STILL);
}

TEST_CASE("WebInterface Pausing Action") {
    TestingMockWorld world{};

    keyboardEvent("Escape", "keydown");
    world.Run();

    CHECK(world.IsPaused());
    CHECK(world.lastAction == MockWorld::ActionType::REMAIN_STILL);

    keyboardEvent("E", "keydown");
    world.Run();
    keyboardEvent("E", "keyup");

    CHECK(world.IsPaused());
    CHECK(world.lastAction == MockWorld::ActionType::REMAIN_STILL);

    keyboardEvent("Escape", "keyup");
    world.Run();

    keyboardEvent("e", "keydown");
    world.Run();
    keyboardEvent("e", "keyup");

    CHECK(world.IsPaused());
    CHECK(world.lastAction == MockWorld::ActionType::REMAIN_STILL);

    keyboardEvent("Escape", "keydown");
    world.Run();
    keyboardEvent("Escape", "keyup");

    CHECK(!world.IsPaused());
    CHECK(world.lastAction == MockWorld::ActionType::REMAIN_STILL);

    keyboardEvent("e", "keydown");
    world.Run();
    keyboardEvent("e", "keyup");

    CHECK(!world.IsPaused());
    CHECK(world.lastAction == MockWorld::ActionType::INTERACT);
}

TEST_CASE("WebInterface Quit Action") {
    TestingMockWorld world{};

    keyboardEvent("q", "keydown");
    world.Run();
    keyboardEvent("Q", "keyup");

    CHECK(world.lastAction == MockWorld::ActionType::QUIT);
    CHECK(world.userQuit);
}

TEST_CASE("WebInterface Unknown Key Handling") {
    TestingMockWorld world{};
    WorldPosition startPos = world.GetInterfacePosition();

    std::vector<std::string> unknownKeys = {"p", "Enter", "Shift", "7", " ", "z"};

    for (const std::string& key: unknownKeys) {
        keyboardEvent(key.c_str(), "keydown");
        world.Run();

        CHECK(world.GetInterfacePosition() == startPos);
        CHECK(world.lastAction == MockWorld::ActionType::REMAIN_STILL);
        CHECK(!world.IsPaused());
        CHECK(!world.userQuit);

        keyboardEvent(key.c_str(), "keyup");
        world.Run();
    }
}

#endif
