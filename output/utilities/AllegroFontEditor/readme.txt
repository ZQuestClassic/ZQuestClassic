
      _____   ___       ___       ________    ______   ________    ______
     /###" \ |##"\     |##"|     |####"   \  /###"  \ |####"   \  /####" \
    /#" _   \|#" |     |#" |     |##  ____| /#" __   \|##  _    |/#" _    \
   |#" | \   |#  |     |#  |     |#" |__   |#" /  \__/|#" | \"  |#" / \"  |
   |#  |_|   |#  |     |#  |     |#     \  |#  |______|#  |_/"  |#  | |   |
   |#   _    |#  |     |#  |     |#   __|  |#  |      |#       /|#  | |   |
   |"  | |   |"  |_____|"  |_____|"  |_____|"  \\_|   |"  |\#  \|"  \_/"  |
   |   | |   |         \         \         \    "     |   | \"  \\        /
   |___| |___|_________|_________|_________|\_______/_|___|__\___\\______/
                   |####"   \ /####" \|###"\   |##"|####"        \
                   |##  ____|/#" _    |##   \  |#" \____     ____|
                   |#" |__  |#" / \"  |#"    \ |#  |    |#" |
                   |#     \ |#  | |   |#  \#  \|#  |    |#  |
                   |#   __| |#  | |   |#  |\#  \"  |    |#  |
                   |"  |    |"  \_/"  |"  | \#     |    |"  |
                   |   |     \        |   |  \"    |    |   |
            _______|___/_____ \______/|___/___\____/____|___/_______
           |####"   \|####"  \ |###"\/####"       \/####" \|####"   \
           |##  ____||##  _   \|##  |____     ____/#" _    |##  _    |
           |#" |__   |#" | \   |#"  |    |#" |   |#" / \"  |#" | \"  |
           |#     \  |#  |  |  |#   |    |#  |   |#  | |   |#  |_/"  |
           |#   __|  |#  |  |  |#   |    |#  |   |#  | |   |#       /
           |"  |_____|"  |__|  |"   |    |"  |   |"  \_/"  |"  |\#  \
           |         \         /    |    |   |    \        |   | \"  \
           |_________|________/|____|    |___/     \______/|___|  \___\


                      ------------------------------------
                        Allegro Font Editor version 1.19b
                      ------------------------------------

                         Copyright (c) 2005 Miran Amon
                                 August 25, 2005





______________________________________________________________________________
\________________________________INTRODUCTION_________________________________\


    Allegro Font Editor is a simple bitmap editor specialized for editing
    bitmap fonts used in computer video games. It can import fonts from many
    popular font formats and save files ready to be imported by the Grabber
    utility into an Allegro datafile. It provides all the basic bitmap edit-
    ing functions and a few effect filters.

    This package contains a MS Windows executable and full source code. See
    the 'compiling' section of this document for instructions on compiling
    the program for different platforms.

    Allegro Font Editor is free which means you can copy, upload and distri-
    bute it as much as you like. The only limitation is that you don't charge
    any money for it and you keep all the files in the distribution package
    intact. If you wish to modify the source or use it or a part of it in
    your own project then plese let me know by sending me an e-mail (see
    contact information at the end of this document).


______________________________________________________________________________
\_________________________________FILE_LIST___________________________________\


    This package should contain these files:

        FontEd.exe      - MS Windows executable
        FontEd.cfg      - program configuration file
        info.txt        - short description of the package
        readme.txt      - this document
        src.zip         - the source code
        skins\*.*       - various skin specific files


______________________________________________________________________________
\_________________________________INSTALLING__________________________________\


    To install the Allegro Font Editor simply extract the contents of this
    package to a new directory. Optionally you can move the alleg40.dll file
    to your windows\system32 directory. To run the program run FontEd.exe
    directly or create a desktop shortcut to it. This version should run on
    all versions of Windows from Windows 95 to WindowsXP.


