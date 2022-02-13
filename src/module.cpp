#include "zdefs.h"
#include "module.h"
#include "zsys.h"

extern zcmodule moduledata;
//Don't define these for ZC here, that's in ffscript.cpp
#ifndef IS_PLAYER
const char CustomComboAttributeTypes[20][4][17] =
{
	{ "cCUSTOM1ATTRIB0", "cCUSTOM1ATTRIB1", "cCUSTOM1ATTRIB2", "cCUSTOM1ATTRIB3" }, 
	{ "cCUSTOM2ATTRIB0", "cCUSTOM2ATTRIB1", "cCUSTOM2ATTRIB2", "cCUSTOM2ATTRIB3" }, 
	{ "cCUSTOM3ATTRIB0", "cCUSTOM3ATTRIB1", "cCUSTOM3ATTRIB2", "cCUSTOM3ATTRIB3" }, 
	{ "cCUSTOM4ATTRIB0", "cCUSTOM4ATTRIB1", "cCUSTOM4ATTRIB2", "cCUSTOM4ATTRIB3" }, 
	{ "cCUSTOM5ATTRIB0", "cCUSTOM5ATTRIB1", "cCUSTOM5ATTRIB2", "cCUSTOM5ATTRIB3" }, 
	{ "cCUSTOM6ATTRIB0", "cCUSTOM6ATTRIB1", "cCUSTOM6ATTRIB2", "cCUSTOM6ATTRIB3" }, 
	{ "cCUSTOM7ATTRIB0", "cCUSTOM7ATTRIB1", "cCUSTOM7ATTRIB2", "cCUSTOM7ATTRIB3" }, 
	{ "cCUSTOM8ATTRIB0", "cCUSTOM8ATTRIB1", "cCUSTOM8ATTRIB2", "cCUSTOM8ATTRIB3" }, 
	{ "cCUSTOM9ATTRIB0", "cCUSTOM9ATTRIB1", "cCUSTOM9ATTRIB2", "cCUSTOM9ATTRIB3" }, 
	{ "cCUSTOM10ATTRIB0", "cCUSTOM10ATTRIB1", "cCUSTOM10ATTRIB2", "cCUSTOM10ATTRIB3" },
	
	{ "cCUSTOM11ATTRIB0", "cCUSTOM11ATTRIB1", "cCUSTOM11ATTRIB2", "cCUSTOM11ATTRIB3" }, 
	{ "cCUSTOM12ATTRIB0", "cCUSTOM12ATTRIB1", "cCUSTOM12ATTRIB2", "cCUSTOM12ATTRIB3" }, 
	{ "cCUSTOM13ATTRIB0", "cCUSTOM13ATTRIB1", "cCUSTOM13ATTRIB2", "cCUSTOM13ATTRIB3" }, 
	{ "cCUSTOM14ATTRIB0", "cCUSTOM14ATTRIB1", "cCUSTOM14ATTRIB2", "cCUSTOM14ATTRIB3" }, 
	{ "cCUSTOM15ATTRIB0", "cCUSTOM15ATTRIB1", "cCUSTOM15ATTRIB2", "cCUSTOM15ATTRIB3" }, 
	{ "cCUSTOM16ATTRIB0", "cCUSTOM16ATTRIB1", "cCUSTOM16ATTRIB2", "cCUSTOM16ATTRIB3" }, 
	{ "cCUSTOM17ATTRIB0", "cCUSTOM17ATTRIB1", "cCUSTOM17ATTRIB2", "cCUSTOM17ATTRIB3" }, 
	{ "cCUSTOM18ATTRIB0", "cCUSTOM18ATTRIB1", "cCUSTOM18ATTRIB2", "cCUSTOM18ATTRIB3" }, 
	{ "cCUSTOM19ATTRIB0", "cCUSTOM19ATTRIB1", "cCUSTOM19ATTRIB2", "cCUSTOM19ATTRIB3" }, 
	{ "cCUSTOM20ATTRIB0", "cCUSTOM20ATTRIB1", "cCUSTOM20ATTRIB2", "cCUSTOM20ATTRIB3" }
};

const char CustomComboATTRuteTypes[20][4][17] =
{
	{ "cCUSTOM1ATTR0", "cCUSTOM1ATTR1", "cCUSTOM1ATTR2", "cCUSTOM1ATTR3" }, 
	{ "cCUSTOM2ATTR0", "cCUSTOM2ATTR1", "cCUSTOM2ATTR2", "cCUSTOM2ATTR3" }, 
	{ "cCUSTOM3ATTR0", "cCUSTOM3ATTR1", "cCUSTOM3ATTR2", "cCUSTOM3ATTR3" }, 
	{ "cCUSTOM4ATTR0", "cCUSTOM4ATTR1", "cCUSTOM4ATTR2", "cCUSTOM4ATTR3" }, 
	{ "cCUSTOM5ATTR0", "cCUSTOM5ATTR1", "cCUSTOM5ATTR2", "cCUSTOM5ATTR3" }, 
	{ "cCUSTOM6ATTR0", "cCUSTOM6ATTR1", "cCUSTOM6ATTR2", "cCUSTOM6ATTR3" }, 
	{ "cCUSTOM7ATTR0", "cCUSTOM7ATTR1", "cCUSTOM7ATTR2", "cCUSTOM7ATTR3" }, 
	{ "cCUSTOM8ATTR0", "cCUSTOM8ATTR1", "cCUSTOM8ATTR2", "cCUSTOM8ATTR3" }, 
	{ "cCUSTOM9ATTR0", "cCUSTOM9ATTR1", "cCUSTOM9ATTR2", "cCUSTOM9ATTR3" }, 
	{ "cCUSTOM10ATTR0", "cCUSTOM10ATTR1", "cCUSTOM10ATTR2", "cCUSTOM10ATTR3" },
	
	{ "cCUSTOM11ATTR0", "cCUSTOM11ATTR1", "cCUSTOM11ATTR2", "cCUSTOM11ATTR3" }, 
	{ "cCUSTOM12ATTR0", "cCUSTOM12ATTR1", "cCUSTOM12ATTR2", "cCUSTOM12ATTR3" }, 
	{ "cCUSTOM13ATTR0", "cCUSTOM13ATTR1", "cCUSTOM13ATTR2", "cCUSTOM13ATTR3" }, 
	{ "cCUSTOM14ATTR0", "cCUSTOM14ATTR1", "cCUSTOM14ATTR2", "cCUSTOM14ATTR3" }, 
	{ "cCUSTOM15ATTR0", "cCUSTOM15ATTR1", "cCUSTOM15ATTR2", "cCUSTOM15ATTR3" }, 
	{ "cCUSTOM16ATTR0", "cCUSTOM16ATTR1", "cCUSTOM16ATTR2", "cCUSTOM16ATTR3" }, 
	{ "cCUSTOM17ATTR0", "cCUSTOM17ATTR1", "cCUSTOM17ATTR2", "cCUSTOM17ATTR3" }, 
	{ "cCUSTOM18ATTR0", "cCUSTOM18ATTR1", "cCUSTOM18ATTR2", "cCUSTOM18ATTR3" }, 
	{ "cCUSTOM19ATTR0", "cCUSTOM19ATTR1", "cCUSTOM19ATTR2", "cCUSTOM19ATTR3" }, 
	{ "cCUSTOM20ATTR0", "cCUSTOM20ATTR1", "cCUSTOM20ATTR2", "cCUSTOM20ATTR3" }
};

const char CustomComboAttribyteTypes[20][8][17] =
{
	{ "cCUSTOM1ATTRIB0", "cCUSTOM1ATTRIB1", "cCUSTOM1ATTRIB2", "cCUSTOM1ATTRIB3", "cCUSTOM1ATTRIB4", "cCUSTOM1ATTRIB5", "cCUSTOM1ATTRIB6", "cCUSTOM1ATTRIB7" }, 
	{ "cCUSTOM2ATTRIB0", "cCUSTOM2ATTRIB1", "cCUSTOM2ATTRIB2", "cCUSTOM2ATTRIB3", "cCUSTOM2ATTRIB4", "cCUSTOM2ATTRIB5", "cCUSTOM2ATTRIB6", "cCUSTOM2ATTRIB7" }, 
	{ "cCUSTOM3ATTRIB0", "cCUSTOM3ATTRIB1", "cCUSTOM3ATTRIB2", "cCUSTOM3ATTRIB3", "cCUSTOM3ATTRIB4", "cCUSTOM3ATTRIB5", "cCUSTOM3ATTRIB6", "cCUSTOM3ATTRIB7" }, 
	{ "cCUSTOM4ATTRIB0", "cCUSTOM4ATTRIB1", "cCUSTOM4ATTRIB2", "cCUSTOM4ATTRIB3", "cCUSTOM4ATTRIB4", "cCUSTOM4ATTRIB5", "cCUSTOM4ATTRIB6", "cCUSTOM4ATTRIB7" }, 
	{ "cCUSTOM5ATTRIB0", "cCUSTOM5ATTRIB1", "cCUSTOM5ATTRIB2", "cCUSTOM5ATTRIB3", "cCUSTOM5ATTRIB4", "cCUSTOM5ATTRIB5", "cCUSTOM5ATTRIB6", "cCUSTOM5ATTRIB7" }, 
	{ "cCUSTOM6ATTRIB0", "cCUSTOM6ATTRIB1", "cCUSTOM6ATTRIB2", "cCUSTOM6ATTRIB3", "cCUSTOM6ATTRIB4", "cCUSTOM6ATTRIB5", "cCUSTOM6ATTRIB6", "cCUSTOM6ATTRIB7" }, 
	{ "cCUSTOM7ATTRIB0", "cCUSTOM7ATTRIB1", "cCUSTOM7ATTRIB2", "cCUSTOM7ATTRIB3", "cCUSTOM7ATTRIB4", "cCUSTOM7ATTRIB5", "cCUSTOM7ATTRIB6", "cCUSTOM7ATTRIB7" }, 
	{ "cCUSTOM8ATTRIB0", "cCUSTOM8ATTRIB1", "cCUSTOM8ATTRIB2", "cCUSTOM8ATTRIB3", "cCUSTOM8ATTRIB4", "cCUSTOM8ATTRIB5", "cCUSTOM8ATTRIB6", "cCUSTOM8ATTRIB7" }, 
	{ "cCUSTOM9ATTRIB0", "cCUSTOM9ATTRIB1", "cCUSTOM9ATTRIB2", "cCUSTOM9ATTRIB3", "cCUSTOM9ATTRIB4", "cCUSTOM9ATTRIB5", "cCUSTOM9ATTRIB6", "cCUSTOM9ATTRIB7" }, 
	{ "cCUSTOM10ATTRIB0", "cCUSTOM10ATTRIB1", "cCUSTOM10ATTRIB2", "cCUSTOM10ATTRIB3", "cCUSTOM10ATTRIB4", "cCUSTOM10ATTRIB5", "cCUSTOM10ATTRIB6", "cCUSTOM10ATTRIB7" },
	
	{ "cCUSTOM11ATTRIB0", "cCUSTOM11ATTRIB1", "cCUSTOM11ATTRIB2", "cCUSTOM11ATTRIB3", "cCUSTOM11ATTRIB4", "cCUSTOM11ATTRIB5", "cCUSTOM11ATTRIB6", "cCUSTOM11ATTRIB7" }, 
	{ "cCUSTOM12ATTRIB0", "cCUSTOM12ATTRIB1", "cCUSTOM12ATTRIB2", "cCUSTOM12ATTRIB3", "cCUSTOM12ATTRIB4", "cCUSTOM12ATTRIB5", "cCUSTOM12ATTRIB6", "cCUSTOM12ATTRIB7" }, 
	{ "cCUSTOM13ATTRIB0", "cCUSTOM13ATTRIB1", "cCUSTOM13ATTRIB2", "cCUSTOM13ATTRIB3", "cCUSTOM13ATTRIB4", "cCUSTOM13ATTRIB5", "cCUSTOM13ATTRIB6", "cCUSTOM13ATTRIB7" }, 
	{ "cCUSTOM14ATTRIB0", "cCUSTOM14ATTRIB1", "cCUSTOM14ATTRIB2", "cCUSTOM14ATTRIB3", "cCUSTOM14ATTRIB4", "cCUSTOM14ATTRIB5", "cCUSTOM14ATTRIB6", "cCUSTOM14ATTRIB7" }, 
	{ "cCUSTOM15ATTRIB0", "cCUSTOM15ATTRIB1", "cCUSTOM15ATTRIB2", "cCUSTOM15ATTRIB3", "cCUSTOM15ATTRIB4", "cCUSTOM15ATTRIB5", "cCUSTOM15ATTRIB6", "cCUSTOM15ATTRIB7" }, 
	{ "cCUSTOM16ATTRIB0", "cCUSTOM16ATTRIB1", "cCUSTOM16ATTRIB2", "cCUSTOM16ATTRIB3", "cCUSTOM16ATTRIB4", "cCUSTOM16ATTRIB5", "cCUSTOM16ATTRIB6", "cCUSTOM16ATTRIB7" }, 
	{ "cCUSTOM17ATTRIB0", "cCUSTOM17ATTRIB1", "cCUSTOM17ATTRIB2", "cCUSTOM17ATTRIB3", "cCUSTOM17ATTRIB4", "cCUSTOM17ATTRIB5", "cCUSTOM17ATTRIB6", "cCUSTOM17ATTRIB7" }, 
	{ "cCUSTOM18ATTRIB0", "cCUSTOM18ATTRIB1", "cCUSTOM18ATTRIB2", "cCUSTOM18ATTRIB3", "cCUSTOM18ATTRIB4", "cCUSTOM18ATTRIB5", "cCUSTOM18ATTRIB6", "cCUSTOM18ATTRIB7" }, 
	{ "cCUSTOM19ATTRIB0", "cCUSTOM19ATTRIB1", "cCUSTOM19ATTRIB2", "cCUSTOM19ATTRIB3", "cCUSTOM19ATTRIB4", "cCUSTOM19ATTRIB5", "cCUSTOM19ATTRIB6", "cCUSTOM19ATTRIB7" }, 
	{ "cCUSTOM20ATTRIB0", "cCUSTOM20ATTRIB1", "cCUSTOM20ATTRIB2", "cCUSTOM20ATTRIB3", "cCUSTOM20ATTRIB4", "cCUSTOM20ATTRIB5", "cCUSTOM20ATTRIB6", "cCUSTOM20ATTRIB7" }
};

