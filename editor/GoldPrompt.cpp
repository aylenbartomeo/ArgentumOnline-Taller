#include "GoldPrompt.h"

#include <algorithm>

int goldAmountFromText(const std::string& text) {
    if (text.empty()) {
        return 1;
    }
    try {
        return std::max(1, std::stoi(text));
    } catch (...) {
        return 1;
    }
}
