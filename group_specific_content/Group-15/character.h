/**
 * @file character.h
 * @author Tim Miao
 *
 *
 */

#ifndef CSE_498_PROJECT_CHARACTER_H
#define CSE_498_PROJECT_CHARACTER_H

#include <vector>
#include <string>

struct chType
{
    std::string characterType;
    std::string characterSkill;
    std::string initWeapons;
    std::string initArmor;

    int level;
    int hp;
    int magic;
    int stamina;
    int strength;
    int dexterity;
    int intelligence;
    int chSkillLevel;
};

struct npcType
{
    std::string npcName;
    std::string comeFrom;
    std::string npcCategory;
    std::string npcSkill;
    std::string npcArmor;
    std::string npcWeapons;

    int hp;
    int basicDamage;
    int skillDamage;
    int friendshipLevel;

};

struct enType
{
    std::string enemyType;
    std::string enemySkill;
    std::string itemDrop;

    int difficultyLevel;
    int hp;
    int normalDamage;
    int skillDamage;
};

class character
{
private:
    std::vector<std::string> mNpcVec;
    std::vector<std::string> mEnemyVec;
    std::vector<std::string> mBossVec;
public:

    std::string selectCharacter(const std::string & characterType);
    std::string displayCharacterStats(const chType & ch);

    chType warrior();
    chType magician();
    chType assassin();

};

#endif //CSE_498_PROJECT_CHARACTER_H