const char CustomComboAttrishortTypes[20][8][17] =
{
	{ "cCUSTOM1ATTRSH0", "cCUSTOM1ATTRSH1", "cCUSTOM1ATTRSH2", "cCUSTOM1ATTRSH3", "cCUSTOM1ATTRSH4", "cCUSTOM1ATTRSH5", "cCUSTOM1ATTRSH6", "cCUSTOM1ATTRSH7" }, 
	{ "cCUSTOM2ATTRSH0", "cCUSTOM2ATTRSH1", "cCUSTOM2ATTRSH2", "cCUSTOM2ATTRSH3", "cCUSTOM2ATTRSH4", "cCUSTOM2ATTRSH5", "cCUSTOM2ATTRSH6", "cCUSTOM2ATTRSH7" }, 
	{ "cCUSTOM3ATTRSH0", "cCUSTOM3ATTRSH1", "cCUSTOM3ATTRSH2", "cCUSTOM3ATTRSH3", "cCUSTOM3ATTRSH4", "cCUSTOM3ATTRSH5", "cCUSTOM3ATTRSH6", "cCUSTOM3ATTRSH7" }, 
	{ "cCUSTOM4ATTRSH0", "cCUSTOM4ATTRSH1", "cCUSTOM4ATTRSH2", "cCUSTOM4ATTRSH3", "cCUSTOM4ATTRSH4", "cCUSTOM4ATTRSH5", "cCUSTOM4ATTRSH6", "cCUSTOM4ATTRSH7" }, 
	{ "cCUSTOM5ATTRSH0", "cCUSTOM5ATTRSH1", "cCUSTOM5ATTRSH2", "cCUSTOM5ATTRSH3", "cCUSTOM5ATTRSH4", "cCUSTOM5ATTRSH5", "cCUSTOM5ATTRSH6", "cCUSTOM5ATTRSH7" }, 
	{ "cCUSTOM6ATTRSH0", "cCUSTOM6ATTRSH1", "cCUSTOM6ATTRSH2", "cCUSTOM6ATTRSH3", "cCUSTOM6ATTRSH4", "cCUSTOM6ATTRSH5", "cCUSTOM6ATTRSH6", "cCUSTOM6ATTRSH7" }, 
	{ "cCUSTOM7ATTRSH0", "cCUSTOM7ATTRSH1", "cCUSTOM7ATTRSH2", "cCUSTOM7ATTRSH3", "cCUSTOM7ATTRSH4", "cCUSTOM7ATTRSH5", "cCUSTOM7ATTRSH6", "cCUSTOM7ATTRSH7" }, 
	{ "cCUSTOM8ATTRSH0", "cCUSTOM8ATTRSH1", "cCUSTOM8ATTRSH2", "cCUSTOM8ATTRSH3", "cCUSTOM8ATTRSH4", "cCUSTOM8ATTRSH5", "cCUSTOM8ATTRSH6", "cCUSTOM8ATTRSH7" }, 
	{ "cCUSTOM9ATTRSH0", "cCUSTOM9ATTRSH1", "cCUSTOM9ATTRSH2", "cCUSTOM9ATTRSH3", "cCUSTOM9ATTRSH4", "cCUSTOM9ATTRSH5", "cCUSTOM9ATTRSH6", "cCUSTOM9ATTRSH7" }, 
	{ "cCUSTOM10ATTRSH0", "cCUSTOM10ATTRSH1", "cCUSTOM10ATTRSH2", "cCUSTOM10ATTRSH3", "cCUSTOM10ATTRSH4", "cCUSTOM10ATTRSH5", "cCUSTOM10ATTRSH6", "cCUSTOM10ATTRSH7" },
	
	{ "cCUSTOM11ATTRSH0", "cCUSTOM11ATTRSH1", "cCUSTOM11ATTRSH2", "cCUSTOM11ATTRSH3", "cCUSTOM11ATTRSH4", "cCUSTOM11ATTRSH5", "cCUSTOM11ATTRSH6", "cCUSTOM11ATTRSH7" }, 
	{ "cCUSTOM12ATTRSH0", "cCUSTOM12ATTRSH1", "cCUSTOM12ATTRSH2", "cCUSTOM12ATTRSH3", "cCUSTOM12ATTRSH4", "cCUSTOM12ATTRSH5", "cCUSTOM12ATTRSH6", "cCUSTOM12ATTRSH7" }, 
	{ "cCUSTOM13ATTRSH0", "cCUSTOM13ATTRSH1", "cCUSTOM13ATTRSH2", "cCUSTOM13ATTRSH3", "cCUSTOM13ATTRSH4", "cCUSTOM13ATTRSH5", "cCUSTOM13ATTRSH6", "cCUSTOM13ATTRSH7" }, 
	{ "cCUSTOM14ATTRSH0", "cCUSTOM14ATTRSH1", "cCUSTOM14ATTRSH2", "cCUSTOM14ATTRSH3", "cCUSTOM14ATTRSH4", "cCUSTOM14ATTRSH5", "cCUSTOM14ATTRSH6", "cCUSTOM14ATTRSH7" }, 
	{ "cCUSTOM15ATTRSH0", "cCUSTOM15ATTRSH1", "cCUSTOM15ATTRSH2", "cCUSTOM15ATTRSH3", "cCUSTOM15ATTRSH4", "cCUSTOM15ATTRSH5", "cCUSTOM15ATTRSH6", "cCUSTOM15ATTRSH7" }, 
	{ "cCUSTOM16ATTRSH0", "cCUSTOM16ATTRSH1", "cCUSTOM16ATTRSH2", "cCUSTOM16ATTRSH3", "cCUSTOM16ATTRSH4", "cCUSTOM16ATTRSH5", "cCUSTOM16ATTRSH6", "cCUSTOM16ATTRSH7" }, 
	{ "cCUSTOM17ATTRSH0", "cCUSTOM17ATTRSH1", "cCUSTOM17ATTRSH2", "cCUSTOM17ATTRSH3", "cCUSTOM17ATTRSH4", "cCUSTOM17ATTRSH5", "cCUSTOM17ATTRSH6", "cCUSTOM17ATTRSH7" }, 
	{ "cCUSTOM18ATTRSH0", "cCUSTOM18ATTRSH1", "cCUSTOM18ATTRSH2", "cCUSTOM18ATTRSH3", "cCUSTOM18ATTRSH4", "cCUSTOM18ATTRSH5", "cCUSTOM18ATTRSH6", "cCUSTOM18ATTRSH7" }, 
	{ "cCUSTOM19ATTRSH0", "cCUSTOM19ATTRSH1", "cCUSTOM19ATTRSH2", "cCUSTOM19ATTRSH3", "cCUSTOM19ATTRSH4", "cCUSTOM19ATTRSH5", "cCUSTOM19ATTRSH6", "cCUSTOM19ATTRSH7" }, 
	{ "cCUSTOM20ATTRSH0", "cCUSTOM20ATTRSH1", "cCUSTOM20ATTRSH2", "cCUSTOM20ATTRSH3", "cCUSTOM20ATTRSH4", "cCUSTOM20ATTRSH5", "cCUSTOM20ATTRSH6", "cCUSTOM20ATTRSH7" }
};

