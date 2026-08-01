// `@deprecated_getter` gives a variable an older function form, which must stay
// callable. Both shapes are covered here: classes that use a ref variable
// (`@zasm_ref`, like `npc`), where the object pointer is pushed and popped, and
// classes that don't (like `Game`), where it is skipped entirely.

#include "std.zh"

void refvar_getters(npc n, lweapon lw, eweapon ew, itemsprite it, combodata cd, messagedata md)
{
	printf("%d %d %d %d\n", n->Max(), lw->Max(), ew->Max(), it->Max());
	printf("%d %d %d %d\n", cd->PosX(), cd->PosY(), cd->Layer(), md->TextWidth());
}

void plain_getters()
{
	printf("%d %d %d\n", Game->GetCurScreen(), Game->GetCurDMap(), Game->GetCurLevel());
	printf("%d %d %d\n", Graphics->NumDraws(), Graphics->MaxDraws(), Screen->NumItems());
}

global script Global
{
	void run()
	{
		refvar_getters(NULL, NULL, NULL, NULL, NULL, NULL);
		plain_getters();
	}
}
