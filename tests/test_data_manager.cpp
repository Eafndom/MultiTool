#include "DataManager.h"
#include <iostream>
#include <cassert>

void TestDataManager() {
    DataManager::Get().profiles.clear();
    DataManager::Get().profiles.push_back({"TestProfile", "guid", 1, 1, 1920, 1080, 144, 120.0});
    DataManager::Get().Save();

    DataManager::Get().profiles.clear();
    DataManager::Get().Load();

    assert(DataManager::Get().profiles.size() >= 1);
    auto p = DataManager::Get().GetProfileByName("TestProfile");
    assert(p.has_value());
    assert(p->displayWidth == 1920);
    assert(p->targetFPS == 120.0);
    std::cout << "TestDataManager passed.\n";
}

int main() {
    TestDataManager();
    return 0;
}
