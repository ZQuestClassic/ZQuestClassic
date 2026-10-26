// Internal untyped arrays whose storage can't hold an object pointer (`@no_objects`,
// like mapdata->D[]) reject storing objects.

class NoObjectsFoo {}

generic script errors_no_objects
{
	void run()
	{
		mapdata md = Game->LoadMapData(1, 0);
		md->D[0] = new NoObjectsFoo(); // Error.
		md->D[1] = 5; // OK.
		untyped u = new NoObjectsFoo();
		md->D[2] = u; // OK: an untyped value is not retained, so isn't an object here.
	}
}
