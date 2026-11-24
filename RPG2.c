#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <string.h>

#define WIDTH 70
#define HEIGHT 40
#define VISION_RADIUS 4
#define SYM_PLAYER   '@'
#define SYM_EMPTY    '.'
#define SYM_MONSTER  'E'
#define SYM_CHEST    'c'
#define SYM_POTION   'H'
#define SYM_GOLD     '$'
#define SYM_BOSS     'B'
#define SYM_FOG      '#'
#define SYM_Princess 'P'



typedef enum {
    CLASS_WARRIOR,
    CLASS_ARCHER,
    CLASS_MAGE
} CharacterClass;

typedef enum {
    WEAPON_MECH,
    WEAPON_LUK,
    WEAPON_POSOH
} WeaponType;

typedef enum {
    ELEMENT_NONE,
    ELEMENT_FIRE,
    ELEMENT_ICE,
    ELEMENT_EARTH,
    ELEMENT_WIND
} ElementType;

typedef struct {
    char *name;
    WeaponType type;
    ElementType element;
} Weapon;

typedef struct {
    CharacterClass chClass;
    int hp;
    int maxHp;
    int gold;
    int potionsCount;
    Weapon inventory[3];
    int activeWeaponIndex;
} Player;

int monsterKillCount=0;
char map[HEIGHT][WIDTH];
int playerX = 35;
int playerY = 20;
Player hero;

float elementMatrix[5][5] = {
    { 1.0,  1.0,  1.0,   1.0,   1.0 },
    { 1.0,  0.0,  1.3,   0.8,   0.8 },
    { 1.0,  1.3,  0.0,   0.8,   0.8 },
    { 1.0,  0.8,  0.8,   0.0,   1.3 },
    { 1.0,  0.8,  0.8,   1.3,   0.0 }
};

char* getElementXName(ElementType el) {
    if (el == ELEMENT_FIRE) return "Fire";
    if (el == ELEMENT_ICE) return "Ice";
    if (el == ELEMENT_EARTH) return "Earth";
    if (el == ELEMENT_WIND) return "Wind";
    return "Normal";
}

char* getTypeXName(WeaponType t) {
    if (t == WEAPON_MECH) return "Sword";
    if (t == WEAPON_LUK) return "Bow";
    return "Staff";
}

void openChest() {
    printf("\033[H\033[J");
    if (hero.gold < 60) {
        printf("=== CHEST ===\n");
        printf("You need 60 Gold to open this chest! You only have %d Gold.\n", hero.gold);
        printf("Press any key to continue...\n");
        _getch();
        return;
    }

    hero.gold -= 60;

    WeaponType newType = rand() % 3;
    ElementType newElement = (rand() % 4) + 1;

    char *wName = malloc(40);
    sprintf(wName, "%s %s", getElementXName(newElement), getTypeXName(newType));

    Weapon newWeapon = {wName, newType, newElement};

    printf("=== CHEST OPENED! ===\n");
    printf("Found: %s (Type: %s, Element: %s)\n\n", newWeapon.name, getTypeXName(newWeapon.type), getElementXName(newWeapon.element));

    int freeSlot = -1;
    for (int i = 0; i < 3; i++) {
        if (strcmp(hero.inventory[i].name, "Empty Slot") == 0) {
            freeSlot = i;
            break;
        }
    }

    if (freeSlot != -1) {
        hero.inventory[freeSlot] = newWeapon;
        printf("Weapon added to slot %d!\n", freeSlot + 1);
    } else {
        printf("Your inventory is FULL! Choose a slot to REPLACE, or press 4 to sell:\n");
        for (int i = 0; i < 3; i++) {
            printf("%d. Replace: %s\n", i + 1, hero.inventory[i].name);
        }
        printf("4. sell new weapon\n");

        char choice = _getch();
        if (choice >= '1' && choice <= '3') {
            int idx = choice - '1';
            printf("Replaced %s with %s!\n", hero.inventory[idx].name, newWeapon.name);
            hero.inventory[idx] = newWeapon;
            hero.gold+=30;
        } else {
            printf("You discarded the new weapon.\n");
            hero.gold+=30;
        }
    }
    printf("\nPress any key to return to map...\n");
    _getch();
}

