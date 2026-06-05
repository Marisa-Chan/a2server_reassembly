#pragma once

namespace spell {
    const int fire_arrow = 1;
    const int fire_ball = 2;
    const int wall_of_fire = 3;
    const int protection_from_fire = 4;
    const int ice_missile = 5;
    const int poison_cloud = 6;
    const int blizzard = 7;
    const int protection_from_water = 8;
    const int acid_stream = 9;
    const int lightning = 10;
    const int prismatic_spray = 11;
    const int invisibility = 12;
    const int protection_from_air = 13;
    const int darkness = 14;
    const int light = 15;
    const int stone_missile = 16;
    const int wall_of_earth = 17;
    const int stone_curse = 18;
    const int protection_from_earth = 19;
    const int bless = 20;
    const int haste = 21;
    const int animate_dead = 22;
    const int teleport = 23;
    const int heal = 24;
    const int summon = 25;
    const int drain_life = 26;
    const int shield = 27;
    const int curse = 28;
    const int slow = 29;

    const int max_spell_id = slow;
};

namespace modifier {
    const int price = 1;
    const int body = 2;
    const int mind = 3;
    const int reaction = 4;
    const int spirit = 5;
    const int health = 6;
    const int healthmax = 7;
    const int healthregeneration = 8;
    const int mana = 9;
    const int manamax = 10;             // 0xa
    const int manaregeneration = 11;    // 0xb
    const int tohit = 12;               // 0xc
    const int damagemin = 13;           // 0xd
    const int damagemax = 14;           // 0xe
    const int defence = 15;             // 0xf
    const int absorbtion = 16;          // 0x10
    const int speed = 17;               // 0x11
    const int rotationspeed = 18;       // 0x12
    const int scanrange = 19;           // 0x13
    const int protection0 = 20;         // 0x14
    const int protectionfire = 21;      // 0x15
    const int protectionwater = 22;     // 0x16
    const int protectionair = 23;       // 0x17
    const int protectionearth = 24;     // 0x18
    const int protectionastral = 25;    // 0x19
    const int fighterskill0 = 26;       // 0x1a
    const int skillblade = 27;          // 0x1b
    const int skillaxe = 28;            // 0x1c
    const int skillbludgeon = 29;       // 0x1d
    const int skillpike = 30;           // 0x1e
    const int skillshooting = 31;       // 0x1f
    const int mageskill0 = 32;          // 0x20
    const int skillfire = 33;           // 0x21
    const int skillwater = 34;          // 0x22
    const int skillair = 35;            // 0x23
    const int skillearth = 36;          // 0x24
    const int skillastral = 37;         // 0x25
    const int itemlore = 38;            // 0x26
    const int magiclore = 39;           // 0x27
    const int creaturelore = 40;        // 0x28
    const int castspell = 41;           // 0x29
    const int teachspell = 42;          // 0x2a
    const int damage = 43;              // 0x2b
    const int damagefire = 44;          // 0x2c
    const int damagewater = 45;         // 0x2d
    const int damageair = 46;           // 0x2e
    const int damageearth = 47;         // 0x2f
    const int damageastral = 48;        // 0x30
    const int damagebonus = 49;         // 0x31
};

namespace alm_group_flags {
    const int has_script_id = 1;
    const int random_position = 2;
    const int kill_group = 4;
    const int intercept_group = 8;
};