const char CustomComboAttributeFlags[20][16][16] =
{
	{ 	
		"cCust1F0", "cCust1F1", "cCust1F2", "cCust1F3", "cCust1F4", "cCust1F5",  "cCust1F6", "cCust1F7", 
		"cCust1F8", "cCust1F9", "cCust1F10", "cCust1F11", "cCust1F12", "cCust1F13", "cCust1F14", "cCust1F15", 
	},
	{ 	
		"cCust2F0", "cCust2F1", "cCust2F2", "cCust2F3", "cCust2F4", "cCust2F5",  "cCust2F6", "cCust2F7", 
		"cCust2F8", "cCust2F9", "cCust2F10", "cCust2F11", "cCust2F12", "cCust2F13", "cCust2F14", "cCust2F15", 
	},
	{ 	
		"cCust3F0", "cCust3F1", "cCust3F2", "cCust3F3", "cCust3F4", "cCust3F5",  "cCust3F6", "cCust3F7", 
		"cCust3F8", "cCust3F9", "cCust3F10", "cCust3F11", "cCust3F12", "cCust3F13", "cCust3F14", "cCust3F15", 
	},
	{ 	
		"cCust4F0", "cCust4F1", "cCust4F2", "cCust4F3", "cCust4F4", "cCust4F5",  "cCust4F6", "cCust4F7", 
		"cCust4F8", "cCust4F9", "cCust4F10", "cCust4F11", "cCust4F12", "cCust4F13", "cCust4F14", "cCust4F15", 
	},
	{ 	
		"cCust5F0", "cCust5F1", "cCust5F2", "cCust5F3", "cCust5F4", "cCust5F5",  "cCust5F6", "cCust5F7", 
		"cCust5F8", "cCust5F9", "cCust5F10", "cCust5F11", "cCust5F12", "cCust5F13", "cCust5F14", "cCust5F15", 
	},
	{ 	
		"cCust6F0", "cCust6F1", "cCust6F2", "cCust6F3", "cCust6F4", "cCust6F5",  "cCust6F6", "cCust6F7", 
		"cCust6F8", "cCust6F9", "cCust6F10", "cCust6F11", "cCust6F12", "cCust6F13", "cCust6F14", "cCust6F15", 
	},
	{ 	
		"cCust7F0", "cCust7F1", "cCust7F2", "cCust7F3", "cCust7F4", "cCust7F5",  "cCust7F6", "cCust7F7", 
		"cCust7F8", "cCust7F9", "cCust7F10", "cCust7F11", "cCust7F12", "cCust7F13", "cCust7F14", "cCust7F15", 
	},
	{ 	
		"cCust8F0", "cCust8F1", "cCust8F2", "cCust8F3", "cCust8F4", "cCust8F5",  "cCust8F6", "cCust8F7", 
		"cCust8F8", "cCust8F9", "cCust8F10", "cCust8F11", "cCust8F12", "cCust8F13", "cCust8F14", "cCust8F15", 
	},
	{ 	
		"cCust9F0", "cCust9F1", "cCust9F2", "cCust9F3", "cCust9F4", "cCust9F5",  "cCust9F6", "cCust9F7", 
		"cCust9F8", "cCust9F9", "cCust9F10", "cCust9F11", "cCust9F12", "cCust9F13", "cCust9F14", "cCust9F15", 
	},
	{ 	
		"cCust10F0", "cCust10F1", "cCust10F2", "cCust10F3", "cCust10F4", "cCust10F5",  "cCust10F6", "cCust10F7", 
		"cCust10F8", "cCust10F9", "cCust10F10", "cCust10F11", "cCust10F12", "cCust10F13", "cCust10F14", "cCust10F15", 
	},
	{ 	
		"cCust11F0", "cCust11F1", "cCust11F2", "cCust11F3", "cCust11F4", "cCust11F5",  "cCust11F6", "cCust11F7", 
		"cCust11F8", "cCust11F9", "cCust11F10", "cCust11F11", "cCust11F12", "cCust11F13", "cCust11F14", "cCust11F15", 
	},
	{ 	
		"cCust12F0", "cCust12F1", "cCust12F2", "cCust12F3", "cCust12F4", "cCust12F5",  "cCust12F6", "cCust12F7", 
		"cCust12F8", "cCust12F9", "cCust12F10", "cCust12F11", "cCust12F12", "cCust12F13", "cCust12F14", "cCust12F15", 
	},
	{ 	
		"cCust13F0", "cCust13F1", "cCust13F2", "cCust13F3", "cCust13F4", "cCust13F5",  "cCust13F6", "cCust13F7", 
		"cCust13F8", "cCust13F9", "cCust13F10", "cCust13F11", "cCust13F12", "cCust13F13", "cCust13F14", "cCust13F15", 
	},
	{ 	
		"cCust14F0", "cCust14F1", "cCust14F2", "cCust14F3", "cCust14F4", "cCust14F5",  "cCust14F6", "cCust14F7", 
		"cCust14F8", "cCust14F9", "cCust14F10", "cCust14F11", "cCust14F12", "cCust14F13", "cCust14F14", "cCust14F15", 
	},
	{ 	
		"cCust15F0", "cCust15F1", "cCust15F2", "cCust15F3", "cCust15F4", "cCust15F5",  "cCust15F6", "cCust15F7", 
		"cCust15F8", "cCust15F9", "cCust15F10", "cCust15F11", "cCust15F12", "cCust15F13", "cCust15F14", "cCust15F15", 
	},
};

const char defaultCustomComboFlags[20][16][32]=
{
	{ 
		"Flag 1", "Flag 2", "Flag 3", "Flag 4", "Flag 5", "Flag 6", "Flag 7", "Flag 8", 
		"Engine", "Flag 10", "Flag 11", "Flag 12", "Flag 13", "Flag 14", "Flag 15", "Flag 16"
	},
	{ 
		"Flag 1", "Flag 2", "Flag 3", "Flag 4", "Flag 5", "Flag 6", "Flag 7", "Flag 8", 
		"Engine", "Flag 10", "Flag 11", "Flag 12", "Flag 13", "Flag 14", "Flag 15", "Flag 16"
	},
	{ 
		"Flag 1", "Flag 2", "Flag 3", "Flag 4", "Flag 5", "Flag 6", "Flag 7", "Flag 8", 
		"Engine", "Flag 10", "Flag 11", "Flag 12", "Flag 13", "Flag 14", "Flag 15", "Flag 16"
	},
	{ 
		"Flag 1", "Flag 2", "Flag 3", "Flag 4", "Flag 5", "Flag 6", "Flag 7", "Flag 8", 
		"Engine", "Flag 10", "Flag 11", "Flag 12", "Flag 13", "Flag 14", "Flag 15", "Flag 16"
	},
	{ 
		"Flag 1", "Flag 2", "Flag 3", "Flag 4", "Flag 5", "Flag 6", "Flag 7", "Flag 8", 
		"Engine", "Flag 10", "Flag 11", "Flag 12", "Flag 13", "Flag 14", "Flag 15", "Flag 16"
	},
	{ 
		"Flag 1", "Flag 2", "Flag 3", "Flag 4", "Flag 5", "Flag 6", "Flag 7", "Flag 8", 
		"Engine", "Flag 10", "Flag 11", "Flag 12", "Flag 13", "Flag 14", "Flag 15", "Flag 16"
	},
	{ 
		"Flag 1", "Flag 2", "Flag 3", "Flag 4", "Flag 5", "Flag 6", "Flag 7", "Flag 8", 
		"Engine", "Flag 10", "Flag 11", "Flag 12", "Flag 13", "Flag 14", "Flag 15", "Flag 16"
	},
	{ 
		"Flag 1", "Flag 2", "Flag 3", "Flag 4", "Flag 5", "Flag 6", "Flag 7", "Flag 8", 
		"Engine", "Flag 10", "Flag 11", "Flag 12", "Flag 13", "Flag 14", "Flag 15", "Flag 16"
	},
	{ 
		"Flag 1", "Flag 2", "Flag 3", "Flag 4", "Flag 5", "Flag 6", "Flag 7", "Flag 8", 
		"Engine", "Flag 10", "Flag 11", "Flag 12", "Flag 13", "Flag 14", "Flag 15", "Flag 16"
	},
	{ 
		"Flag 1", "Flag 2", "Flag 3", "Flag 4", "Flag 5", "Flag 6", "Flag 7", "Flag 8", 
		"Engine", "Flag 10", "Flag 11", "Flag 12", "Flag 13", "Flag 14", "Flag 15", "Flag 16"
	},
	{ 
		"Flag 1", "Flag 2", "Flag 3", "Flag 4", "Flag 5", "Flag 6", "Flag 7", "Flag 8", 
		"Engine", "Flag 10", "Flag 11", "Flag 12", "Flag 13", "Flag 14", "Flag 15", "Flag 16"
	},
	{ 
		"Flag 1", "Flag 2", "Flag 3", "Flag 4", "Flag 5", "Flag 6", "Flag 7", "Flag 8", 
		"Engine", "Flag 10", "Flag 11", "Flag 12", "Flag 13", "Flag 14", "Flag 15", "Flag 16"
	},
	{ 
		"Flag 1", "Flag 2", "Flag 3", "Flag 4", "Flag 5", "Flag 6", "Flag 7", "Flag 8", 
		"Engine", "Flag 10", "Flag 11", "Flag 12", "Flag 13", "Flag 14", "Flag 15", "Flag 16"
	},
	{ 
		"Flag 1", "Flag 2", "Flag 3", "Flag 4", "Flag 5", "Flag 6", "Flag 7", "Flag 8", 
		"Engine", "Flag 10", "Flag 11", "Flag 12", "Flag 13", "Flag 14", "Flag 15", "Flag 16"
	},
	{ 
		"Flag 1", "Flag 2", "Flag 3", "Flag 4", "Flag 5", "Flag 6", "Flag 7", "Flag 8", 
		"Engine", "Flag 10", "Flag 11", "Flag 12", "Flag 13", "Flag 14", "Flag 15", "Flag 16"
	},
	{ 
		"Flag 1", "Flag 2", "Flag 3", "Flag 4", "Flag 5", "Flag 6", "Flag 7", "Flag 8", 
		"Engine", "Flag 10", "Flag 11", "Flag 12", "Flag 13", "Flag 14", "Flag 15", "Flag 16"
	},
	{ 
		"Flag 1", "Flag 2", "Flag 3", "Flag 4", "Flag 5", "Flag 6", "Flag 7", "Flag 8", 
		"Engine", "Flag 10", "Flag 11", "Flag 12", "Flag 13", "Flag 14", "Flag 15", "Flag 16"
	},
	{ 
		"Flag 1", "Flag 2", "Flag 3", "Flag 4", "Flag 5", "Flag 6", "Flag 7", "Flag 8", 
		"Engine", "Flag 10", "Flag 11", "Flag 12", "Flag 13", "Flag 14", "Flag 15", "Flag 16"
	},
	{ 
		"Flag 1", "Flag 2", "Flag 3", "Flag 4", "Flag 5", "Flag 6", "Flag 7", "Flag 8", 
		"Engine", "Flag 10", "Flag 11", "Flag 12", "Flag 13", "Flag 14", "Flag 15", "Flag 16"
	},
	{ 
		"Flag 1", "Flag 2", "Flag 3", "Flag 4", "Flag 5", "Flag 6", "Flag 7", "Flag 8", 
		"Engine", "Flag 10", "Flag 11", "Flag 12", "Flag 13", "Flag 14", "Flag 15", "Flag 16"
	}

};

const char defaultCustomComboAttribytes[20][8][17]=
{
	{ "Attribytes[0]", "Attribytes[1]", "Attribytes[2]", "Attribytes[3]", "Attribytes[4]", "Attribytes[5]", "Attribytes[6]", "Attribytes[7]" },
	{ "Attribytes[0]", "Attribytes[1]", "Attribytes[2]", "Attribytes[3]", "Attribytes[4]", "Attribytes[5]", "Attribytes[6]", "Attribytes[7]" },
	{ "Attribytes[0]", "Attribytes[1]", "Attribytes[2]", "Attribytes[3]", "Attribytes[4]", "Attribytes[5]", "Attribytes[6]", "Attribytes[7]" },
	{ "Attribytes[0]", "Attribytes[1]", "Attribytes[2]", "Attribytes[3]", "Attribytes[4]", "Attribytes[5]", "Attribytes[6]", "Attribytes[7]" },
	{ "Attribytes[0]", "Attribytes[1]", "Attribytes[2]", "Attribytes[3]", "Attribytes[4]", "Attribytes[5]", "Attribytes[6]", "Attribytes[7]" },
	{ "Attribytes[0]", "Attribytes[1]", "Attribytes[2]", "Attribytes[3]", "Attribytes[4]", "Attribytes[5]", "Attribytes[6]", "Attribytes[7]" },
	{ "Attribytes[0]", "Attribytes[1]", "Attribytes[2]", "Attribytes[3]", "Attribytes[4]", "Attribytes[5]", "Attribytes[6]", "Attribytes[7]" },
	{ "Attribytes[0]", "Attribytes[1]", "Attribytes[2]", "Attribytes[3]", "Attribytes[4]", "Attribytes[5]", "Attribytes[6]", "Attribytes[7]" },
	{ "Attribytes[0]", "Attribytes[1]", "Attribytes[2]", "Attribytes[3]", "Attribytes[4]", "Attribytes[5]", "Attribytes[6]", "Attribytes[7]" },
	{ "Attribytes[0]", "Attribytes[1]", "Attribytes[2]", "Attribytes[3]", "Attribytes[4]", "Attribytes[5]", "Attribytes[6]", "Attribytes[7]" },
	{ "Attribytes[0]", "Attribytes[1]", "Attribytes[2]", "Attribytes[3]", "Attribytes[4]", "Attribytes[5]", "Attribytes[6]", "Attribytes[7]" },
	{ "Attribytes[0]", "Attribytes[1]", "Attribytes[2]", "Attribytes[3]", "Attribytes[4]", "Attribytes[5]", "Attribytes[6]", "Attribytes[7]" },
	{ "Attribytes[0]", "Attribytes[1]", "Attribytes[2]", "Attribytes[3]", "Attribytes[4]", "Attribytes[5]", "Attribytes[6]", "Attribytes[7]" },
	{ "Attribytes[0]", "Attribytes[1]", "Attribytes[2]", "Attribytes[3]", "Attribytes[4]", "Attribytes[5]", "Attribytes[6]", "Attribytes[7]" },
	{ "Attribytes[0]", "Attribytes[1]", "Attribytes[2]", "Attribytes[3]", "Attribytes[4]", "Attribytes[5]", "Attribytes[6]", "Attribytes[7]" },
	{ "Attribytes[0]", "Attribytes[1]", "Attribytes[2]", "Attribytes[3]", "Attribytes[4]", "Attribytes[5]", "Attribytes[6]", "Attribytes[7]" },
	{ "Attribytes[0]", "Attribytes[1]", "Attribytes[2]", "Attribytes[3]", "Attribytes[4]", "Attribytes[5]", "Attribytes[6]", "Attribytes[7]" },
	{ "Attribytes[0]", "Attribytes[1]", "Attribytes[2]", "Attribytes[3]", "Attribytes[4]", "Attribytes[5]", "Attribytes[6]", "Attribytes[7]" },
	{ "Attribytes[0]", "Attribytes[1]", "Attribytes[2]", "Attribytes[3]", "Attribytes[4]", "Attribytes[5]", "Attribytes[6]", "Attribytes[7]" },
	{ "Attribytes[0]", "Attribytes[1]", "Attribytes[2]", "Attribytes[3]", "Attribytes[4]", "Attribytes[5]", "Attribytes[6]", "Attribytes[7]" }
};