void selectCharacter() {
    printf("=== SELECT YOUR CLASS ===\n");
    printf("1. Warrior (200 HP, Prefers Swords)\n");
    printf("2. Archer  (150 HP, Prefers Bows)\n");
    printf("3. Mage    (110 HP, Prefers Staffs)\n");
    printf("Enter choice (1-3): ");

    int choice;
    if (scanf("%d", &choice) != 1) choice = 1;

    hero.gold = 60;
    hero.potionsCount = 0;
    hero.activeWeaponIndex = 0;

    if (choice == 1) {
        hero.chClass = CLASS_WARRIOR;
        hero.maxHp = 200;
        hero.hp = 200;
        hero.inventory[0] = (Weapon){"Basic Sword", WEAPON_MECH, ELEMENT_NONE};
    }
    else if (choice == 2) {
        hero.chClass = CLASS_ARCHER;
        hero.maxHp = 150;
        hero.hp = 150;
        hero.inventory[0] = (Weapon){"Basic Bow", WEAPON_LUK, ELEMENT_NONE};
    }
    else {
        hero.chClass = CLASS_MAGE;
        hero.maxHp = 110;
        hero.hp = 110;
        hero.inventory[0] = (Weapon){"Basic Staff", WEAPON_POSOH, ELEMENT_NONE};
    }

    hero.inventory[1] = (Weapon){"Empty Slot", WEAPON_MECH, ELEMENT_NONE};
    hero.inventory[2] = (Weapon){"Empty Slot", WEAPON_MECH, ELEMENT_NONE};

    printf("\nClass selected! Press any key to enter the Great World...\n");
    _getch();
}

void initMap() {
    srand(time(NULL));

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            map[y][x] = SYM_EMPTY;
        }
    }

    map[2][WIDTH - 3] = SYM_BOSS;

    int monstersCount = 200;
    int chestsCount = 100;
    int potionsCount = 80;
    int goldCount = 40;

    while (monstersCount > 0) {
        int x = rand() % WIDTH;
        int y = rand() % HEIGHT;

        if (map[y][x] == SYM_EMPTY && (x != playerX || y != playerY)) {
            map[y][x] = SYM_MONSTER;
            monstersCount--;
        }
    }

    while (chestsCount > 0) {
        int x = rand() % WIDTH;
        int y = rand() % HEIGHT;
        if (map[y][x] == SYM_EMPTY && (x != playerX || y != playerY)) {
            map[y][x] = SYM_CHEST;
            chestsCount--;
        }
    }

    while (potionsCount > 0) {
        int x = rand() % WIDTH;
        int y = rand() % HEIGHT;
        if (map[y][x] == SYM_EMPTY && (x != playerX || y != playerY)) {
            map[y][x] = SYM_POTION;
            potionsCount--;
        }
    }

    while (goldCount > 0) {
        int x = rand() % WIDTH;
        int y = rand() % HEIGHT;
        if (map[y][x] == SYM_EMPTY && (x != playerX || y != playerY)) {
            map[y][x] = SYM_GOLD;
            goldCount--;
        }
    }
    int xP = rand() % WIDTH;
    int yP = rand() % HEIGHT;

    if (map[yP][xP] == SYM_EMPTY && (xP != playerX || yP != playerY)) {
        map[yP][xP] = SYM_Princess;
    }
}

