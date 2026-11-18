#include "allowed_picture.h"
#include "../player_file.h"

#include <vector>

const std::vector<uint8_t> allowed_warrior = {32, 33, 34, 35, 36, 37, 38, 39, 41, 42, 43, 44, 45, 47, 48, 49, 50, 51, 52, 5, 2, 3, 4, 1, 6, 7, 8, 9, 10, 22, 23, 24, 25, 31};
const std::vector<uint8_t> allowed_mage = {79, 81, 82, 67, 66, 65, 68, 72, 73};
const std::vector<uint8_t> allowed_amazon = {139, 140, 145, 129, 130, 131, 132, 133, 134, 135, 138};
const std::vector<uint8_t> allowed_witch = {198, 200, 201, 207, 208, 210, 212, 193, 194, 195, 197};

uint8_t FilterAllowedCharacterPicture(uint8_t character_class) {
    CharacterClassCode cls(character_class);
    const auto& allowed_vector = cls.class_index == CharacterClassCode::CLASS_WARRIOR_MALE ? allowed_warrior
        : (cls.class_index == CharacterClassCode::CLASS_MAGE_MALE ? allowed_mage
        : (cls.class_index == CharacterClassCode::CLASS_WARRIOR_FEMALE ? allowed_amazon : allowed_witch));
    for (auto& allowed : allowed_vector) {
        if (allowed == character_class) {
            return character_class;
        }
    }

    // Reset the picture to the first in the allowed list.
    return allowed_vector[0];
}