______________________________________________________________________________
\_______________________________CONFIGURATION_________________________________\


    Allegro Font Editor should run without problems out of the box. If how-
    ever you do experience problems or just wish to change the configuration
    of the program you can do so by editing the 'FontEd.cfg' file. This is a
    normal text file containing various settings for the program:

    skinPath = 'path'  - full or relative path to the MASkinG skin you want
                         the program to use; leave empty if you want the
                         default skin
    screenWidth = 640  - the screen resolution or window size; you shouldn't
    screenHeight = 480   change this as the program was written specifically
                         for the 640x480 resolution
    colorDepth = 16    - the color depth of the screen; if you run the editor
                         in a window this should match your desktop color
                         depth; possible values are 15, 16, 24 and 32
    refreshRate = 60   - the refresh rate of your monitor in Hz; if the
                         frequency can't be set the default value of 60 Hz will
                         be used; note: this has no meaning in windowed mode
    gfxMode = 0        - the graphics mode the program will try to use; DXAC
                         selects DirectX fullscreen mode, DXWN selects windowed
                         mode; see the Allegro documentation for the full list
                         of all supported modes; change this only if the
                         program is having problems setting the gfx mode
    fullscreen = 0     - should the program run in a window or fullscreen?

    There may be other settings but aren'r very important of have no effect
    at all on this program.


______________________________________________________________________________
\__________________________________COMPILING__________________________________\


    The entire source code for the program is included in the package. If you
    want to compile it yourself though you will need a few libraries. First
    of all you will need Allegro. This version was successfully compiled
    with Allegro 4.0.3 and 4.2.0 beta 3. I haven't tested any other version.
    Secondly you will need MASkinG which is an Allegro add-on library I wrote
    for making graphical user interfaces with Allegro. MASkinG also requires
    another library called AllegroFont for loading some formats of fonts. You
    can get these libraries from these sites:

        - Allegro:      http://alleg.sourceforge.net/
        - AllegroFont:  http://nekros.freeshell.org/delirium/alfont.php
        - MASkinG:      http://ferisrv5.uni-mb.si/~ma0747

    After you've downloaded and installed Allegro, AllegroFont and MASkinG (in
    that order) you can compile the program itself. I've included makefiles
    for MinGW and Linux. If you have problems compiling just edit the makefile
    to match your system configuration.

    Note: It is possible that you will have to get the latest WIP of MASkinG
    off SourceForge CVS!


______________________________________________________________________________
\________________________________MAKING_FONTS_________________________________\


    To make fonts and use them in an Allegro game you first need to know what
    exactly a font is in Allegro. Allegro can only work with so called bitmap
    fonts. Bitmap fonts are nothing but 8 bpp bitmaps containing images of the
    characters (also called glyphs) arranged in a certain predefined way.

    The thing that makes fonts different from other bitmaps is the way they are
    organized. Open a bitmap font in your favourite bitmap editor and look at
    the palette. You will see that color #255 (i.e. the last color) is used for
    the background (usually this color is bright yellow although this is not
    really required) and on this background is a grid of boxes each containing
    one character. If you look closely at one of the little boxes you see that
    color #0 (i.e. the first color) is used for the background of the character
    (usually this color is magenta or bright pink but again this isn't nece-
    ssary) and other colors are used for the character itself.

    In some fonts only one color is used for the characters. These fonts are
    called monochrome fonts. You can make Allegro color the text printed with
    a monochrome font in any color you want like this:

        textout(screen, monoFont, "Hello, world!", x, y, fontColor);

    The other type of fonts are so called colored fonts. In these fonts the
    individual characters are painted with more than 1 color. If you print
    text in Allegro with the above line the character won't look like they
    were originally painted but will be colored with the 'fontColor' passed
    to the function. If you want Allegro to use the 'real' colors in a colored
    font you have to pass -1 for the color argument of the textout functions:

        textout(screen, colorFont, "Hello, world!", x, y, -1);

    Another thing you should know about fonts is the difference between fixed
    width and variable width fonts. In a fixed width font all the characters
    have the same width (and height), for example the default Allegro font has
    characters that are all 8x8 pixels big. In variable width fonts on the
    other hand each character can have a different width (height still has to
    be the same).