void drawMap() {
    printf("\033[H\033[J");

    printf("=== GREAT RPG WORLD (70x40) ===\n");
    printf("Controls: WASD. Inventory/Sell: I. Quit: Q. Heal H\n\n");

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (x == playerX && y == playerY) {
                printf("%c ", SYM_PLAYER);
                continue;
            }

            if (x >= playerX - VISION_RADIUS && x <= playerX + VISION_RADIUS &&
                y >= playerY - VISION_RADIUS && y <= playerY + VISION_RADIUS) {
                printf("%c ", map[y][x]);
            } else {
                printf("%c ", map[y][x]);
                //printf("%c ", SYM_FOG);
            }
        }
        printf("\n");
    }
    printf("\nHP: %d/%d | Gold: %d | Potions: %d\n", hero.hp, hero.maxHp, hero.gold, hero.potionsCount);
    printf("Equipped Weapon: %s\n", hero.inventory[hero.activeWeaponIndex].name);
    printf("Coordinates: [%d, %d]\n", playerX, playerY);
}

void showInventoryAndSell() {
    while (1) {
        printf("\033[H\033[J");
        printf("=== INVENTORY & EQUIPMENT ===\n");
        for (int i = 0; i < 3; i++) {
            printf("%d. [%s] Type: %s | Element: %s",
                   i + 1,
                   hero.inventory[i].name,
                   getTypeXName(hero.inventory[i].type),
                   getElementXName(hero.inventory[i].element));
            if (i == hero.activeWeaponIndex) printf(" (Equipped)");
            printf("\n");
        }

        printf("\nPress 1-3 to EQUIP weapon\n");
        printf("Press S to enter SELL mode (Get 30 Gold)\n");
        printf("Press any other key to close inventory\n");

        char choice = _getch();
        if (choice >= '1' && choice <= '3') {
            int idx = choice - '1';
            if (strcmp(hero.inventory[idx].name, "Empty Slot") == 0) {
                printf("Cannot equip an empty slot!\n");
                _getch();
            } else {
                hero.activeWeaponIndex = idx;
                printf("Equipped %s!\n", hero.inventory[idx].name);
                _getch();
            }
        }
        else if (choice == 's' || choice == 'S') {
            printf("\n[SELL MODE] Select 1-3 to sell weapon for 30 Gold: ");
            char sellChoice = _getch();
            if (sellChoice >= '1' && sellChoice <= '3') {
                int idx = sellChoice - '1';
                if (strcmp(hero.inventory[idx].name, "Empty Slot") == 0) {
                    printf("\nSlot is already empty!\n");
                    _getch();
                } else if (idx == hero.activeWeaponIndex) {
                    printf("\nCannot sell currently equipped weapon!\n");
                    _getch();
                } else {
                    hero.gold += 30;
                    printf("\nSold %s for 30 Gold!\n", hero.inventory[idx].name);
                    hero.inventory[idx] = (Weapon){"Empty Slot", WEAPON_MECH, ELEMENT_NONE};
                    _getch();
                }
            }
        }
        else {
            break;
        }
    }
}

