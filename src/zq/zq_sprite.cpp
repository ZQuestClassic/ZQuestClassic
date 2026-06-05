#include "sprite.h"

int32_t fadeclk=-1;
int32_t global_frame=8;
bool BSZ=false;
int32_t conveyclk=0;
byte newconveyorclk=0;
bool freeze_holdup = false, freeze_message = false, freeze_general = false, freeze_ffc = false;

void sprite::handle_sprlighting()
{
	return;
}

bool sprite::on_sideview_solid() const
{
	return false;
}
newcombo const* sprite::check_conveyor()
{
    return nullptr;
}
void movingblock::handle_sprlighting()
{
	return;
}

void movingblock::push(zfix,zfix,int32_t,int32_t)
{
    return;
}

bool movingblock::animate(int32_t)
{
    return false;
}
