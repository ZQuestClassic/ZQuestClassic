#include "Quest.h"

QuestModule &Quest::getModule(const std::string &name)
{
    if (name.length() == 0)
    {
        assert(!"Invalid module name");
    }
    return questModules_[name]; 
}

itemdata &Quest::getItemDefinition(const ItemDefinitionRef &ref)
{
    if (!isValid(ref))
    {
        assert(!"Invalid item ref");
    }
    return questModules_[ref.module].itemDefTable().getItemDefinition(ref.slot);
}

wpndata &Quest::getSpriteDefinition(const SpriteDefinitionRef &ref)
{
    if (!isValid(ref))
    {
        assert(!"Invalid item ref");
    }
    return questModules_[ref.module].spriteDefTable().getSpriteDefinition(ref.slot);
}

guydata &Quest::getEnemyDefinition(const EnemyDefinitionRef &ref)
{
    if (!isValid(ref))
    {
        assert(!"Invalid item ref");
    }
    return questModules_[ref.module].enemyDefTable().getEnemyDefinition(ref.slot);
}

const std::string &Quest::getEnemyName(const EnemyDefinitionRef &ref)
{
    if (!isValid(ref))
    {
        assert(!"Invalid enemy ref");
    }
    return questModules_[ref.module].enemyDefTable().getEnemyName(ref.slot);
}

void Quest::setEnemyName(const EnemyDefinitionRef &ref, const std::string &newname)
{
    if (!isValid(ref))
    {
        assert(!"Invalid enemy ref");
    }
    questModules_[ref.module].enemyDefTable().setEnemyName(ref.slot, newname);
}

bool Quest::isValid(const ItemDefinitionRef &ref)
{
    if (ref.module.length() == 0)               
        return false;
    return questModules_[ref.module].itemDefTable().isValid(ref.slot);
}

bool Quest::isValid(const SpriteDefinitionRef &ref)
{
    if (ref.module.length() == 0)               
        return false;
    return questModules_[ref.module].spriteDefTable().isValid(ref.slot);
}

bool Quest::isValid(const EnemyDefinitionRef &ref)
{
    if (ref.module.length() == 0)               
        return false;
    return questModules_[ref.module].enemyDefTable().isValid(ref.slot);
}


ItemDefinitionRef Quest::getCanonicalItemID(int family)
{
    std::string lowestmodule;
    int lowestid = -1;
    int lowestlevel = -1;

    for (std::map<std::string, QuestModule>::iterator it = questModules_.begin(); it != questModules_.end(); ++it)
    {
        for (uint32_t i = 0; i < it->second.itemDefTable().getNumItemDefinitions(); i++)
        {
            const itemdata &id = it->second.itemDefTable().getItemDefinition(i);
            if (id.family == family && (id.fam_type < lowestlevel || lowestlevel == -1))
            {
                lowestlevel = id.fam_type;
                lowestid = i;
                lowestmodule = it->first;
            }
        }
    }

    return ItemDefinitionRef(lowestmodule, lowestid);
}

ItemDefinitionRef Quest::getHighestLevelOfFamily(gamedata *source, int family, bool checkenabled)
{
    std::string resmodule;
    int result = -1;
    int highestlevel = -1;

    for (std::map<std::string, QuestModule>::iterator it = questModules_.begin(); it != questModules_.end(); ++it)
    {
        QuestModule &module = it->second;
        for (uint32_t i = 0; i < module.itemDefTable().getNumItemDefinitions(); i++)
        {
            ItemDefinitionRef ref(it->first, i);
            if (module.itemDefTable().getItemDefinition(i).family == family && source->get_item(ref) && (checkenabled ? (!(source->get_disabled_item(ref))) : 1))
            {
                if (module.itemDefTable().getItemDefinition(i).fam_type >= highestlevel)
                {
                    highestlevel = module.itemDefTable().getItemDefinition(i).fam_type;
                    result = i;
                    resmodule = it->first;
                }
            }
        }
    }
    return ItemDefinitionRef(resmodule, result);
}

ItemDefinitionRef Quest::getHighestLevelOfFamily(zinitdata *source, int family)
{
    std::string resmodule;
    int result = -1;
    int highestlevel = -1;

    for (std::map<std::string, QuestModule>::iterator it = questModules_.begin(); it != questModules_.end(); ++it)
    {
        QuestModule &module = it->second;
        for(uint32_t i=0; i<module.itemDefTable().getNumItemDefinitions(); i++)
        {
            ItemDefinitionRef ref(it->first, i);
            if(module.itemDefTable().getItemDefinition(i).family == family && source->inventoryItems.count(ref)>0)
            {
                if(module.itemDefTable().getItemDefinition(i).fam_type >= highestlevel)
                {
                    highestlevel = module.itemDefTable().getItemDefinition(i).fam_type;
                    result=i;
                    resmodule = it->first;
                }
            }
        }
    }

    return ItemDefinitionRef(resmodule, result);
}