int startCombat() {
    int enemyHp = 60 + rand() % 21;
    ElementType enemyElement = (rand() % 4) + 1;

    printf("\033[H\033[J");
    printf(" !!! BATTLE STARTED !!! \n");
    printf("Enemy Element: %s\n", getElementXName(enemyElement));
    printf("Press any key to enter tactical screen...\n");
    _getch();

    while (hero.hp > 0 && enemyHp > 0) {
        int playerTurns = (hero.chClass == CLASS_WARRIOR) ? 1 : 2;
        int fled = 0;

        for (int t = 0; t < playerTurns; t++) {
            if (enemyHp <= 0) break;

            printf("\033[H\033[J");
            printf("=== COMBAT ROUND ===\n");
            printf("PLAYER: HP %d/%d | Active Weapon: %s (%s)\n",
                   hero.hp, hero.maxHp, hero.inventory[hero.activeWeaponIndex].name,
                   getElementXName(hero.inventory[hero.activeWeaponIndex].element));
            printf("ENEMY:  HP %d | Element: %s\n", enemyHp, getElementXName(enemyElement));
            printf("----------------------------------------\n");
            printf("Action (%d/%d):\n", t + 1, playerTurns);
            printf("1. Attack\n");
            printf("2. Drink Potion\n");
            printf("3. Change Weapon\n");
            printf("4. Flee (30%% Chance)\n");

            char action = _getch();

            if (action == '1') {
                int baseDmg = 10 + rand() % 11;
                float modifier = 1.0f;

                if ((hero.chClass == CLASS_WARRIOR && hero.inventory[hero.activeWeaponIndex].type == WEAPON_MECH) ||
                    (hero.chClass == CLASS_ARCHER && hero.inventory[hero.activeWeaponIndex].type == WEAPON_LUK) ||
                    (hero.chClass == CLASS_MAGE && hero.inventory[hero.activeWeaponIndex].type == WEAPON_POSOH)) {
                    modifier += 0.40f;
                }

                ElementType playerElement = hero.inventory[hero.activeWeaponIndex].element;
                float elementMod = elementMatrix[playerElement][enemyElement];
                int finalDmg = (int)(baseDmg * modifier * elementMod);

                printf("\nYou strike with %s!\n", hero.inventory[hero.activeWeaponIndex].name);
                if (elementMod == 0.0f) {
                    printf(" IMMUNE! This element deals 0 damage! Change weapon!\n");
                    finalDmg = 0;
                } else if (elementMod > 1.1f) {
                    printf(" ELEMENTAL CRITICAL!\n");
                } else if (elementMod < 0.9f && playerElement != ELEMENT_NONE) {
                    printf("️ Element is ineffective...\n");
                }

                enemyHp -= finalDmg;
                printf("Dealt %d damage!\n", finalDmg);
                _getch();
            }
            else if (action == '2') {
                if(hero.potionsCount>0){
                int heal = 20 + rand() % 21;
                hero.hp += heal;
                hero.potionsCount--;
                if (hero.hp > hero.maxHp) hero.hp = hero.maxHp;
                printf("\nYou healed for %d HP!\n", heal);
                _getch();
                }else{
                printf("You don't have potinos");
                }
            }
            else if (action == '3') {
                printf("\n--- QUICK CHANGE ---\n");
                for (int i = 0; i < 3; i++) {
                    printf("%d. %s\n", i + 1, hero.inventory[i].name);
                }
                char wpChoice = _getch();
                if (wpChoice >= '1' && wpChoice <= '3') {
                    int idx = wpChoice - '1';
                    if (strcmp(hero.inventory[idx].name, "Empty Slot") != 0) {
                        hero.activeWeaponIndex = idx;
                        printf("Switched to %s!\n", hero.inventory[idx].name);
                    } else {
                        printf("Slot is empty!\n");
                    }
                }
                _getch();
            }
            else if (action == '4') {
                if ((rand() % 100) < 30) {
                    printf("\n You escaped from battle!\n");
                    _getch();
                    fled = 1;
                    break;
                } else {
                    printf("\nFlee failed!\n");
                    _getch();
                }
            }
        }

        if (fled) return 2;
        if (enemyHp <= 0){
            monsterKillCount++;
            break;}

        printf("\n Enemy attacks!\n");
        int enemyDmg = 20 + rand() % 9;
        if (hero.chClass == CLASS_WARRIOR) {
            enemyDmg = (int)(enemyDmg * 0.70);
            printf(" Warrior parried 30%% damage.\n");
        }
        hero.hp -= enemyDmg;
        printf("Enemy dealt %d damage to you!\n", enemyDmg);
        _getch();
    }

    if (hero.hp <= 0) {
        return 0;
    } else {
        int goldFound = 10 + rand() % 15;
        hero.gold += goldFound;
        printf("\n VICTORY! Enemy defeated!\n");
        _getch();
        return 1;
    }
}

