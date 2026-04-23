#include "allowed_picture.h"

#include <vector>

const std::vector<uint8_t> allowed_warrior = {32, 33, 34, 35, 36, 37, 38, 39, 41, 42, 43, 44, 45, 47, 48, 49, 50, 51, 52, 5, 2, 3, 4, 1, 6, 7, 8, 9, 10, 22, 23, 24, 25, 31};
const std::vector<uint8_t> allowed_amazon = {79, 81, 82, 67, 66, 65, 68, 72, 73};
const std::vector<uint8_t> allowed_mage = {139, 140, 145, 129, 130, 131, 132, 133, 134, 135, 138};
const std::vector<uint8_t> allowed_witch = {198, 200, 201, 207, 208, 210, 212, 193, 194, 195, 197};

uint8_t FilterAllowedCharacterPicture(uint8_t character_class) {
    uint8_t class_index = (character_class) >> 6;
    const auto& allowed_vector = class_index == 0 ? allowed_warrior : (class_index == 1 ? allowed_amazon : (class_index == 2 ? allowed_mage : allowed_witch));
    for (auto& allowed : allowed_vector) {
        if (allowed == character_class) {
            return character_class;
        }
    }

    // Reset the picture to the first in the allowed list.
    return character_class | (allowed_vector[0] & 0xc0);
}
