#ifndef QUESTREFS_H
#define QUESTREFS_H

#include <string>

struct ItemDefinitionRef
{
    ItemDefinitionRef() : module(""), slot(0) {}
    ItemDefinitionRef(std::string module_, uint32_t slot_) : module(module_), slot(slot_) {}

    bool operator<(const ItemDefinitionRef &other) const
    {
        if (module < other.module)
            return true;
        else if (module > other.module)
            return false;
        return slot < other.slot;
    }

    bool operator==(const ItemDefinitionRef &other) const
    {
        return !(*this < other) && !(other < *this);
    }

    bool operator!=(const ItemDefinitionRef &other) const
    {
        return (*this < other) || (other < *this);
    }

    std::string module;
    uint32_t slot;
};

struct SpriteDefinitionRef
{
    SpriteDefinitionRef() : module(""), slot(0) {}
    SpriteDefinitionRef(std::string module_, uint32_t slot_) : module(module_), slot(slot_) {}

    bool operator<(const SpriteDefinitionRef &other) const
    {
        if (module < other.module)
            return true;
        else if (module > other.module)
            return false;
        return slot < other.slot;
    }

    bool operator==(const SpriteDefinitionRef &other) const
    {
        return !(*this < other) && !(other < *this);
    }

    bool operator!=(const SpriteDefinitionRef &other) const
    {
        return (*this < other) || (other < *this);
    }

    std::string module;
    uint32_t slot;
};

#endif
