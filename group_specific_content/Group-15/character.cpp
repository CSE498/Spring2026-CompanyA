/**
 * @file character.cpp
 * @author Tim Miao
 */

#include "character.h"

using namespace std;

std::string character::selectCharacter(const std::string &characterType)
{
    if (characterType == "Warrior")
    {
        return displayCharacterStats(warrior());
    }
    if (characterType == "Magician")
    {
        return displayCharacterStats(magician());
    }
    if (characterType == "Assassin")
    {
        return displayCharacterStats(assassin());
    }
    return "";
}

std::string character::displayCharacterStats(const chType &ch)
{
    std::string description = "Character type: " + ch.characterType + '\n'
                                + "Character Skill: " + ch.characterSkill + "\n"
                                + "Initial weapons: " + ch.initWeapons + '\n'
                                + "Initial armor: " + ch.initArmor + '\n'
                                + "Level: " + std::to_string(ch.level) + '\n'
                                + "HP: " + std::to_string(ch.hp) + "\n"
                                + "Magic: " + std::to_string(ch.magic) + '\n'
                                + "Stamina: " + std::to_string(ch.stamina) + "\n"
                                + "Strength: " + std::to_string(ch.strength) + "\n"
                                + "Dexterity: " + std::to_string(ch.dexterity) + "\n"
                                + "Intelligence: " + std::to_string(ch.intelligence) + "\n"
                                + "Character Skill Level (Maximum Skill Level is 5): " + std::to_string(ch.chSkillLevel);
    return description;
}

chType character::warrior()
{
    return {
    "Warrior", "Wide range attack. ", "Wood Blade and Wood Shield",
        "Wood Armor Sets",
        1, 11, 5, 15, 12, 9, 9, 1};
}

chType character::magician()
{
    return {"Magician", "Magic attack. ", "Wooden Staff",
        "Textile Robe",
        1, 5, 19, 5, 3, 3, 15, 1};
}

chType character::assassin()
{
    return {"Assassin", "Be able to one attack kill the regular enemy without being seen, level up the skill level to be able to assassinate multiple regular enemies at one time. ", "Two Daggers",
        "Fabric Clothes and Mask",
        1, 7, 2, 15, 5, 16, 11, 1};
}