const char defaultCustomComboAttrishorts[20][8][17]=
{
	{ "Attrishorts[0]", "Attrishorts[1]", "Attrishorts[2]", "Attrishorts[3]", "Attrishorts[4]", "Attrishorts[5]", "Attrishorts[6]", "Attrishorts[7]" },
	{ "Attrishorts[0]", "Attrishorts[1]", "Attrishorts[2]", "Attrishorts[3]", "Attrishorts[4]", "Attrishorts[5]", "Attrishorts[6]", "Attrishorts[7]" },
	{ "Attrishorts[0]", "Attrishorts[1]", "Attrishorts[2]", "Attrishorts[3]", "Attrishorts[4]", "Attrishorts[5]", "Attrishorts[6]", "Attrishorts[7]" },
	{ "Attrishorts[0]", "Attrishorts[1]", "Attrishorts[2]", "Attrishorts[3]", "Attrishorts[4]", "Attrishorts[5]", "Attrishorts[6]", "Attrishorts[7]" },
	{ "Attrishorts[0]", "Attrishorts[1]", "Attrishorts[2]", "Attrishorts[3]", "Attrishorts[4]", "Attrishorts[5]", "Attrishorts[6]", "Attrishorts[7]" },
	{ "Attrishorts[0]", "Attrishorts[1]", "Attrishorts[2]", "Attrishorts[3]", "Attrishorts[4]", "Attrishorts[5]", "Attrishorts[6]", "Attrishorts[7]" },
	{ "Attrishorts[0]", "Attrishorts[1]", "Attrishorts[2]", "Attrishorts[3]", "Attrishorts[4]", "Attrishorts[5]", "Attrishorts[6]", "Attrishorts[7]" },
	{ "Attrishorts[0]", "Attrishorts[1]", "Attrishorts[2]", "Attrishorts[3]", "Attrishorts[4]", "Attrishorts[5]", "Attrishorts[6]", "Attrishorts[7]" },
	{ "Attrishorts[0]", "Attrishorts[1]", "Attrishorts[2]", "Attrishorts[3]", "Attrishorts[4]", "Attrishorts[5]", "Attrishorts[6]", "Attrishorts[7]" },
	{ "Attrishorts[0]", "Attrishorts[1]", "Attrishorts[2]", "Attrishorts[3]", "Attrishorts[4]", "Attrishorts[5]", "Attrishorts[6]", "Attrishorts[7]" },
	{ "Attrishorts[0]", "Attrishorts[1]", "Attrishorts[2]", "Attrishorts[3]", "Attrishorts[4]", "Attrishorts[5]", "Attrishorts[6]", "Attrishorts[7]" },
	{ "Attrishorts[0]", "Attrishorts[1]", "Attrishorts[2]", "Attrishorts[3]", "Attrishorts[4]", "Attrishorts[5]", "Attrishorts[6]", "Attrishorts[7]" },
	{ "Attrishorts[0]", "Attrishorts[1]", "Attrishorts[2]", "Attrishorts[3]", "Attrishorts[4]", "Attrishorts[5]", "Attrishorts[6]", "Attrishorts[7]" },
	{ "Attrishorts[0]", "Attrishorts[1]", "Attrishorts[2]", "Attrishorts[3]", "Attrishorts[4]", "Attrishorts[5]", "Attrishorts[6]", "Attrishorts[7]" },
	{ "Attrishorts[0]", "Attrishorts[1]", "Attrishorts[2]", "Attrishorts[3]", "Attrishorts[4]", "Attrishorts[5]", "Attrishorts[6]", "Attrishorts[7]" },
	{ "Attrishorts[0]", "Attrishorts[1]", "Attrishorts[2]", "Attrishorts[3]", "Attrishorts[4]", "Attrishorts[5]", "Attrishorts[6]", "Attrishorts[7]" },
	{ "Attrishorts[0]", "Attrishorts[1]", "Attrishorts[2]", "Attrishorts[3]", "Attrishorts[4]", "Attrishorts[5]", "Attrishorts[6]", "Attrishorts[7]" },
	{ "Attrishorts[0]", "Attrishorts[1]", "Attrishorts[2]", "Attrishorts[3]", "Attrishorts[4]", "Attrishorts[5]", "Attrishorts[6]", "Attrishorts[7]" },
	{ "Attrishorts[0]", "Attrishorts[1]", "Attrishorts[2]", "Attrishorts[3]", "Attrishorts[4]", "Attrishorts[5]", "Attrishorts[6]", "Attrishorts[7]" },
	{ "Attrishorts[0]", "Attrishorts[1]", "Attrishorts[2]", "Attrishorts[3]", "Attrishorts[4]", "Attrishorts[5]", "Attrishorts[6]", "Attrishorts[7]" }
};

const char defaultCustomComboAttributes[20][4][17]=
{
	{ "Attributes[0]", "Attributes[1]", "Attributes[2]", "Attributes[3]" },
	{ "Attributes[0]", "Attributes[1]", "Attributes[2]", "Attributes[3]" },
	{ "Attributes[0]", "Attributes[1]", "Attributes[2]", "Attributes[3]" },
	{ "Attributes[0]", "Attributes[1]", "Attributes[2]", "Attributes[3]" },
	{ "Attributes[0]", "Attributes[1]", "Attributes[2]", "Attributes[3]" },
	{ "Attributes[0]", "Attributes[1]", "Attributes[2]", "Attributes[3]" },
	{ "Attributes[0]", "Attributes[1]", "Attributes[2]", "Attributes[3]" },
	{ "Attributes[0]", "Attributes[1]", "Attributes[2]", "Attributes[3]" },
	{ "Attributes[0]", "Attributes[1]", "Attributes[2]", "Attributes[3]" },
	{ "Attributes[0]", "Attributes[1]", "Attributes[2]", "Attributes[3]" },
	{ "Attributes[0]", "Attributes[1]", "Attributes[2]", "Attributes[3]" },
	{ "Attributes[0]", "Attributes[1]", "Attributes[2]", "Attributes[3]" },
	{ "Attributes[0]", "Attributes[1]", "Attributes[2]", "Attributes[3]" },
	{ "Attributes[0]", "Attributes[1]", "Attributes[2]", "Attributes[3]" },
	{ "Attributes[0]", "Attributes[1]", "Attributes[2]", "Attributes[3]" },
	{ "Attributes[0]", "Attributes[1]", "Attributes[2]", "Attributes[3]" },
	{ "Attributes[0]", "Attributes[1]", "Attributes[2]", "Attributes[3]" },
	{ "Attributes[0]", "Attributes[1]", "Attributes[2]", "Attributes[3]" },
	{ "Attributes[0]", "Attributes[1]", "Attributes[2]", "Attributes[3]" },
	{ "Attributes[0]", "Attributes[1]", "Attributes[2]", "Attributes[3]" }
};

