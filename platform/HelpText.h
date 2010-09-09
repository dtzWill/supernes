/*
 * ===========================================================================
 *
 *       Filename:  HelpText.h
 *
 *    Description:  Text for the help screens...
 *                  Meant to be #include'd only in OptionMenu
 *
 *        Version:  1.0
 *        Created:  08/17/2010 02:10:01 AM
 *
 *         Author:  Will Dietz (WD), w@wdtz.org
 *        Company:  dtzTech
 *
 * ===========================================================================
 */


typedef struct
{
  char * msg;
  SDL_Color color;
} line;

// "14" lines is what it takes
// to fill the screen nicely at current resolution
// and current font size.

line helpROMs[][14] =
{
{
    {"Welcome to SuperNES for WebOS!",              textColor },
    {" ",                                           textColor },
    {"SuperNES is a Super Nintendo Entertainment",  textColor },
    {"System (SNES) emulator.",                     textColor },
    {" ",                                           textColor },
    {"What that means is SuperNES allows you",      textColor },
    {"to play games made for the SNES.",            textColor },
    {"However, much like a real SNES needs",        textColor },
    {"separate games to play, SuperNES needs",      textColor },
    {"game too.  These games are generally",        textColor },
    {"called 'ROM's, which are computer copies",    textColor },
    {"of games for the SNES console.",              textColor },
    {" ",                                           textColor },
    {"(Tap to go to next screen)",                  linkColor }
},
{
    {"Where do I get ROMs?",                        textColor },
    {" ",                                           textColor },
    {"There are many great ROMs freely available",  textColor },
    {"all over the internet.  Look for 'Public",    textColor },
    {"Domain' ROMs and you should find many.",      textColor },
    {"SNES can also play many commercial games",    textColor },
    {"made for the Super Nintendo.",                textColor },
    {"We don't cover how to dump these games",      textColor },
    {"from your originals, and we remind you",      textColor },
    {"to observe all relevant laws that might",     textColor },
    {"apply before doing so.",                      textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {"(Tap to go to next screen)",                  linkColor }
},
{
    {"Okay I got the ROMs, what now?",              textColor },
    {" ",                                           textColor },
    {"To play your ROMs, connect your device",      textColor },
    {"to your computer and put it in USB mode.",    textColor },
    {"You'll want to put your ROMs in",             textColor },
    {"a folder called",                             textColor },
    {"/snes/roms",                                  hiColor   },
    {"which you might have to create.",             textColor },
    {"First create a 'snes' folder then create",    textColor },
    {"a 'roms' folder inside of that.",             textColor },
    {"Watch the capitalization, all lower case.",   textColor },
    {"Once you have the ROMs there, restart",       textColor },
    {"SuperNES and then just tap them to play.",    textColor },
    {"(Tap to go to next screen)",                  linkColor }
},
{
    {"My ROMs aren't showing up?!",                 textColor },
    {" ",                                           textColor },
    {"If you're having trouble getting your ROMS",  textColor },
    {"to appear in the ROM selector, check:",       textColor },
    {"-All files end in either \"smc\" or \"zip\"", textColor },
    {"-Zip files contain exactly 1 ROM",            textColor },
    {"-Try restarting the device",                  textColor },
    {"-Make sure the folders are 'snes/roms'",      textColor },
    {" ",                                           textColor },
    {"If you still aren't seeing your ROMs feel ",  textColor },
    {"free to contact me for support:",             textColor },
    {"webos@wdtz.org",                              hiColor   },
    {" ",                                           textColor },
    {"(Tap to return to help)",                     linkColor }
}

};

line helpControls[][14] =
{
{
    {"How do I play?",                              textColor },
    {" ",                                           textColor },
    {"SuperNES has two ways of playing:",           textColor },
    {"Physical/keyboard or touchscreen skins.",     textColor },
    {"You can use physical keyboard at anytime,",   textColor },
    {"but you have to pick and enable a skin to",   textColor },
    {"make use of the touchscreen.",                textColor },
    {" ",                                           textColor },
    {"To change your skin settings, use the",       textColor },
    {"'Skins' menu item.",                          textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {"(Tap to go to next screen)",                  linkColor }
},
{
    {"Physical buttons -- Defaults",                textColor },
    {" ",                                           textColor },
    {"Default keyboard bindings:",                  textColor },
    {"E/S/W/D -- Up/Down/Left/Right (d-pad)",       textColor },
    {"K/L/M/comma -- Y/X/B/A (action buttons)",     textColor },
    {"I/O -- L/R (shoulder trigger buttons)",       textColor },
    {"Backspace -- Start",                          textColor },
    {"Enter -- Select",                             textColor },
    {"Space -- Toggle turbo",                       textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {"(Tap to go to next screen)",                  linkColor }
},
{
    {"Physical buttons -- Configuration",           textColor },
    {" ",                                           textColor },
    {"If you don't like the defaults,",             textColor },
    {"you're in luck -- you can easily change",     textColor },
    {"them to your liking.",                        textColor },
    {"To start the process, just press",            textColor },
    {"either the '=' or '?' keys",                  textColor },
    {"while in-game and follow the ",               textColor },
    {"on-screen instructions.",                     textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {"(Tap to go to next screen)",                  linkColor }
},
{
    {"Physical buttons -- Extras",                  textColor },
    {" ",                                           textColor },
    {"There are also some non-configurable",        textColor },
    {"'extra' keyboard buttons, which are",         textColor },
    {"listed below:",                               textColor },
    {" ",                                           textColor },
    {"1,2,3 -- Save state number 1,2,3",            textColor },
    {"4,5,6 -- Load state number 1,2,3",            textColor },
    {"Back gesture -- Launch Menu",                 textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {"(Tap to return to help)",                     linkColor }
},
};

line helpSettings[][14] =
{
{
    {"What do the settings mean?",                  textColor },
    {" ",                                           textColor },
    {"SuperNES gives you a number of options to",   textColor },
    {"configure your gaming experience.",           textColor },
    {"Here are brief description of what",          textColor },
    {"the various options mean.",                   textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {"(Tap to go to next screen)",                  linkColor }
},
{
    {"What do the settings mean?",                  textColor },
    {" ",                                           textColor },
    {"Orientation:",                                linkColor },
    {"Show screen in Portrait or Landscape.",       textColor },
    {"Sound:",                                      linkColor },
    {"Determines if sound is enabled or not.",      textColor },
    {"Turning sound off can make games faster.",    textColor },
    {"Filter:"    ,                                 linkColor },
    {"When scaling the screen to fit your device",  textColor },
    {"should we err on smooth and blurry or",       textColor },
    {"sharp and pixelated?",                        textColor },
    {"Stretch",                                     linkColor },
    {"Enable to fullscreen (ignoring aspect ratio)",textColor },
    {"(Tap to go to next screen)",                  linkColor }
},
{
    {"What do the settings mean?",                  textColor },
    {" ",                                           textColor },
    {"Show Speed:",                                 linkColor },
    {"Display emulation speed.",                    textColor },
    {"Autosave:",                                   linkColor },
    {"Autosave saves your game automatically:",     textColor },
    {"periodically and when you pause or exit.",    textColor },
    {"SuperNES will start there next time.",        textColor },
    {"It can make starting/quitting much easier.",  textColor },
    {"Transparency:",                               linkColor },
    {"This is an advanced feature. Having this",    textColor },
    {"on provides more accurate graphics but",      textColor },
    {"can be slower (on by default).",              textColor },
    {"(Tap to return to help)",                     linkColor }
}
};

line helpWiki[14] =
{
    {"SuperNES Wiki",                               textColor },
    {" ",                                           textColor },
    {"Beyond the in-game help, SuperNES also ",     textColor },
    {"has a Wiki page that contains more",          textColor },
    {"explanations, guides, compatibility lists",   textColor },
    {"and more.",                                   textColor },
    {"To view this wiki, just tap the screen.",     textColor },
    {"To exit without going to the wiki just",      textColor },
    {"do a back gesture.",                          textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {"(Back gesture to return to help)",            linkColor },
    {"(Tap to go to launch wiki)",                  linkColor }
};