______________________________________________________________________________
\_________________________USING_ALLEGRO_FONT_EDITOR___________________________\


    The way you use the Allegro Font Editor program depends on what kind of
    you would like to make. Usually you will either create a new font form
    scratch and hand edit each inidividual character or you will convert an
    existing font to the bitmap font format Allegro can use. Either way your
    final product will be a bitmap (usually in PCX format) that you can import
    into a datafile by the Allegro Grabber tool. This step is necessary becau-
    se Allegro doesn't yet have a function that loads a bitmap font from a
    file on disk so you must use the datafile system.

    The program window is separated in roughly 5 different areas: the charac-
    ter and character range selection lists, the character editor, the palette
    panel, the preview panel and the menu.

    The character selection list:
    ------------------------------
    This is a list containing all characters in the currently selected range.
    By default only the 7 bit ASCII range of characters is enabled (characters
    with ASCII codes between 32 and 127). You can add any UNICODE or extended
    ASCII range by selecting "Edit/Add Range" in the menu.

    The character editor:
    ----------------------
    The editor displays the currently selected character as a grid of pixels
    zoomed in as much as possible. If you wish you can turn the gridlines on
    and off with the checkbox just right of the editor. If the background of
    the characters is palette color #0 (bright pink) it is painted as gray.
    Drawing is done with the mouse. The left mouse button paints the primary
    color (see below) while the right mouse button paints with the secondary
    color. If your mouse has a third button you can use it to clear the pixels
    to the background color otherwise just use the secondary color for painting
    color #0. By default, clicking the mouse will paint single pixels but if
    you press and hold the shift key while clicking you can also draw lines.
    To draw a line between points A and B just click on point A, press and hold
    the shift key and click on point B. By holding the control key on the
    keyboard the floodfill tool is activated. The floodfill tool will fill the
    entire area where you click with the selected color.

    The palette panel:
    -------------------
    This tool is used for selecting the primary and secondary colors and for
    changing the palette. To select your primary color click on it with the
    left mouse button and for selecting the secondary color click with the
    right mouse button. To edit a colour double click it with either the left
    or the right button. A little red rectangle is drawn around the selected
    primary color and a blue rectangle is drawn around the secondary color.
    You cannot select color #255 as this is reserved for the final font bitmap
    and you cannot edit colors #255 and #0 (color #0 is always bright pink).

    The preview panel:
    -------------------
    Use the preview panel to see how the font will look like when used to
    printout actual text. You can edit the text you want to use for the preview
    in the text edit box just above the panel. By default the preview will be
    updated whenever the font is edited but you can disable this behaviour on
    slow computers by deselecting the checkbox just above the palette panel.
    In that mode you can manually force the preview to update by pressing the
    enter key on the keyboard. Note: the preview panel currently works only
    with ASCII characters. If you enter any other character the preview may
    display wrong text and the program may even crash.

    The menu:
    ----------
    The menu is used to access most functions of the editor. Each menu item
    may also have a keayboard shortcut associated with it. This is printed
    on the right side of each menu item. The menu is divided in these
    sections: file, edit, glyph, palette, effects, help.

    file/new:
    Starts a new font and discards the old one. You have to enter the size of
    the characters in pixels. The default is 8x8. The secondary color is used
    for the background of the font so you may want to select your preferred
    color in the pallete panel before chosing this function.

    file/open:
    Allows you to browse the local filesystem to select a font you want to
    import. You can import any font the Grabber tool can (i.e. bitmap fonts,
    BIOS fonts and GRX fonts) and lots of other fonts such as True Type Fonts
    and Type 1 fonts. Again the secondary color will be used as the background
    color. This is especially important if you want to import a True Type font
    or a Type 1 font using antialiasing. In this case the background color will
    be used to do the antialiasing effect. If the backround color is color #0
    (bright pink) the antialiasing will not work.
    Note: be carefull when loading bitmap fonts. If the file you're opening is
    a bitmap but is not a valid font the program has no way of knowing that and
    will probably die a slow and agonizing death. The same may apply for one or
    two other supported formats.

    file/save:
    Quick-saves the selected font. If you haven't saved the font yet it will
    popup the Save As dialog.

    file/save as:
    Opens a save as dialog which allows you to browse the local file system to
    select the path and the filename for your font. You can save in any bitmap
    format Allegro natively supports (PCX, BMP, TGA and LBM). If the font
    contains more than one range a script with the same name as the name you
    choose but with the .txt extension will also be saved. You can then load
    this script with the Grabber tool.

    file/quit:
    Exits the program.

    edit/cut:
    edit/copy:
    edit/paste:
    Use these functions for manipulating the characters in the same way as in
    most GUI programs.

    edit/add range
    Adds another range of characters to the font. You must enter the codes
    of the first and last characters in decimal format.

    edit/canvas size
    Changes the canvas size of all the characters in the font. You may enter
    the sizes of the left, right, top and bottom borders. The sizes are in
    pixels and may be positive or negative. Positive values grow the glyphs
    while negative values shrink them.

    glyph/clear:
    Deletes the contents of the currently selected glyph.

    glyph/next:
    glyph/previous:
    Use these to select the next or the previous glyph in the character list.

    glyph/resize:
    Opens a small dialog that allows you to resize the currently selected
    character. You may enter both the new width and height.

    glyph/canvas size:
    The same as edit/canvas size except that this function changes only the
    currently selected character.

    glyph/push/up:
    glyph/push/down:
    glyph/push/left:
    glyph/push/right:
    Use this to change the position of the character in the character box by
    one pixel in each direction.

    glyph/import:
    glyph/export:
    Use these command to import and export individual glyphs to and from
    the font. You can for example edit the glyphs with your favourite
    bitmap editor and then import them into the font.

    palette/default palette:
    Generates the default palette.

    palette/load:
    Allows you grab the palette from a bitmap.

    palette/gradient:
    Generates a gradient in the palette between the colors that are selected
    as primary and secondary. The program will ask you to select two colors
    and interpolate the palette entries between primary and secondary to
    create a nice looking gradient.

    effects/gradient:
    Fills the characters with a gradient between the primary and secondary
    colors. To use this tool first select both colors in the palette panel
    then select this function. Usually you will need to use the
    palette/gradient tool to define a gradient before using this effect.

    effects/outline:
    Draws a one pixel outline with the primary color around all characters.
    To use this effect first select the primary color in the palette panel
    then select this function.

    effects/smooth:
    effects/blur:
    These effects apply a simple filter which blurs the characters a bit.
    They both use the secondary color instead of bright pink for transparent
    pixels. If the secondary color is also bright pink then transparent
    pixels are ignored. As these two effects apply a 3x3 convolution matrix
    to the glyph data, every character needs to be resized by 1 pixel in
    each direction. If this is not what you want you may manually change
    the sizes of the glyphs back by selecting edit/canvas size and entering
    -1 in all input fields.

    effects/add noise:
    Adds a little random noise to the characters. You have to select the level
    of noise you want in the range between 0 and 255.

    effects/cutout:
    This is basically the opposite effect from outline. It leaves the outline
    and changes the insides of the characters to the primary color.

    effects/make wave:
    Separates each character in two parts with a sine wave pattern filling
    the top part with the primary colour and the bottom part with the secon-
    dary colour.


