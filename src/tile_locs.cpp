//2.54 expanded tile pages
//This is going to be fun, as absolutely every reference to tiles that expects a short needs to become a long. :( -Z
#define TILE_PAGES_254      214747/(TILES_PER_ROW*TILE_ROWS_PER_PAGE)
#define NEWMAXTILES254      214747 - ( 214747 % (TILES_PER_PAGE*TILE_PAGES) )     // 32760 tiles
#define NEWTILE_SIZE254       (NEWMAXTILES254*SINGLE_TILE_SIZE)  // 4193280 bytes (new packed format, 6 pages)



#define SINGLE_TILE_SIZE    128
#define TILES_PER_ROW       20
#define TILE_ROWS_PER_PAGE  13
#define TILES_PER_PAGE      (TILES_PER_ROW*TILE_ROWS_PER_PAGE)

/*
#define TILE_PAGES          252

#define OLDMAXTILES         (TILES_PER_PAGE*6)              // 1560 tiles
#define NEWMAXTILES         (TILES_PER_PAGE*TILE_PAGES)     // 32760 tiles
*/

#define TILE_PAGES          825
#define TILE_PAGES_ZC250    252 //2.50.x

#define OLDMAXTILES         (TILES_PER_PAGE*6)              // 1560 tiles
#define NEWMAXTILES         (TILES_PER_PAGE*TILE_PAGES)     // 214500 tiles
#define ZC250MAXTILES         (TILES_PER_PAGE*TILE_PAGES_ZC250)     // 32760 tiles

/*
#define NEWTILE_SIZE2       (NEWMAXTILES*SINGLE_TILE_SIZE)  // 4193280 bytes (new packed format, 6 pages)
*/

#define NEWTILE_SIZE2       (NEWMAXTILES*SINGLE_TILE_SIZE)  // 27456000 bytes 
#define ZC250TILESIZE       (ZC250MAXTILES*SINGLE_TILE_SIZE)  // 4193280 bytes (new packed format, 6 pages)


#define OLDTILE_SIZE2       (OLDMAXTILES*SINGLE_TILE_SIZE)  // 199680 bytes (new packed format, 6 pages)
// 133120 bytes (new packed format, 4 pages)
#define OLDTILE_SIZE        (TILES_PER_PAGE*4*SINGLE_TILE_SIZE)
//#define NEWTILE_SIZE      (260*6*128) // 199680 bytes (new packed format, 6 pages)
//#define OLDTILE_SIZE      (260*4*128) // 133120 bytes (new packed format, 4 pages)
#define TILEBUF_SIZE        (320*480)                       // 153600 bytes (old unpacked format)

//All references to these needs one case if V_TILES < 2, and a new
//case for tiles being a long (V_TILES >= 2)
struct itemdata
{
    word tile;
	
struct wpndata
{
    word tile;
	
struct guydata
{
    word  tile;
	word  s_tile; //secondary (additional) tile(s)
	word  e_tile;
	
struct newcombo
{
    word tile; //16 bits
	
struct MsgStr
{
    word tile;
	
struct dmap
{
    
    word minimap_1_tile;        
word minimap_2_tile;                                      //after getting map
	//before getting map
	word largemap_1_tile;                                     //large map
	word largemap_2_tile;                                     //large map
	
struct pondtype
{
    byte fairytile;
	
struct zcolors
{
	 word triforce_tile;
    word triframe_tile;
    word overworld_map_tile;
    word dungeon_map_tile;
    word blueframe_tile;
    //30
    word HCpieces_tile;
	