bool ZModule::init(bool d) //bool default
{
	memset(moduledata.module_name, 0, sizeof(moduledata.module_name));
	memset(moduledata.quests, 0, sizeof(moduledata.quests));
	memset(moduledata.skipnames, 0, sizeof(moduledata.skipnames));
	memset(moduledata.datafiles, 0, sizeof(moduledata.datafiles));
	memset(moduledata.enem_type_names, 0, sizeof(moduledata.enem_type_names));
	memset(moduledata.enem_anim_type_names, 0, sizeof(moduledata.enem_anim_type_names));
	memset(moduledata.combo_type_names, 0, sizeof(moduledata.combo_type_names));
	memset(moduledata.combo_flag_names, 0, sizeof(moduledata.combo_flag_names));
	
	memset(moduledata.roomtype_names, 0, sizeof(moduledata.roomtype_names));
	memset(moduledata.walkmisc7_names, 0, sizeof(moduledata.walkmisc7_names));
	memset(moduledata.walkmisc9_names, 0, sizeof(moduledata.walkmisc9_names));
	memset(moduledata.guy_type_names, 0, sizeof(moduledata.guy_type_names));
	memset(moduledata.enemy_weapon_names, 0, sizeof(moduledata.enemy_weapon_names));
	memset(moduledata.enemy_weapon_names, 0, sizeof(moduledata.enemy_scriptweaponweapon_names)); 
	memset(moduledata.player_weapon_names, 0, sizeof(moduledata.player_weapon_names));
	memset(moduledata.counter_names, 0, sizeof(moduledata.counter_names));
	memset(moduledata.base_NSF_file, 0, sizeof(moduledata.base_NSF_file));
	memset(moduledata.copyright_strings, 0, sizeof(moduledata.copyright_strings));
	memset(moduledata.copyright_string_vars, 0, sizeof(moduledata.copyright_string_vars));
	memset(moduledata.delete_quest_data_on_wingame, 0, sizeof(moduledata.delete_quest_data_on_wingame));
	memset(moduledata.select_screen_tile_csets, 0, sizeof(moduledata.select_screen_tile_csets));
	memset(moduledata.select_screen_tiles, 0, sizeof(moduledata.select_screen_tiles));
	moduledata.old_quest_serial_flow = 0;
	memset(moduledata.startingdmap, 0, sizeof(moduledata.startingdmap));
	memset(moduledata.startingscreen, 0, sizeof(moduledata.startingscreen));
	moduledata.max_quest_files = 0;
	moduledata.animate_NES_title = 0;
	moduledata.title_track = moduledata.tf_track = moduledata.gameover_track = moduledata.ending_track = moduledata.dungeon_track = moduledata.overworld_track = moduledata.lastlevel_track = 0;
	moduledata.refresh_title_screen = 0;
	
	memset(moduledata.moduletitle, 0, sizeof(moduledata.moduletitle));
	memset(moduledata.moduleauthor, 0, sizeof(moduledata.moduleauthor));
	memset(moduledata.moduleinfo0, 0, sizeof(moduledata.moduleinfo0));
	memset(moduledata.moduleinfo1, 0, sizeof(moduledata.moduleinfo1));	
	memset(moduledata.moduleinfo2, 0, sizeof(moduledata.moduleinfo2));
	memset(moduledata.moduleinfo3, 0, sizeof(moduledata.moduleinfo3));
	memset(moduledata.moduleinfo4, 0, sizeof(moduledata.moduleinfo4));
	memset(moduledata.moduletimezone, 0, sizeof(moduledata.moduletimezone));
	memset(moduledata.combotypeCustomAttributes, 0, sizeof(moduledata.combotypeCustomAttributes));
	memset(moduledata.combotypeCustomAttribytes, 0, sizeof(moduledata.combotypeCustomAttribytes));
	memset(moduledata.combotypeCustomFlags, 0, sizeof(moduledata.combotypeCustomFlags));
	//memset(moduledata.module_base_nsf, 0, sizeof(moduledata.module_base_nsf));
		
	moduledata.modver_1 = 0;
	moduledata.modver_2 = 0;	
	moduledata.modver_3 = 0;
	moduledata.modver_4 = 0;
	moduledata.modbuild = 0;
	moduledata.modbeta = 0;
	moduledata.modmonth = 0;
	moduledata.modday = 0;
	moduledata.modyear = 0;
	moduledata.modhour = 0;
	moduledata.modminute = 0;
	
	//strcpy(moduledata.module_name,"default.zmod");
	//al_trace("Module name set to %s\n",moduledata.module_name);
	//We load the current module name from zc.cfg or zquest.cfg!
	//Otherwise, we don't know what file to access to load the module vars! 
	strcpy(moduledata.module_name,zc_get_config("ZCMODULE","current_module","modules/classic.zmod"));
	al_trace("The Current ZQuest Creator Module is: %s\n",moduledata.module_name); 
	if(!fileexists((char*)moduledata.module_name))
	{
		Z_error_fatal("ZQuest Creator I/O Error:\nNo module definitions found.\nZQuest Creator cannot run without these definitions,\nand is now exiting.\nPlease check your settings in %s.cfg.\n","zcl");
		exit(1);
		return false;
	}
	if ( d )
	{
		
		//zcm path
		set_config_file(moduledata.module_name); //Switch to the module to load its config properties.
		//al_trace("Module name set to %s\n",moduledata.module_name);
		
		//Metadata
		strcpy(moduledata.moduletitle,zc_get_config("METADATA","title",""));
		strcpy(moduledata.moduleauthor,zc_get_config("METADATA","author",""));
		strcpy(moduledata.moduleinfo0,zc_get_config("METADATA","info_0",""));
		strcpy(moduledata.moduleinfo1,zc_get_config("METADATA","info_1",""));
		strcpy(moduledata.moduleinfo2,zc_get_config("METADATA","info_2",""));
		strcpy(moduledata.moduleinfo3,zc_get_config("METADATA","info_3",""));
		strcpy(moduledata.moduleinfo4,zc_get_config("METADATA","info_4",""));
		strcpy(moduledata.moduletimezone,zc_get_config("METADATA","timezone","GMT"));
		//strcpy(moduledata.module_base_nsf,zc_get_config("METADATA","nsf",""));
		moduledata.modver_1 = zc_get_config("METADATA","version_first",0);
		moduledata.modver_2 = zc_get_config("METADATA","version_second",0);	
		moduledata.modver_3 = zc_get_config("METADATA","version_third",0);
		moduledata.modver_4 = zc_get_config("METADATA","version_fourth",0);
		moduledata.modbuild = zc_get_config("METADATA","version_build",0);
		moduledata.modbeta = zc_get_config("METADATA","version_beta",0);
		moduledata.modmonth = zc_get_config("METADATA","version_month",0);
		moduledata.modday = zc_get_config("METADATA","version_day",0);
		moduledata.modyear = zc_get_config("METADATA","version_year",0);
		moduledata.modhour = zc_get_config("METADATA","version_hour",0);
		moduledata.modminute = zc_get_config("METADATA","version_minute",0); 
		
		//quests
		moduledata.old_quest_serial_flow = zc_get_config("QUESTS","quest_flow",1);
		moduledata.max_quest_files = vbound(zc_get_config("QUESTS","num_quest_files",0),0,10);
		strcpy(moduledata.quests[0],zc_get_config("QUESTS","first_qst","-"));
		strcpy(moduledata.quests[1],zc_get_config("QUESTS","second_qst","-"));
		strcpy(moduledata.quests[2],zc_get_config("QUESTS","third_qst","-"));
		strcpy(moduledata.quests[3],zc_get_config("QUESTS","fourth_qst","-"));
		strcpy(moduledata.quests[4],zc_get_config("QUESTS","fifth_qst","-"));
		strcpy(moduledata.quests[5],zc_get_config("QUESTS","sixth_qst","-"));
		strcpy(moduledata.quests[6],zc_get_config("QUESTS","seventh_qst","-"));
		strcpy(moduledata.quests[7],zc_get_config("QUESTS","eighth_qst","-"));
		strcpy(moduledata.quests[8],zc_get_config("QUESTS","ninth_qst","-"));
		strcpy(moduledata.quests[9],zc_get_config("QUESTS","tenth_qst","-"));
		for ( int32_t q = 0; q < 10; q++ )
		{
			if ( moduledata.quests[q][0] == '-' ) strcpy(moduledata.quests[q],"");
		}
		
		//quest skip names
		moduledata.skipnames[0][0] = 0;
		strcpy(moduledata.skipnames[1],zc_get_config("NAMEENTRY","second_qst_skip","-"));
		strcpy(moduledata.skipnames[2],zc_get_config("NAMEENTRY","third_qst_skip","-"));
		strcpy(moduledata.skipnames[3],zc_get_config("NAMEENTRY","fourth_qst_skip","-"));
		strcpy(moduledata.skipnames[4],zc_get_config("NAMEENTRY","fifth_qst_skip","-"));
		strcpy(moduledata.skipnames[5],zc_get_config("NAMEENTRY","sixth_qst_skip","-"));
		strcpy(moduledata.skipnames[6],zc_get_config("NAMEENTRY","seventh_qst_skip","-"));
		strcpy(moduledata.skipnames[7],zc_get_config("NAMEENTRY","eighth_qst_skip","-"));
		strcpy(moduledata.skipnames[8],zc_get_config("NAMEENTRY","ninth_qst_skip","-"));
		strcpy(moduledata.skipnames[9],zc_get_config("NAMEENTRY","tenth_qst_skip","-"));
		
		//datafiles
		strcpy(moduledata.datafiles[zelda_dat],zc_get_config("DATAFILES","zcplayer_datafile","zelda.dat"));
		al_trace("Module zelda_dat set to %s\n",moduledata.datafiles[zelda_dat]);
		strcpy(moduledata.datafiles[zquest_dat],zc_get_config("DATAFILES","zquest_datafile","zquest.dat"));
		al_trace("Module zquest_dat set to %s\n",moduledata.datafiles[zquest_dat]);
		strcpy(moduledata.datafiles[fonts_dat],zc_get_config("DATAFILES","fonts_datafile","fonts.dat"));
		al_trace("Module fonts_dat set to %s\n",moduledata.datafiles[fonts_dat]);
		strcpy(moduledata.datafiles[sfx_dat],zc_get_config("DATAFILES","sounds_datafile","sfx.dat"));
		al_trace("Module sfx_dat set to %s\n",moduledata.datafiles[sfx_dat]);
		strcpy(moduledata.datafiles[qst_dat],zc_get_config("DATAFILES","quest_template_datafile","qst.dat"));
		al_trace("Module qst_dat set to %s\n",moduledata.datafiles[qst_dat]);
		
		
		strcpy(moduledata.base_NSF_file,zc_get_config("DATAFILES","base_NSF_file","zelda.nsf"));
		al_trace("Base NSF file: %s\n", moduledata.base_NSF_file);
		
		moduledata.title_track = zc_get_config("DATAFILES","title_track",0);
		moduledata.ending_track = zc_get_config("DATAFILES","ending_track",1);
		moduledata.tf_track = zc_get_config("DATAFILES","tf_track",5);
		moduledata.gameover_track = zc_get_config("DATAFILES","gameover_track",0);
		moduledata.dungeon_track = zc_get_config("DATAFILES","dungeon_track",0);
		moduledata.overworld_track = zc_get_config("DATAFILES","overworld_track",0);
		moduledata.lastlevel_track = zc_get_config("DATAFILES","lastlevel_track",0);
		
		const char enemy_family_strings[eeMAX][255] =
		{
			"ee_family_guy","ee_family_walk","ee_family_shoot","ee_family_tek","ee_family_lev",
			"ee_family_pea","ee_family_zor","ee_family_rock","ee_family_gh","ee_family_arm",
			//10
			"ee_family_ke","ee_family_ge","ee_family_zl","ee_family_rp","ee_family_gor",
			"ee_family_trap","ee_family_wm","ee_family_jinx","ee_family_vir","ee_family_rike",
			//20
			"ee_family_pol","ee_family_wiz","ee_family_aqu","ee_family_mold","ee_family_dod",
			"ee_family_mhd","ee_family_glk","ee_family_dig","ee_family_goh","ee_family_lan",
			//30
			"ee_family_pat","ee_family_gan","ee_family_proj","ee_family_gtrib","ee_family_ztrib",
			"ee_family_vitrib","ee_family_ketrib","ee_family_spintile","ee_family_none","ee_family_faerie",
			//40
			"ee_family_otherflt","ee_family_other", "max250",
			"Custom_01", "Custom_02", "Custom_03", "Custom_04", "Custom_05",
			"Custom_06", "Custom_07", "Custom_08", "Custom_09", "Custom_10",
			"Custom_11", "Custom_12", "Custom_13", "Custom_14", "Custom_15",
			"Custom_16", "Custom_17", "Custom_18", "Custom_19", "Custom_20",
			"Friendly_NPC_01", "Friendly_NPC_02", "Friendly_NPC_03", "Friendly_NPC_04",
			"Friendly_NPC_05", "Friendly_NPC_06", "Friendly_NPC_07",
			"Friendly_NPC_08", "Friendly_NPC_09", "Friendly_NPC_10"
		};
		
		const char default_enemy_types[eeMAX][255] =
		{
		    "-Guy","Walking Enemy","-Unused","Tektite","Leever",
			"Peahat","Zora","Rock","Ghini","-Unused",
			"Keese","-Unused","-Unused","-Unused","-Unused",//goriya
			"Trap","Wall Master","-Unused","-Unused","-Unused",//likelike
			"-Unused","Wizzrobe","Aquamentus","Moldorm","Dodongo",
			"Manhandla","Gleeok","Digdogger","Gohma","Lanmola",
			"Patra","Ganon","Projectile Shooter","-Unused","-Unused",//zol trib
			"-Unused","-Unused","Spin Tile","(None)","-Fairy","Other (Floating)","Other",
			"-max250",
  		        "Custom 01", "Custom 02", "Custom 03", "Custom 04", "Custom 05",
		        "Custom 06", "Custom 07", "Custom 08", "Custom 09", "Custom 10",
		        "Custom 11", "Custom 12", "Custom 13", "Custom 14", "Custom 15",
		        "Custom 16", "Custom 17", "Custom 18", "Custom 19", "Custom 20",
		        "Friendly NPC 01", "Friendly NPC 02", "Friendly NPC 03", "Friendly NPC 04",
		        "Friendly NPC 05", "Friendly NPC 06", "Friendly NPC 07",
		        "Friendly NPC 08", "Friendly NPC 09", "Friendly NPC 10"
		};
		for ( int32_t q = 0; q < eeMAX; q++ )
		{
			strcpy(moduledata.enem_type_names[q],zc_get_config("ENEMIES",enemy_family_strings[q],default_enemy_types[q]));
			//al_trace("Enemy family ID %d is: %s\n", q, moduledata.enem_type_names[q]);
		}
		const char default_enemy_anims[aMAX][255] =
		{
			"(None)","Flip","-Unused","2-Frame","-Unused",
			"Octorok (NES)","Tektite (NES)","Leever (NES)","Walker","Zora (NES)",
			"Zora (4-Frame)","Ghini","Armos (NES)","Rope","Wall Master (NES)",
			"Wall Master (4-Frame)","Darknut (NES)","Vire","3-Frame","Wizzrobe (NES)",
			"Aquamentus","Dodongo (NES)","Manhandla","Gleeok","Digdogger",
			"Gohma","Lanmola","2-Frame Flying","4-Frame 4-Dir + Tracking","4-Frame 8-Dir + Tracking",
			"4-Frame 4-Dir + Firing","4-Frame 4-Dir","4-Frame 8-Dir + Firing","Armos (4-Frame)","4-Frame Flying 4-Dir",
			"4-Frame Flying 8-Dir","-Unused","4-Frame 8-Dir Big","Tektite (4-Frame)","3-Frame 4-Dir",
			"2-Frame 4-Dir","Leever (4-Frame)","2-Frame 4-Dir + Tracking","Wizzrobe (4-Frame)","Dodongo (4-Frame)",
			"Dodongo BS (4-Frame)","4-Frame Flying 8-Dir + Firing","4-Frame Flying 4-Dir + Firing","4-Frame","Ganon",
			"2-Frame Big", "4-Frame 8-Dir Big + Tracking", "4-Frame 4-Dir Big + Tracking", "4-Frame 8-Dir Big + Firing",
			"4-Frame 4-Dir Big", "4-Frame 4-Dir Big + Firing"
		};
		const char enemy_anim_strings[aMAX][255] =
		{
			"ea_none","ea_flip","ea_unused1","ea_2frame","ea_unused2",
			"ea_oct","ea_tek","ea_lev","ea_walk","ea_zor",
			"ea_zor4","ea_gh","ea_arm","ea_rp","ea_wm",
			"ea_wm4","ea_dkn","ea_vir", "ea_3f","ea_wiz",
			"ea_aqu","ea_dod","ea_mhn","ea_gkl","ea_dig",
			"ea_goh","ea_lan","ea_fly2","ea_4f4dT","ea_4f8dT",
			"ea_4f4dF","ea_4f4d","ea_4f8dF","ea_arm","ea_fly_4f4d",
			"ea_fly4f8d","ea_unused3","ea_4f8dLG","ea_tek4","ea_3f4d",
			"ea_2f4d","ea_lev4","ea_2f4dT","ea_wiz4","ea_dod4",
			"ea_bsdod","ea_fly4f4dT","ea_fly_4f4dF","ea_4f","ea_gan",
			"ea_2fLG"
		};
		for ( int32_t q = 0; q < aMAX; q++ )
		{
			strcpy(moduledata.enem_anim_type_names[q],zc_get_config("ENEMIES",enemy_anim_strings[q],default_enemy_anims[q]));
			//al_trace("Enemy animation type ID %d is: %s\n", q, moduledata.enem_anim_type_names[q]);
		}
		
		//combo editor
		const char combo_name_fields[cMAX][255]=
		{
		    "cNONE", "cSTAIR", "cCAVE", "cLIQUID", "cSTATUE", "cGRAVE", "cDOCK",
		    "cUNDEF", "cPUSH_WAIT", "cPUSH_HEAVY", "cPUSH_HW", "cL_STATUE", "cR_STATUE",
		    "cWALKSLOW", "cCVUP", "cCVDOWN", "cCVLEFT", "cCVRIGHT", "cSWIMWARP", "cDIVEWARP",
		    "cLADDERORGRAPPLE", "cTRIGNOFLAG", "cTRIGFLAG", "cWINGAME", "cSLASH", "cSLASHITEM",
		    "cPUSH_HEAVY2", "cPUSH_HW2", "cPOUND", "cHSGRAB", "cHSBRIDGE", "cDAMAGE1",
		    "cDAMAGE2", "cDAMAGE3", "cDAMAGE4", "cC_STATUE", "cTRAP_H", "cTRAP_V", "cTRAP_4",
		    "cTRAP_LR", "cTRAP_UD", "cPIT", "cGRAPPLEONLY", "cOVERHEAD", "cNOFLYZONE", "cMIRROR",
		    "cMIRRORSLASH", "cMIRRORBACKSLASH", "cMAGICPRISM", "cMAGICPRISM4",
		    "cMAGICSPONGE", "cCAVE2", "cEYEBALL_A", "cEYEBALL_B", "cNOJUMPZONE", "cBUSH",
		    "cFLOWERS", "cTALLGRASS", "cSHALLOWLIQUID", "cLOCKBLOCK", "cLOCKBLOCK2",
		    "cBOSSLOCKBLOCK", "cBOSSLOCKBLOCK2", "cLADDERONLY", "cBSGRAVE",
		    "cCHEST", "cCHEST2", "cLOCKEDCHEST", "cLOCKEDCHEST2", "cBOSSCHEST", "cBOSSCHEST2",
		    "cRESET", "cSAVE", "cSAVE2", "cCAVEB", "cCAVEC", "cCAVED",
		    "cSTAIRB", "cSTAIRC", "cSTAIRD", "cPITB", "cPITC", "cPITD",
		    "cCAVE2B", "cCAVE2C", "cCAVE2D", "cSWIMWARPB", "cSWIMWARPC", "cSWIMWARPD",
		    "cDIVEWARPB", "cDIVEWARPC", "cDIVEWARPD", "cSTAIRR", "cPITR",
		    "cAWARPA", "cAWARPB", "cAWARPC", "cAWARPD", "cAWARPR",
		    "cSWARPA", "cSWARPB", "cSWARPC", "cSWARPD", "cSWARPR", "cSTRIGNOFLAG", "cSTRIGFLAG",
		    "cSTEP", "cSTEPSAME", "cSTEPALL", "cSTEPCOPY", "cNOENEMY", "cBLOCKARROW1", "cBLOCKARROW2",
		    "cBLOCKARROW3", "cBLOCKBRANG1", "cBLOCKBRANG2", "cBLOCKBRANG3", "cBLOCKSBEAM", "cBLOCKALL",
		    "cBLOCKFIREBALL", "cDAMAGE5", "cDAMAGE6", "cDAMAGE7", "cCHANGE", "cSPINTILE1", "cSPINTILE2",
		    "cSCREENFREEZE", "cSCREENFREEZEFF", "cNOGROUNDENEMY", "cSLASHNEXT", "cSLASHNEXTITEM", "cBUSHNEXT",
		    "cSLASHTOUCHY", "cSLASHITEMTOUCHY", "cBUSHTOUCHY", "cFLOWERSTOUCHY", "cTALLGRASSTOUCHY",
		    "cSLASHNEXTTOUCHY", "cSLASHNEXTITEMTOUCHY", "cBUSHNEXTTOUCHY", "cEYEBALL_4", "cTALLGRASSNEXT",
		    "cSCRIPT1", "cSCRIPT2", "cSCRIPT3", "cSCRIPT4", "cSCRIPT5",
		    "cSCRIPT6", "cSCRIPT7", "cSCRIPT8", "cSCRIPT9", "cSCRIPT10",
		    "cSCRIPT11", "cSCRIPT12", "cSCRIPT13", "cSCRIPT14", "cSCRIPT15",
		    "cSCRIPT16", "cSCRIPT17", "cSCRIPT18", "cSCRIPT19", "cSCRIPT20",
		    "cTRIGGERGENERIC", "cPITFALL", "cSTEPSFX", "cBRIDGE", "cSIGNPOST",
		    "cCSWITCH", "cCSWITCHBLOCK", "cTORCH", "cSPOTLIGHT", "cGLASS", "cLIGHTTARGET", "cSWITCHHOOK"
		};
		
		const char default_ctype_strings[cMAX][255] = 
		{
			"(None)", "Stairs [A]", "Cave (Walk Down) [A]", "Liquid", "Armos",
			"Grave", "Dock", "-UNDEF", "Push (Wait)", "Push (Heavy)",
			"Push (Heavy, Wait)", "Left Statue", "Right Statue", "Slow Walk", "Conveyor Up",
			"Conveyor Down", "Conveyor Left", "Conveyor Right", "Swim Warp [A]", "Dive Warp [A]",
			"Ladder or Hookshot", "Step->Secrets (Temporary)", "Step->Secrets (Permanent)", "-WINGAME", "Slash",
			"Slash (Item)", "Push (Very Heavy)", "Push (Very Heavy, Wait)", "Pound", "Hookshot Grab",
			"-HSBRIDGE", "Damage (1/2 Heart)", "Damage (1 Heart)", "Damage (2 hearts)", "Damage (4 Hearts)",
			"Center Statue", "Trap (Horizontal, Line of Sight)", "Trap (Vertical, Line of Sight)", "Trap (4-Way)", "Trap (Horizontal, Constant)",
			"Trap (Vertical, Constant)", "Direct Warp [A]", "Hookshot Only", "Overhead", "No Flying Enemies",
			"Magic Mirror (4-Way)", "Magic Mirror (Up-Left, Down-Right)", "Magic Mirror (Up-Right, Down-Left)", "Magic Prism (3-Way)", "Magic Prism (4-Way)",
			"Block Magic", "Cave (Walk Up) [A]", "Eyeball (8-Way A)", "Eyeball (8-Way B)", "No Jumping Enemies",
			"Bush", "Flowers", "Tall Grass", "Shallow Liquid", "Lock Block (Normal)",
			"Lock Block (Normal, Copycat)", "Lock Block (Boss)", "Lock Block (Boss, Copycat)", "Ladder Only", "BS Grave",
			"Chest (Basic)", "Chest (Basic, Copycat)", "Chest (Locked)", "Chest (Locked, Copycat)", "Chest (Boss)",
			"Chest (Boss, Copycat)", "Reset Room", "Save Point", "Save-Quit Point", "Cave (Walk Down) [B]",
			"Cave (Walk Down) [C]", "Cave (Walk Down) [D]", "Stairs [B]", "Stairs [C]", "Stairs [D]",
			"Direct Warp [B]", "Direct Warp [C]", "Direct Warp [D]", "Cave (Walk Up) [B]", "Cave (Walk Up) [C]",
			"Cave (Walk Up) [D]", "Swim Warp [B]", "Swim Warp [C]", "Swim Warp [D]", "Dive Warp [B]",
			"Dive Warp [C]", "Dive Warp [D]", "Stairs [Random]", "Direct Warp [Random]", "Auto Side Warp [A]",
			"Auto Side Warp [B]","Auto Side Warp [C]","Auto Side Warp [D]","Auto Side Warp [Random]","Sensitive Warp [A]",
			"Sensitive Warp [B]","Sensitive Warp [C]","Sensitive Warp [D]","Sensitive Warp [Random]","Step->Secrets (Sensitive, Temp)",
			"Step->Secrets (Sensitive, Perm.)","Step->Next","Step->Next (Same)","Step->Next (All)","Step->Next (Copycat)",
			"No Enemies","Block Arrow (L1)","Block Arrow (L1, L2)","Block Arrow (All)","Block Brang (L1)",
			"Block Brang (L1, L2)","Block Brang (All)","Block Sword Beam","Block All","Block Fireball",
			"Damage (8 hearts)","Damage (16 hearts)","Damage (32 hearts)","-Unused","Spinning Tile (Immediate)",
			"-Unused","Screen Freeze (Except FFCs)","Screen Freeze (FFCs Only)","No Ground Enemies","Slash->Next",
			"Slash->Next (Item)","Bush->Next","Slash (Continuous)","Slash (Item, Continuous)","Bush (Continuous)",
			"Flowers (Continuous)","Tall Grass (Continuous)","Slash->Next (Continuous)","Slash->Next (Item, Continuous)","Bush->Next (Continuous)",
			"Eyeball (4-Way)","Tall Grass->Next","Script 01","Script 02","Script 03",
			"Script 04","Script 05","Script 06","Script 07","Script 08",
			"Script 09","Script 10","Script 11","Script 12","Script 13",
			"Script 14","Script 15","Script 16","Script 17","Script 18",
			"Script 19", "Script 20", "Generic", "Pitfall", "Step->Effects",
			"Bridge", "Signpost", "Switch", "Switch Block", "Torch",
			"Spotlight", "Glass", "Light Trigger", "SwitchHook Block"
		};
		
		for ( int32_t q = 0; q < cMAX; q++ )
		{
			if(!default_ctype_strings[q] || default_ctype_strings[q][0]=='-')
			{
				strcpy(moduledata.combo_type_names[q],"-");
			}
			else strcpy(moduledata.combo_type_names[q],zc_get_config("COMBOS",combo_name_fields[q],default_ctype_strings[q]));
		}
		
		//map flags
		
		const char map_flag_cats[mfMAX][256]=
		{
			"mfNONE","mfPUSHUD","mfPUSH4","mfWHISTLE","mfBCANDLE","mfARROW","mfBOMB","mfFAIRY","mfRAFT","mfSTATUE_SECRET","mfSTATUE_ITEM","mfSBOMB","mfRAFT_BRANCH","mfDIVE_ITEM","mfLENSMARKER","mfWINGAME",
			"mfSECRETS01","mfSECRETS02","mfSECRETS03","mfSECRETS04","mfSECRETS05","mfSECRETS06","mfSECRETS07","mfSECRETS08","mfSECRETS09","mfSECRETS10","mfSECRETS11","mfSECRETS12","mfSECRETS13","mfSECRETS14","mfSECRETS15","mfSECRETS16",
			"mfTRAP_H","mfTRAP_V","mfTRAP_4","mfTRAP_LR","mfTRAP_UD","mfENEMY0","mfENEMY1","mfENEMY2","mfENEMY3","mfENEMY4","mfENEMY5","mfENEMY6","mfENEMY7","mfENEMY8","mfENEMY9","mfPUSHLR",
			"mfPUSHU","mfPUSHD","mfPUSHL","mfPUSHR","mfPUSHUDNS","mfPUSHLRNS","mfPUSH4NS","mfPUSHUNS","mfPUSHDNS","mfPUSHLNS","mfPUSHRNS","mfPUSHUDINS","mfPUSHLRINS","mfPUSH4INS","mfPUSHUINS","mfPUSHDINS",
			"mfPUSHLINS","mfPUSHRINS","mfBLOCKTRIGGER","mfNOBLOCKS","mfBRANG","mfMBRANG","mfFBRANG","mfSARROW","mfGARROW","mfRCANDLE","mfWANDFIRE","mfDINSFIRE","mfWANDMAGIC","mfREFMAGIC","mfREFFIREBALL","mfSWORD",
			"mfWSWORD","mfMSWORD","mfXSWORD","mfSWORDBEAM","mfWSWORDBEAM","mfMSWORDBEAM","mfXSWORDBEAM","mfGRAPPLE","mfWAND","mfHAMMER","mfSTRIKE","mfBLOCKHOLE","mfMAGICFAIRY","mfALLFAIRY","mfSINGLE","mfSINGLE16",
			"mfNOENEMY","mfNOGROUNDENEMY","mfSCRIPT1","mfSCRIPT2","mfSCRIPT3","mfSCRIPT4","mfSCRIPT5","mfRAFT_BOUNCE","mfPUSHED","mfSCRIPT6","mfSCRIPT7","mfSCRIPT8","mfSCRIPT9","mfSCRIPT10","mfSCRIPT11","mfSCRIPT12",
			"mfSCRIPT13","mfSCRIPT14","mfSCRIPT15","mfSCRIPT16","mfSCRIPT17","mfSCRIPT18","mfSCRIPT19","mfSCRIPT20","mfPITHOLE","mfPITFALLFLOOR","mfLAVA","mfICE","mfICEDAMAGE","mfDAMAGE1","mfDAMAGE2","mfDAMAGE4",
			"mfDAMAGE8","mfDAMAGE16","mfDAMAGE32","mfFREEZEALL","mfFREZEALLANSFFCS","mfFREEZEFFCSOLY","mfSCRITPTW1TRIG","mfSCRITPTW2TRIG","mfSCRITPTW3TRIG","mfSCRITPTW4TRIG","mfSCRITPTW5TRIG","mfSCRITPTW6TRIG","mfSCRITPTW7TRIG","mfSCRITPTW8TRIG","mfSCRITPTW9TRIG","mfSCRITPTW10TRIG",
			"mfTROWEL","mfTROWELNEXT","mfTROWELSPECIALITEM","mfSLASHPOT","mfLIFTPOT","mfLIFTORSLASH","mfLIFTROCK","mfLIFTROCKHEAVY","mfDROPITEM","mfSPECIALITEM","mfDROPKEY","mfDROPLKEY","mfDROPCOMPASS","mfDROPMAP","mfDROPBOSSKEY","mfSPAWNNPC",
			"mfSWITCHHOOK","mfSIDEVIEWLADDER","mfSIDEVIEWPLATFORM","mfNOENEMYSPAWN","mfENEMYALL","mfSECRETSNEXT","mfNOMIRROR","mf167","mf168","mf169","mf170","mf171","mf172","mf173","mf174","mf175",
			"mf176","mf177","mf178","mf179","mf180","mf181","mf182","mf183","mf184","mf185","mf186","mf187","mf188","mf189","mf190","mf191",
			"mf192","mf193","mf194","mf195","mf196","mf197","mf198","mf199","mf200","mf201","mf202","mf203","mf204","mf205","mf206","mf207",
			"mf208","mf209","mf210","mf211","mf212","mf213","mf214","mf215","mf216","mf217","mf218","mf219","mf220","mf221","mf222","mf223",
			"mf224","mf225","mf226","mf227","mf228","mf229","mf230","mf231","mf232","mf233","mf234","mf235","mf236","mf237","mf238","mf239",
			"mf240","mf241","mf242","mf243","mf244","mf245","mf246","mf247","mf248","mf249","mf250","mf251","mf252","mf253","mf254","mfEXTENDED"
		};
		const char map_flag_default_string[mfMAX][255] =
		{
			"(None)", "Push Block (Vertical, Trigger)", "Push Block (4-Way, Trigger)", "Whistle Trigger", "Burn Trigger (Any)", "Arrow Trigger (Any)", "Bomb Trigger (Any)", "Fairy Ring (Life)",
			"Raft Path", "Armos -> Secret", "Armos/Chest -> Item", "Bomb (Super)", "Raft Branch", "Dive -> Item", "Lens Marker", "Zelda (Win Game)",
			"Secret Tile 0", "Secret Tile 1", "Secret Tile 2", "Secret Tile 3", "Secret Tile 4", "Secret Tile 5", "Secret Tile 6", "Secret Tile 7",
			"Secret Tile 8", "Secret Tile 9", "Secret Tile 10", "Secret Tile 11", "Secret Tile 12", "Secret Tile 13", "Secret Tile 14", "Secret Tile 15",
			"Trap (Horizontal, Line of Sight)", "Trap (Vertical, Line of Sight)", "Trap (4-Way, Line of Sight)", "Trap (Horizontal, Constant)", "Trap (Vertical, Constant)", "Enemy 0", "Enemy 1", "Enemy 2",
			"Enemy 3", "Enemy 4", "Enemy 5", "Enemy 6", "Enemy 7", "Enemy 8", "Enemy 9", "Push Block (Horiz, Once, Trigger)",
			"Push Block (Up, Once, Trigger)", "Push Block (Down, Once, Trigger)", "Push Block (Left, Once, Trigger)", "Push Block (Right, Once, Trigger)", "Push Block (Vert, Once)", "Push Block (Horizontal, Once)", "Push Block (4-Way, Once)", "Push Block (Up, Once)",
			"Push Block (Down, Once)", "Push Block (Left, Once)", "Push Block (Right, Once)", "Push Block (Vertical, Many)", "Push Block (Horizontal, Many)", "Push Block (4-Way, Many)", "Push Block (Up, Many)", "Push Block (Down, Many)",
			"Push Block (Left, Many)", "Push Block (Right, Many)", "Block Trigger", "No Push Blocks", "Boomerang Trigger (Any)", "Boomerang Trigger (Magic +)", "Boomerang Trigger (Fire)", "Arrow Trigger (Silver +)",
			"Arrow Trigger (Golden)", "Burn Trigger (Red Candle +)", "Burn Trigger (Wand Fire)", "Burn Trigger (Din's Fire)", "Magic Trigger (Wand)", "Magic Trigger (Reflected)", "Fireball Trigger (Reflected)", "Sword Trigger (Any)",
			"Sword Trigger (White +)", "Sword Trigger (Magic +)", "Sword Trigger (Master)", "Sword Beam Trigger (Any)", "Sword Beam Trigger (White +)", "Sword Beam Trigger (Magic +)", "Sword Beam Trigger (Master)", "Hookshot Trigger",
			"Wand Trigger", "Hammer Trigger", "Strike Trigger", "Block Hole (Block -> Next)", "Fairy Ring (Magic)", "Fairy Ring (All)", "Trigger -> Self Only", "Trigger -> Self, Secret Tiles",
			"No Enemies", "No Ground Enemies", "General Purpose 1 (Scripts)", "General Purpose 2 (Scripts)", "General Purpose 3 (Scripts)", "General Purpose 4 (Scripts)", "General Purpose 5 (Scripts)", "Raft Bounce",
			 "Pushed", "General Purpose 6 (Scripts)", "General Purpose 7 (Scripts)", "General Purpose 8 (Scripts)", "General Purpose 9 (Scripts)", "General Purpose 10 (Scripts)", "General Purpose 11 (Scripts)", "General Purpose 12 (Scripts)",
			"General Purpose 13 (Scripts)", "General Purpose 14 (Scripts)", "General Purpose 15 (Scripts)", "General Purpose 16 (Scripts)", "General Purpose 17 (Scripts)", "General Purpose 18 (Scripts)", "General Purpose 19 (Scripts)", "General Purpose 20 (Scripts)",
			"Pit or Hole (Scripted)", "Pit or Hole, Fall Down Floor (Scripted)", "Fire or Lava (Scripted)", "Ice (Scripted)", "Ice, Damaging (Scripted)", "Damage-1 (Scripted)", "Damage-2 (Scripted)", "Damage-4 (Scripted)",
			"Damage-8 (Scripted)", "Damage-16 (Scripted)", "Damage-32 (Scripted)", "Freeze Screen (Unimplemented)", "Freeze Screen, Except FFCs (Unimplemented)", "Freeze FFCs Only (Unimplemented)", "Trigger LW_SCRIPT1 (Unimplemented)", "Trigger LW_SCRIPT2 (Unimplemented)",
			"Trigger LW_SCRIPT3 (Unimplemented)", "Trigger LW_SCRIPT4 (Unimplemented)", "Trigger LW_SCRIPT5 (Unimplemented)", "Trigger LW_SCRIPT6 (Unimplemented)", "Trigger LW_SCRIPT7 (Unimplemented)", "Trigger LW_SCRIPT8 (Unimplemented)", "Trigger LW_SCRIPT9 (Unimplemented)", "Trigger LW_SCRIPT10 (Unimplemented)",
			"Dig Spot (Scripted)", "Dig Spot, Next (Scripted)", "Dig Spot, Special Item (Scripted)", "Pot, Slashable (Scripted)", "Pot, Liftable (Scripted)", "Pot, Slash or Lift (Scripted)", "Rock, Lift Normal (Scripted)", "Rock, Lift Heavy (Scripted)",
			"Dropset Item (Scripted)", "Special Item (Scripted)", "Drop Key (Scripted)", "Drop level-Specific Key (Scripted)", "Drop Compass (Scripted)", "Drop Map (Scripted)", "Drop Bosskey (Scripted)", "Spawn NPC (Scripted)",
			"SwitchHook Spot (Scripted)", "Sideview Ladder", "Sideview Platform","Spawn No Enemies","Spawn All Enemies","Secrets->Next","No Mirroring","-mf167","-mf168","-mf169", "-mf170","-mf171","-mf172","-mf173","-mf174","-mf175","-mf176","-mf177","-mf178","-mf179",
			"-mf180","-mf181","-mf182","-mf183","-mf184","-mf185","-mf186","-mf187","-mf188","-mf189", "-mf190","-mf191","-mf192","-mf193","-mf194","-mf195","-mf196","-mf197","-mf198","-mf199",
			"-mf200","-mf201","-mf202","-mf203","-mf204","-mf205","-mf206","-mf207","-mf208","-mf209", "-mf210","-mf211","-mf212","-mf213","-mf214","-mf215","-mf216","-mf217","-mf218","-mf219",
			"-mf220","-mf221","-mf222","-mf223","-mf224","-mf225","-mf226","-mf227","-mf228","-mf229", "-mf230","-mf231","-mf232","-mf233","-mf234","-mf235","-mf236","-mf237","-mf238","-mf239",
			"-mf240","-mf241","-mf242","-mf243","-mf244","-mf245","-mf246","-mf247","-mf248","-mf249", "-mf250","-mf251","-mf252","-mf253","-mf254",
			"Extended (Extended Flag Editor)"
		};
		for ( int32_t q = 0; q < mfMAX; q++ )
		{
			if(map_flag_default_string[q][0] == '-')
				strcpy(moduledata.combo_flag_names[q], map_flag_default_string[q]);
			else strcpy(moduledata.combo_flag_names[q],zc_get_config("MAPFLAGS",map_flag_cats[q],map_flag_default_string[q]));
		}
		const char roomtype_cats[rMAX][256] =
		{
			"rNONE","rSP_ITEM","rINFO","rMONEY","rGAMBLE","rREPAIR","rRP_HC","rGRUMBLE",
			"rQUESTOBJ","rP_SHOP","rSHOP","rBOMBS","rSWINDLE","r10RUPIES","rWARP","rMAINBOSS","rWINGAME",
			"rITEMPOND","rMUPGRADE","rLEARNSLASH","rARROWS","rTAKEONE","rBOTTLESHOP"
		};
		const char roomtype_defaults[rMAX][255] =
		{
		    "(None)","Special Item","Pay for Info","Secret Money","Gamble",
		    "Door Repair","Red Potion or Heart Container","Feed the Goriya","Triforce Check",
		    "Potion Shop","Shop","More Bombs","Leave Money or Life","10 Rupees",
		    "3-Stair Warp","Ganon","Zelda", "-<item pond>", "1/2 Magic Upgrade", "Learn Slash",
			"More Arrows","Take One Item","Bottle Shop"
		};
		for ( int32_t q = 0; q < rMAX; q++ )
		{
			strcpy(moduledata.roomtype_names[q],zc_get_config("ROOMTYPES",roomtype_cats[q],roomtype_defaults[q]));
			//al_trace("Map Flag ID %d is: %s\n", q, moduledata.roomtype_names[q]);
		}
		
		const char enemy_walk_type_defaults[e9tARMOS+1][255] =
		{
		    "Normal", "Rope", "Vire", "Pols Voice", "Armos"
		};

		const char enemy_walk_style_cats[e9tARMOS+1][255]=
		{
			"wsNormal","wsCharge","wsHopSplit","wsHop","wsStatue"
		};
		for ( int32_t q = 0; q < e9tARMOS+1; q++ )
		{
			strcpy(moduledata.walkmisc9_names[q],zc_get_config("ENEMYWALKSTYLE",enemy_walk_style_cats[q],enemy_walk_type_defaults[q]));
			//al_trace("Map Flag ID %d is: %s\n", q, moduledata.walkmisc9_names[q]);
		}
		const char guy_types[gDUMMY1][255]=
		{
			"gNONE", "gOLDMAN", "gOLDWOMAN", "gDUDE", "gORC",
		    "gFIRE", "gFAIRY", "gGRUMBLE", "gPRINCESS", "gOLDMAN2",
		    "gEMPTY"
		};

		const char guy_default_names[gDUMMY1][255]=
		{
			"(None)","Abei","Ama","Merchant","Moblin","Fire",
			"Fairy","Goriya","Zelda","Abei 2","Empty"
		};
		for ( int32_t q = 0; q < gDUMMY1; q++ )
		{
			strcpy(moduledata.guy_type_names[q],zc_get_config("GUYS",guy_types[q],guy_default_names[q]));
			//al_trace("Map Flag ID %d is: %s\n", q, moduledata.guy_type_names[q]);
		}
		
		const char enemy_weapon_cats[wMax-wEnemyWeapons][255]=
		{
			"ewNone",
			"ewFireball",
			"ewArrow",
			"ewBrang",
			"ewSword",
			"ewRock",
			"ewMagic",
			"ewBomb",
			"ewSBomb",
			"ewLitBomb",
			"ewLitSBomb",
			"ewFireTrail",
			"ewFlame",
			"ewWind",
			"ewFlame2",
			"ewFlame2Trail",
			"ewIce",
			"ewFireball2"
		};
		
		const char enemy_weapon_default_names[wMax-wEnemyWeapons][255]=
		{
			"(None)",
			"Fireball",
			"Arrow",
			"Boomerang",
			"Sword",
			"Rock",
			"Magic",
			"Bomb Blast",
			"Super Bomb Blast",
			"Lit Bomb",
			"Lit Super Bomb",
			"Fire Trail",
			"Flame",
			"Wind",
			"Flame 2",
			"-Flame 2 Trail <unused>",
			"-Ice <unused>",
			"Fireball (Rising)"
		};
		
		for ( int32_t q = 0; q < sizeof(enemy_weapon_default_names)/255; q++ )
		{
			strcpy(moduledata.enemy_weapon_names[q],zc_get_config("EWEAPONS",enemy_weapon_cats[q],enemy_weapon_default_names[q]));
			//al_trace("EWeapon ID %d is: %s\n", q, moduledata.enemy_weapon_names[q]);
		}
		
		
		strcpy(moduledata.enemy_scriptweaponweapon_names[0],zc_get_config("EWEAPONS","Custom_1","Custom 01"));
		strcpy(moduledata.enemy_scriptweaponweapon_names[1],zc_get_config("EWEAPONS","Custom_2","Custom 02"));
		strcpy(moduledata.enemy_scriptweaponweapon_names[2],zc_get_config("EWEAPONS","Custom_3","Custom 03"));
		strcpy(moduledata.enemy_scriptweaponweapon_names[3],zc_get_config("EWEAPONS","Custom_4","Custom 04"));
		strcpy(moduledata.enemy_scriptweaponweapon_names[4],zc_get_config("EWEAPONS","Custom_5","Custom 05"));
		strcpy(moduledata.enemy_scriptweaponweapon_names[5],zc_get_config("EWEAPONS","Custom_6","Custom 06"));
		strcpy(moduledata.enemy_scriptweaponweapon_names[6],zc_get_config("EWEAPONS","Custom_7","Custom 07"));
		strcpy(moduledata.enemy_scriptweaponweapon_names[7],zc_get_config("EWEAPONS","Custom_8","Custom 08"));
		strcpy(moduledata.enemy_scriptweaponweapon_names[8],zc_get_config("EWEAPONS","Custom_9","Custom 09"));
		strcpy(moduledata.enemy_scriptweaponweapon_names[9],zc_get_config("EWEAPONS","Custom_10","Custom 10"));
		
		const char lweapon_cats[wIce+1][255]=
		{
			"lwNone","lwSword","lwBeam","lwBrang","lwBomb","lwSBomb","lwLitBomb",
			"lwLitSBomb","lwArrow","lwFire","lwWhistle","lwMeat","lwWand","lwMagic","lwCatching",
			"lwWind","lwRefMagic","lwRefFireball","lwRefRock", "lwHammer","lwGrapple", "lwHSHandle", 
			"lwHSChain", "lwSSparkle","lwFSparkle", "lwSmack", "lwPhantom", 
			"lwCane","lwRefBeam", "lwStomp","","lwScript1", "lwScript2", "lwScript3", 
			"lwScript4","lwScript5", "lwScript6", "lwScript7", "lwScript8","lwScript9", "lwScript10", "lwIce"
		};
		const char lweapon_default_names[wIce+1][255]=
		{
			"(None)","Sword","Sword Beam","Boomerang","Bomb","Super Bomb","Lit Bomb",
			"Lit Super Bomb","Arrow","Fire","Whistle","Bait","Wand","Magic","-Catching",
			"Wind","Reflected Magic","Reflected Fireball","Reflected Rock", "Hammer","Hookshot", "-HSHandle", 
			"-HSChain", "Sparkle","-FSparkle", "-Smack", "-Phantom", 
			"Cane of Byrna","Reflected Sword Beam", "-Stomp","-lwmax","Script1", "Script2", "Script3", 
			"Script4","Script5", "Script6", "Script7", "Script8","Script9", "Script10", "Ice"
		};
		for ( int32_t q = 0; q < wIce+1; q++ )
		{
			strcpy(moduledata.player_weapon_names[q],(lweapon_cats[q][0] ? zc_get_config("LWEAPONS",lweapon_cats[q],lweapon_default_names[q]) : lweapon_default_names[q]));
			//al_trace("LWeapon ID %d is: %s\n", q, moduledata.player_weapon_names[q]);
		}
		const char counter_cats[33][255]=
		{
			"crNONE","crLIFE","crMONEY","crBOMBS","crARROWS","crMAGIC","crKEYS",
			"crSBOMBS","crCUSTOM1","crCUSTOM2","crCUSTOM3","crCUSTOM4","crCUSTOM5","crCUSTOM6",
			"crCUSTOM7","crCUSTOM8","crCUSTOM9","crCUSTOM10","crCUSTOM11","crCUSTOM12","crCUSTOM13",
			"crCUSTOM14","crCUSTOM15","crCUSTOM16","crCUSTOM17","crCUSTOM18","crCUSTOM19",
			"crCUSTOM20","crCUSTOM21","crCUSTOM22","crCUSTOM23","crCUSTOM24","crCUSTOM25"
		};

		for ( int32_t q = 0; q < 20; q++ )
		{
			for ( int32_t w = 0; w < 4; w++ )
				strcpy(moduledata.combotypeCustomAttributes[q][w],zc_get_config("CUSTOMCOMBOTYPES",CustomComboAttributeTypes[q][w],defaultCustomComboAttributes[q][w]));
		}
		for ( int32_t q = 0; q < 20; q++ )
		{
			for ( int32_t w = 0; w < 8; w++ )
				strcpy(moduledata.combotypeCustomAttribytes[q][w],zc_get_config("CUSTOMCOMBOTYPES",CustomComboAttribyteTypes[q][w],defaultCustomComboAttribytes[q][w]));
		}
		for ( int32_t q = 0; q < 20; q++ )
		{
			for ( int32_t w = 0; w < 8; w++ )
				strcpy(moduledata.combotypeCustomAttrishorts[q][w],zc_get_config("CUSTOMCOMBOTYPES",CustomComboAttrishortTypes[q][w],defaultCustomComboAttrishorts[q][w]));
		}
		for ( int32_t q = 0; q < 20; q++ )
		{
			for ( int32_t e = 0; e < 16; e++ )
				strcpy(moduledata.combotypeCustomFlags[q][e],zc_get_config("CUSTOMCOMBOFLAGS",CustomComboAttributeFlags[q][e],defaultCustomComboFlags[q][e]));
		
		}
		
		const char counter_default_names[33][255]=
		{
			"None","Life","Rupees", "Bombs","Arrows","Magic",
			"Keys","Super Bombs","Custom 1","Custom 2","Custom 3",
			"Custom 4","Custom 5","Custom 6","Custom 7","Custom 8",
			"Custom 9","Custom 10","Custom 11","Custom 12",
			"Custom 13","Custom 14","Custom 15","Custom 16","Custom 17",
			"Custom 18","Custom 19","Custom 20","Custom 21","Custom 22"
			"Custom 23","Custom 24","Custom 25"	
		};
		for ( int32_t q = 0; q < 33; q++ )
		{
			strcpy(moduledata.counter_names[q],zc_get_config("COUNTERS",counter_cats[q],counter_default_names[q]));
			//al_trace("Counter ID %d is: %s\n", q, moduledata.counter_names[q]);
		}
		
		al_trace("Module Title: %s\n", moduledata.moduletitle);
		al_trace("Module Author: %s\n", moduledata.moduleauthor);
		al_trace("Module Info: \n%s\n%s\n%s\n%s\n%s\n", moduledata.moduleinfo0, moduledata.moduleinfo1, moduledata.moduleinfo2, moduledata.moduleinfo3, moduledata.moduleinfo4);
		//al_trace("Module Base NSF: %s\n", moduledata.module_base_nsf);
		
		al_trace("Module Version: %d.%d.%d.%d\n", moduledata.modver_1,moduledata.modver_2,moduledata.modver_3, moduledata.modver_4);
		al_trace("Module Build: %d, %s: %d\n", moduledata.modbuild, (moduledata.modbeta<0) ? "Alpha" : "Beta", moduledata.modbeta );
		
		//al_trace("Build Day: %s\n",dayextension(moduledata.modday).c_str());
		//al_trace("Build Month: %s\n",(char*)months[moduledata.modmonth]);
		//al_trace("Build Year: %d\n",moduledata.modyear);
		al_trace("Build Date: %s %s, %d at @ %d:%d %s\n", dayextension(moduledata.modday).c_str(), 
			(char*)months[moduledata.modmonth], moduledata.modyear, moduledata.modhour, moduledata.modminute, moduledata.moduletimezone);
	}
	
	//shift back to the normal config file, when done
	set_config_standard();
	return true;
}
#endif