______________________________________________________________________________
\_______________________________KNOWN_PROBLEMS________________________________\


    The program still has a few bugs that I know of and some commonly used
    functions/effects aren't implemented yet:

       - the user interface is not very user friendly as the program doesn't
         yet have an undo function,
       - some of the effects don't yet work the way they're supposed,
       - loading some fonts doesn't work and may even crash or freeze the
         program; also when loading ttf fonts some characters may be too
         big or too small,
       - after loading some variable size fonts (i.e. True Type fonts, Type 1
         fonts, Windows fonts, etc.) the program may crash on exit,
       - the preview panel doesn't work with UNICODE fonts and may crash the
         program if not used properly,
       - some elements of the GUI have some small issues: the character list
         sometimes can't be scrolled all the way (arrow keys work though),
         the menus sometimes draw over one another and the file selector
         doesn't work properly all the time

    If you find any bugs or other problems with the program don't hesitate to
    contact me (see below for contact information).


______________________________________________________________________________
\___________________________________CREDITS___________________________________\


    I would like to say thank you to the following people who made this
    program possible:

        - Shawn Hargreaves and everybody else who made or helped make the
          Allegro programming library,

        - David Turner, Robert Wilhelm and Werner Lemberg for making the
          FreeType project (www.freetype.org),

        - Javier Gonzalez for making the AlegroFont library,

        - evryone else who offered assistance and support either by
          reporting bugs, suggesting program features, providing solutions
          to problems or in any other way helping develop this program.


______________________________________________________________________________
\________________________________CONTACT_INFO_________________________________\


    You can get the latest version of this program at my homepage:

        http://ferisrv5.uni-mb.si/~ma0747

    If you want to contact me for any reason you can reach me at this address:

        miran.amon@gmail.com


______________________________________________________________________________
\_____________________________________EOF_____________________________________\