int startCombatB(int isBoss) {
    int enemyHp = 180 + (rand() % 21);
    ElementType enemyElement = (rand() % 4) + 1;

    printf("\033[H\033[J");
    if (isBoss) {
        printf(" !!! FINAL BOSS BATTLE !!! \n");
    } else {
        printf(" !!! BATTLE STARTED !!! \n");
    }
    printf("Enemy Element: %s\n", getElementXName(enemyElement));
    printf("Press any key to enter tactical screen...\n");
    _getch();

    while (hero.hp > 0 && enemyHp > 0) {
        int playerTurns = (hero.chClass == CLASS_WARRIOR) ? 1 : 2;

        for (int t = 0; t < playerTurns; t++) {
            if (enemyHp <= 0) break;

            int actionTaken = 0;
            while (!actionTaken) {
                printf("\033[H\033[J");
                if (isBoss) printf("=== SUPREME BOSS COGNITION ===\n");
                else printf("=== COMBAT ROUND ===\n");

                printf("PLAYER: HP %d/%d | Active Weapon: %s (%s)\n",
                       hero.hp, hero.maxHp, hero.inventory[hero.activeWeaponIndex].name,
                       getElementXName(hero.inventory[hero.activeWeaponIndex].element));
                printf("ENEMY:  HP %d | Element: %s\n", enemyHp, getElementXName(enemyElement));
                printf("----------------------------------------\n");
                printf("Action (%d/%d):\n", t + 1, playerTurns);
                printf("1. Attack\n");
                printf("2. Drink Potion (Potions left: %d)\n", hero.potionsCount);
                printf("3. Change Weapon (Free Action)\n");

                char action = _getch();

                if (action == '1') {
                    int baseDmg = 10 + rand() % 11;
                    float modifier = 1.0f;

                    if ((hero.chClass == CLASS_WARRIOR && hero.inventory[hero.activeWeaponIndex].type == WEAPON_MECH) ||
                        (hero.chClass == CLASS_ARCHER && hero.inventory[hero.activeWeaponIndex].type == WEAPON_LUK) ||
                        (hero.chClass == CLASS_MAGE && hero.inventory[hero.activeWeaponIndex].type == WEAPON_POSOH)) {
                        modifier += 0.40f;
                    }

                    ElementType playerElement = hero.inventory[hero.activeWeaponIndex].element;
                    float elementMod = elementMatrix[playerElement][enemyElement];
                    int finalDmg = (int)(baseDmg * modifier * elementMod);

                    printf("\nYou strike with %s!\n", hero.inventory[hero.activeWeaponIndex].name);
                    if (elementMod == 0.0f) {
                        printf(" IMMUNE! This element deals 0 damage! Change weapon!\n");
                        finalDmg = 0;
                    } else if (elementMod > 1.1f) {
                        printf(" ELEMENTAL CRITICAL!\n");
                    } else if (elementMod < 0.9f && playerElement != ELEMENT_NONE) {
                        printf(" Element is ineffective...\n");
                    }

                    enemyHp -= finalDmg;
                    printf("Dealt %d damage!\n", finalDmg);
                    _getch();
                    actionTaken = 1;
                }
                else if (action == '2') {
                    if (hero.potionsCount > 0) {
                        hero.potionsCount--;
                        int heal = 35 + rand() % 21;
                        hero.hp += heal;
                        if (hero.hp > hero.maxHp) hero.hp = hero.maxHp;
                        printf("\nYou healed for %d HP!\n", heal);
                    } else {
                        printf("\nNo potions left!\n");
                    }
                    _getch();
                    actionTaken = 1;
                }
                else if (action == '3') {
                    printf("\n--- QUICK CHANGE (FREE) ---\n");
                    for (int i = 0; i < 3; i++) {
                        printf("%d. %s\n", i + 1, hero.inventory[i].name);
                    }
                    char wpChoice = _getch();
                    if (wpChoice >= '1' && wpChoice <= '3') {
                        int idx = wpChoice - '1';
                        if (strcmp(hero.inventory[idx].name, "Empty Slot") != 0) {
                            hero.activeWeaponIndex = idx;
                            printf("Switched to %s!\n", hero.inventory[idx].name);
                        } else {
                            printf("Slot is empty!\n");
                        }
                    }
                    _getch();
                }
            }
        }
        if (enemyHp <= 0) break;

        printf("\n Enemy attacks!\n");
        int enemyDmg = 25 + rand() % 11 ;
        if (hero.chClass == CLASS_WARRIOR) {
            enemyDmg = (int)(enemyDmg * 0.70);
            printf(" Warrior parried 30%% damage.\n");
        }
        hero.hp -= enemyDmg;
        printf("Enemy dealt %d damage to you!\n", enemyDmg);
        if((rand() % 100) < 40){
            enemyElement = (rand() % 4) + 1;
            printf("New Enemy Element: %s\n", getElementXName(enemyElement));
        }
        _getch();
    }

    if (hero.hp <= 0) {
        return 0;
    } else {
        printf("\n VICTORY! Enemy defeated!\n");
        if (isBoss) {
            printf(" YOU HAVE BEATEN THE FINAL BOSS! THE WORLD IS SAVED!\n");
        }
        _getch();
        return 1;
    }
}

