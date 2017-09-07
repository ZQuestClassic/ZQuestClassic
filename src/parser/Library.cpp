#include "../precompiled.h"
#include "Library.h"

#include <cassert>
#include "Types.h"

using namespace ZScript;

Library const& Libraries::get(DataTypeClass const& type)
{
	if (type == DataType::GAME)
		return Libraries::Game::singleton();
	if (type == DataType::DEBUG)
		return Libraries::Debug::singleton();
	if (type == DataType::SCREEN)
		return Libraries::Screen::singleton();
	if (type == DataType::AUDIO)
		return Libraries::Audio::singleton();
	if (type == DataType::_LINK)
		return Libraries::Link::singleton();
	if (type == DataType::ITEMCLASS)
		return Libraries::ItemClass::singleton();
	if (type == DataType::ITEM)
		return Libraries::Item::singleton();
	if (type == DataType::NPCDATA)
		return Libraries::NpcClass::singleton();
	if (type == DataType::NPC)
		return Libraries::Npc::singleton();
	if (type == DataType::FFC)
		return Libraries::Ffc::singleton();
	if (type == DataType::LWPN)
		return Libraries::LWeapon::singleton();
	if (type == DataType::EWPN)
		return Libraries::EWeapon::singleton();

	assert(!"Unrecognized class type");
}