ItemDefinitionRef Quest::getItemFromFamilyLevel(int family, int level)
{
    if(level<0) return getCanonicalItemID(family);

    for (std::map<std::string, QuestModule>::iterator it = questModules_.begin(); it != questModules_.end(); ++it)
    {
        QuestModule &module = it->second;

        for (uint32_t i = 0; i < module.itemDefTable().getNumItemDefinitions(); i++)
        {
            if (module.itemDefTable().getItemDefinition(i).family == family && module.itemDefTable().getItemDefinition(i).fam_type == level)
                return ItemDefinitionRef(it->first, i);
        }
    }

    return ItemDefinitionRef();
}

void Quest::getModules(std::vector<std::string> &moduleNames)
{
    moduleNames.clear();

    for (std::map<std::string, QuestModule>::iterator it = questModules_.begin(); it != questModules_.end(); ++it)
        moduleNames.push_back(it->first);
}

void Quest::addOldStyleFamily(zinitdata *dest, int family, char levels)
{
    for(int i=0; i<8; i++)
    {
        if(levels & (1<<i))
        {
            ItemDefinitionRef id = getItemFromFamilyLevel(family, i+1);

            if (isValid(id))
            {
                dest->inventoryItems.insert(id);
            }
        }
    }
}


ItemDefinitionRef Quest::getItemFromFamilyPower(int family, int power)
{
    for (std::map<std::string, QuestModule>::iterator it = questModules_.begin(); it != questModules_.end(); ++it)
    {
        QuestModule &module = it->second;
        for (uint32_t i = 0; i < module.itemDefTable().getNumItemDefinitions(); i++)
        {
            if (module.itemDefTable().getItemDefinition(i).family == family && module.itemDefTable().getItemDefinition(i).power == power)
                return ItemDefinitionRef(it->first, i);
        }
    }

    return ItemDefinitionRef();
}



void Quest::removeItemsOfFamily(zinitdata *z, int family)
{
    for (std::map<std::string, QuestModule>::iterator it = questModules_.begin(); it != questModules_.end(); ++it)
    {
        QuestModule &module = it->second;
        for (uint32_t i = 0; i < module.itemDefTable().getNumItemDefinitions(); i++)
        {
            if (module.itemDefTable().getItemDefinition(i).family == family)
            {
                std::set<ItemDefinitionRef>::iterator it2 = z->inventoryItems.find(ItemDefinitionRef(it->first, i));
                if (it2 != z->inventoryItems.end())
                    z->inventoryItems.erase(*it2);
            }
        }
    }
}

void Quest::removeLowerLevelItemsOfFamily(gamedata *g, int family, int level)
{
    for (std::map<std::string, QuestModule>::iterator it = questModules_.begin(); it != questModules_.end(); ++it)
    {
        QuestModule &module = it->second;
        for (uint32_t i = 0; i < module.itemDefTable().getNumItemDefinitions(); i++)
        {
            if (module.itemDefTable().getItemDefinition(i).family == family && module.itemDefTable().getItemDefinition(i).fam_type < level)
                g->set_item(ItemDefinitionRef(it->first,i), false);
        }
    }
}

int Quest::computeOldStyleBitfield(zinitdata *source, int family)
{
    int rval=0;

    for (std::map<std::string, QuestModule>::iterator it = questModules_.begin(); it != questModules_.end(); ++it)
    {
        QuestModule &module = it->second;
        for (uint32_t i = 0; i < module.itemDefTable().getNumItemDefinitions(); i++) {
            if (module.itemDefTable().getItemDefinition(i).family == family && source->inventoryItems.count(ItemDefinitionRef(it->first, i)) > 0)
            {
                if (module.itemDefTable().getItemDefinition(i).fam_type > 0)
                    rval |= 1 << (module.itemDefTable().getItemDefinition(i).fam_type - 1);
            }
        }
    }

    return rval;
}

void Quest::removeItemsOfFamily(gamedata *g, int family)
{
    for (std::map<std::string, QuestModule>::iterator it = questModules_.begin(); it != questModules_.end(); ++it)
    {
        QuestModule &module = it->second;
        for (uint32_t i = 0; i < module.itemDefTable().getNumItemDefinitions(); i++) {
            {
                if (module.itemDefTable().getItemDefinition(i).family == family)
                    g->set_item(ItemDefinitionRef(it->first, i), false);
            }
        }
    }
}