void startDialog(){
    printf("\033[H\033[J");
    printf("mm\n");
    _getch();
}

void checkTileInteraction(int nextX, int nextY) {
    char tile = map[nextY][nextX];

    if (tile == SYM_GOLD) {
        int goldFound = 5 + rand() % 11;
        hero.gold += goldFound;
        map[nextY][nextX] = SYM_EMPTY;
        playerX = nextX;
        playerY = nextY;
    }
    else if (tile == SYM_POTION) {
        hero.potionsCount++;
        map[nextY][nextX] = SYM_EMPTY;
        playerX = nextX;
        playerY = nextY;
    }
    else if (tile == SYM_CHEST) {
        printf("You want to open chest for 60? Y/N");
        char choice = _getch();
        if (choice == 'Y' || choice == 'y') {
            if (hero.gold >= 60) {
                openChest();
                map[nextY][nextX] = SYM_EMPTY;
                playerX = nextX;
                playerY = nextY;
            }
        }
    }
    else if (tile == SYM_MONSTER) {
        int combatResult = startCombat(0);
        if (combatResult == 1) {
            map[nextY][nextX] = SYM_EMPTY;
            playerX = nextX;
            playerY = nextY;
        }
    }
    else if (tile == SYM_BOSS) {
        if(monsterKillCount>9){
        int combatResult = startCombatB(1);
        if (combatResult == 1) {
            map[nextY][nextX] = SYM_EMPTY;
            playerX = nextX;
            playerY = nextY;
            printf("\nCONGRATULATIONS! YOU WON THE GAME!\n");
            _getch();
            exit(0);
        }}else printf("\nYou must kill %d monsters\n",10-monsterKillCount);
    }
    else if (tile == SYM_EMPTY) {
        playerX = nextX;
        playerY = nextY;
    }
    else if (tile == SYM_Princess) {
        startDialog();
    }
}

int main() {
    selectCharacter();
    initMap();
    char move;

    while (1) {
        drawMap();

        move = _getch();

        int nextX = playerX;
        int nextY = playerY;

        if (move == 'w' || move == 'W') nextY--;
        else if (move == 's' || move == 'S') nextY++;
        else if (move == 'a' || move == 'A') nextX--;
        else if (move == 'd' || move == 'D') nextX++;
        else if (move == 'h' || move == 'H') {
            if(hero.potionsCount>0){
            int heal = 20 + rand() % 21;
            hero.hp += heal;
            hero.potionsCount--;
            if (hero.hp > hero.maxHp) hero.hp = hero.maxHp;
                    printf("\nYou healed for %d HP!\n", heal);
            }else{
            printf("You don't have potions");
            }
        }
        else if (move == 'i' || move == 'I') {
            showInventoryAndSell();
            continue;
        }
        else if (move == 'q' || move == 'Q') break;

        if (nextX >= 0 && nextX < WIDTH && nextY >= 0 && nextY < HEIGHT) {
            checkTileInteraction(nextX, nextY);
        }
    }

    printf("\nSee you next time!\n");
    return 0;
}
