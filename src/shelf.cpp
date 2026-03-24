#include "shelf.h"
#include "item.h"

// 545865
void MultiShopShelf::ClearItems() {
    for (int i = 0; i < items.GetSize(); i++) {
        delete items[i];
    }
    items.RemoveAll();
}
