/**
 * @file main.cpp
 * @author Tim Miao
 */

#include "character.cpp"
#include <iostream>

using namespace std;

int main()
{
    character ch;
    cout << ch.selectCharacter("Warrior") << endl;

    return 0;
}