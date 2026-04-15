/**
 * @file MovementTypesTest.cpp
 * @author Logan Rimarcik
 */

#include "../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../source/Agents/Classic/MovementTypes.hpp"
#include "../../source/Worlds/DemoG2/WorldActions.hpp"
#include "../../source/tools/PathVector.hpp"

using cse498::MovementTypes;
using cse498::WorldActions;

TEST_CASE("Movement  Types -- Getting Id of movement", "[ids]")
{
    {
        CHECK(MovementTypes::GetActionID(0.0, -1.0) == WorldActions::MOVE_UP);
        CHECK(MovementTypes::GetActionID(cse498::PathVector(0.0, -1.0)) == WorldActions::MOVE_UP);
    }

    {
        CHECK(MovementTypes::GetActionID(0.0, 1.0) == WorldActions::MOVE_DOWN);
        CHECK(MovementTypes::GetActionID(cse498::PathVector(0.0, 1.0)) == WorldActions::MOVE_DOWN);
    }

    {
        CHECK(MovementTypes::GetActionID(-1.0, 0.0) == WorldActions::MOVE_LEFT);
        CHECK(MovementTypes::GetActionID(cse498::PathVector(-1.0, 0.0)) == WorldActions::MOVE_LEFT);
    }

    {
        CHECK(MovementTypes::GetActionID(1.0, 0.0) == WorldActions::MOVE_RIGHT);
        CHECK(MovementTypes::GetActionID(cse498::PathVector(1.0, 0.0)) == WorldActions::MOVE_RIGHT);
    }

    {
        CHECK(MovementTypes::GetActionID(0.0, 0.0) == WorldActions::REMAIN_STILL);
        CHECK(MovementTypes::GetActionID(cse498::PathVector(0.0, 0.0)) == WorldActions::REMAIN_STILL);
        CHECK(MovementTypes::GetActionID(44.0, -29.0) == WorldActions::REMAIN_STILL);
        CHECK(MovementTypes::GetActionID(cse498::PathVector(-392.0, 2.0)) == WorldActions::REMAIN_STILL);
    }
}

TEST_CASE("Movement  Types -- Getting String of movement", "[ids]")
{
    {
        CHECK(MovementTypes::GetActionName(0.0, -1.0) == "w");
        CHECK(MovementTypes::GetActionName(cse498::PathVector(0.0, -1.0)) == "w");
    }

    {
        CHECK(MovementTypes::GetActionName(0.0, 1.0) == "s");
        CHECK(MovementTypes::GetActionName(cse498::PathVector(0.0, 1.0)) == "s");
    }

    {
        CHECK(MovementTypes::GetActionName(-1.0, 0.0) == "a");
        CHECK(MovementTypes::GetActionName(cse498::PathVector(-1.0, 0.0)) == "a");
    }

    {
        CHECK(MovementTypes::GetActionName(1.0, 0.0) == "d");
        CHECK(MovementTypes::GetActionName(cse498::PathVector(1.0, 0.0)) == "d");
    }

    {
        CHECK(MovementTypes::GetActionName(0.0, 0.0) == "stay");
        CHECK(MovementTypes::GetActionName(cse498::PathVector(0.0, 0.0)) == "stay");
    }
}
