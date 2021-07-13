#include "stdafx.h"

#include "HArrayGeneric.h"

const uint32_t MIN_COUNT_PAGES = 1;
const uint32_t COUNT_VALUE_ON_ONE_PAGE = 1024;

ValuePool::ValuePool(uint32_t valueLen)
{
    PositionOnPage = 0; //skip first byte
    CurrentPage = 0;
    SizePages = MIN_COUNT_PAGES;

    Pages.reserve(MIN_COUNT_PAGES);
    Pages[CurrentPage].reserve(valueLen * COUNT_VALUE_ON_ONE_PAGE);

    ValueLen = valueLen;
}

void ValuePool::resizePages()
{
    Pages.resize(SizePages * 2);
    SizePages *= 2;
}

void* ValuePool::insValue(uint32_t valueLen, uint32_t& position)
{
    //allocate position
    if (PositionOnPage + valueLen > MAX_SHORT)
    {
        CurrentPage++;

        if (CurrentPage == SizePages)
        {
            resizePages();
        }

        Pages[CurrentPage].resize(MAX_SHORT);
    }

    position = PositionOnPage;

    PositionOnPage += ValueLen;

    return &Pages[position >> 16][position & 0xFFFF];
}

void ValuePool::delValue(uint32_t position)
{
    char* value = &Pages[position >> 16][position & 0xFFFF];

    for (uint32_t i = 0; i < ValueLen; i++)
    {
        value[i] = 0;
    }
}

void* ValuePool::getValue(uint32_t position)
{
    return &Pages[position >> 16][position & 0xFFFF];
}

uint32_t ValuePool::getUsedMemory()
{
    uint32_t usedMemory = sizeof(ValuePool) + ((CurrentPage + 1) * MAX_SHORT);

    return usedMemory;
}

uint32_t ValuePool::getTotalMemory()
{
    uint32_t usedMemory = sizeof(ValuePool) + (SizePages * sizeof(char*)) + ((CurrentPage + 1) * MAX_SHORT);

    return usedMemory;
}

template <typename K, typename V>
uint32_t* HArrayGeneric<K, V>::getKeySegments(const int& obj, uint32_t* keyBuff, uint32_t& keyLen)
{
    keyLen = 1;

    return (uint32_t*)&obj;
}

template <typename K, typename V>
uint32_t* HArrayGeneric<K, V>::getKeySegments(const unsigned int& obj, uint32_t* keyBuff, uint32_t& keyLen)
{
    keyLen = 1;

    return (uint32_t*)&obj;
}

template <typename K, typename V>
uint32_t* HArrayGeneric<K, V>::getKeySegments(const std::string& obj, uint32_t* keyBuff, uint32_t& keyLen)
{
    keyLen = obj.length();
    char* keyBuffBytes = (char*)keyBuff;

    memcpy(keyBuff, obj.c_str(), keyLen);

    while (keyLen & 0x3)
    {
        keyBuffBytes[keyLen++] = 0;
    }

    keyLen >>= 2;

    return keyBuff;
}

template <typename K, typename V>
uint32_t* HArrayGeneric<K, V>::getKeySegments(const char*& obj, uint32_t* keyBuff, uint32_t& keyLen)
{
    keyLen = strlen(obj);
    char* keyBuffBytes = (char*)keyBuff;

    for (uint32_t i = 0; i < keyLen; i++)
    {
        keyBuffBytes[i] = obj[i];
    }

    while (keyLen & 0x3)
    {
        keyBuffBytes[keyLen++] = 0;
    }

    keyLen >>= 2;

    return keyBuff;
}
