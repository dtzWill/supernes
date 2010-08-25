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
    {"Welcome to VisualBoyAdvance (VBA)!",          textColor },
    {" ",                                           textColor },
    {"VBA is a Gameboy, Gameboy Color,",            textColor },
    {"and Gameboy Advance emulator.",               textColor },
    {" ",                                           textColor },
    {"What that means is VBA allows you",           textColor },
    {"to play games made for those systems.",       textColor },
    {"However, much like your gameboy needs",       textColor },
    {"separate games to play, VBA needs games",     textColor },
    {"too.  These games are generally called",      textColor },
    {"'ROM's, which are computer copies of",        textColor },
    {"games for those Gameboy devices.",            textColor },
    {" ",                                           textColor },
    {"(Tap to go to next screen)",                  linkColor }
},
{
    {"Where do I get ROMs?",                        textColor },
    {" ",                                           textColor },
    {"There are many great ROMs freely available",  textColor },
    {"all over the internet.  Examples of such",    textColor },
    {"include \"Anguna\" and \"Another World\".",   textColor },
    {"VBA can also play many commercial games",     textColor },
    {"made for the Gameboy (Color/Advance).",       textColor },
    {"We don't cover how to dump these games",      textColor },
    {"from your originals, and we remind you",      textColor },
    {"to observe all relevant laws that might",     textColor },
    {"apply before doing so.",                      textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {"(Tap to go to next screen)",                linkColor }
},
{
    {"Okay I got the ROMs, what now?",              textColor },
    {" ",                                           textColor },
    {"To play your ROMs, connect your device",      textColor },
    {"to your computer and put it in USB mode.",    textColor },
    {"You'll want to put your ROMs in",             textColor },
    {"a folder called",                             textColor },
    {"/vba/roms",                                   hiColor },
    {"which you might have to create.",             textColor },
    {"First create a 'vba' folder then create",     textColor },
    {"a 'roms' folder inside of that.",             textColor },
    {"Watch the capitalization, all lower case.",   textColor },
    {"Once you have the ROMs there, restart VBA",   textColor },
    {"and then just tap them to play.",             textColor },
    {"(Tap to go to return to help)",               linkColor }
}
};

line helpControls[][14] =
{
{
    {"How do I play?",                              textColor },
    {" ",                                           textColor },
    {"VBA has two ways of playing:",                textColor },
    {"Physical or touchscreen controls.",           textColor },
    {"You can use either at any time, but ",        textColor },
    {"the controller skin only can be drawn in",    textColor },
    {"landscape mode, so keep that in mind.",       textColor },
    {" ",                                           textColor },
    {"To change skins, use the 'Skins' menu. ",     textColor },
    {"For more information, visit the",             textColor },
    {"'Settings' help topic.",                      textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {"(Tap to go to next screen)",                  linkColor }
},
{
    {"Physical buttons",                            textColor },
    {" ",                                           textColor },
    {"Instead of touchscreen controls you",         textColor },
    {"can use the keyboard buttons to play.",       textColor },
    {"There are default buttons, but also ",        textColor },
    {"VBA lets you set your own to suit you",       textColor },
    {"and your needs and gaming style.",            textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {"(Tap to go to next screen)",                  linkColor }
},
{
    {"Physical buttons -- Defaults",                textColor },
    {" ",                                           textColor },
    {"Default keyboard bindings:",                  textColor },
    {"W/A/S/D -- Up/Down/Left/Right (d-pad)",       textColor },
    {"K/L -- B/A (action buttons)",                 textColor },
    {"Q/P -- L/R (shoulder trigger buttons)",       textColor },
    {"Enter -- Start",                              textColor },
    {"Spacebar -- Select",                          textColor },
    {"@ -- Turbo",                                  textColor },
    {". -- Screenshot",                             textColor },
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
    {"t -- Start/Select/A/B together (reset)",      textColor },
    {"- (minus) -- Toggle displaying speed",        textColor },
    {"' (apostrophe) -- Toggle scaling filter",     textColor },
    {"0 -- Toggle orientation",                     textColor },
    {"* (asterisk) -- Toggle sound",                textColor },
    {"1,2,3 -- Save state number 1,2,3",            textColor },
    {"4,5,6 -- Load state number 1,2,3",            textColor },
    {" ",                                           textColor },
    {"(Tap to go to next screen)",                  linkColor }
},
{
    {"Physical buttons -- Extras (Continued)",      textColor },
    {" ",                                           textColor },
    {"+ (plus) -- Toggle displaying skin",          textColor },
    {"& (ampersand) -- Toggle autosave feature",    textColor },
    {"/ (slash) -- Toggle active skin",             textColor },
    {"Back gesture -- Launch menu",                 textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {" ",                                           textColor },
    {"(Tap to go to next screen)",                  linkColor }
}
};

line helpSettings[][14] =
{
{
    {"What do the settings mean?",                  textColor },
    {" ",                                           textColor },
    {"VBA gives you a number of options to",        textColor },
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
    {"Note that skins only work in Landscape.",     textColor },
    {"Sound:",                                      linkColor },
    {"Determines if sound is enabled or not.",      textColor },
    {"Turning sound off can make games faster.",    textColor },
    {"Filter:"    ,                                 linkColor },
    {"When scaling the screen to fit your device",  textColor },
    {"should we make things smooth or sharp?",      textColor },
    {"Smooth looks nicer but can be blurry.",       textColor },
    {"Sharp can look worse, but is more clear.",    textColor },
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
    {"VBA will start there next time.",             textColor },
    {"It can make starting/quitting much easier.",  textColor },
    {"Autoframeskip:",                              linkColor },
    {"Turning this on tells VBA to 'skip' drawing", textColor },
    {"to make things run at 100% speed." ,          textColor },
    {"This can makes games appear 'choppy'.",       textColor },
    {"(Tap to go to next screen)",                  linkColor }
},
{
    {"What do the settings mean?",                  textColor },
    {" ",                                           textColor },
    {"Turbo Toggles:",                              linkColor },
    {"This determines what happens when you",       textColor },
    {"press the turbo button.",                     textColor },
    {"Off (default) makes the game run turbo",      textColor },
    {"while you hold it down. On makes the ",       textColor },
    {"button toggle whether or not turbo is on.",   textColor },
    {"Skins",                                       linkColor },
    {"Pick which visual controller skin is used",   textColor },
    {"if any at all. Try them all and pick",        textColor },
    {"your favorite one.",                          textColor },
    {"Remember skins only work in landscape.",      textColor },
    {"(Tap to go to return to help)",               linkColor }
}
};

line helpWiki[14] =
{
    {"VBA Wiki",                                    textColor },
    {" ",                                           textColor },
    {"Beyond the in-game help, VBA also has a",     textColor },
    {"Wiki page that contains more in-depth",       textColor },
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

