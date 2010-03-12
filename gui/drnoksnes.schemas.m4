<?xml version="1.0" encoding="utf-8"?>
define(`choose', `dnl
ifelse(eval(MAEMO_MAJOR < 5), 1, `$1', `$2')dnl>
')dnl
define(`N_', `$*')dnl
<gconfschemafile>
  <schemalist>
    <schema>
      <key>/schemas/apps/maemo/drnoksnes/display-framerate</key>
      <applyto>/apps/maemo/drnoksnes/display-framerate</applyto>
      <owner>drnoksnes</owner>
      <type>bool</type>
      <default>false</default>
      <locale name="C">
        <short>Display framerate</short>
        <long>
          Display a framerate counter in the lower left corner.
        </long>
      </locale>
    </schema>
    <schema>
      <key>/schemas/apps/maemo/drnoksnes/frameskip</key>
      <applyto>/apps/maemo/drnoksnes/frameskip</applyto>
      <owner>drnoksnes</owner>
      <type>int</type>
      <default>0</default>
      <locale name="C">
        <short>Frameskip</short>
        <long>
          Skip this many frames after rendering one frame (or 0 for auto).
        </long>
      </locale>
    </schema>
    <schema>
      <key>/schemas/apps/maemo/drnoksnes/rom</key>
      <applyto>/apps/maemo/drnoksnes/rom</applyto>
      <owner>drnoksnes</owner>
      <type>string</type>
      <default></default>
      <locale name="C">
        <short>ROM to load</short>
        <long>
          Full path to the ROM file to load on next startup.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/drnoksnes/saver</key>
      <applyto>/apps/maemo/drnoksnes/saver</applyto>
      <owner>drnoksnes</owner>
      <type>bool</type>
      <default>true</default>
      <locale name="C">
        <short>Enable power saving</short>
        <long>
          This will save and close the emulator when it is deactivated or
          the device enters idle state.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/drnoksnes/sound</key>
      <applyto>/apps/maemo/drnoksnes/sound</applyto>
      <owner>drnoksnes</owner>
      <type>bool</type>
      <default>true</default>
      <locale name="C">
        <short>Enable sound</short>
        <long>
          Enable emulation and output of sound.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/drnoksnes/scaler</key>
      <applyto>/apps/maemo/drnoksnes/scaler</applyto>
      <owner>drnoksnes</owner>
      <type>string</type>
      <default></default>
      <locale name="C">
        <short>Scaler</short>
        <long>
          Name of the preferred scaler to use. Available scalers depend on 
          platform. Leave empty to select best scaler available.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/drnoksnes/speedhacks</key>
      <applyto>/apps/maemo/drnoksnes/speedhacks</applyto>
      <owner>drnoksnes</owner>
      <type>int</type>
      <default>0</default>
      <locale name="C">
        <short>Speedhacks</short>
        <long>
          Set to 0 to disable speedhacks, to 1 to enable safe speedhacks, 
          to 2 to enable all speedhacks.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/drnoksnes/transparency</key>
      <applyto>/apps/maemo/drnoksnes/transparency</applyto>
      <owner>drnoksnes</owner>
      <type>bool</type>
      ifelse(eval(MAEMO_MAJOR < 5), 1, `<default>false</default>', `<default>true</default>')
      <locale name="C">
        <short>Transparency</short>
        <long>
          Enable emulated transparency effects.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/drnoksnes/turbo</key>
      <applyto>/apps/maemo/drnoksnes/turbo</applyto>
      <owner>drnoksnes</owner>
      <type>bool</type>
      <default>false</default>
      <locale name="C">
        <short>Turbo mode</short>
        <long>
          Do not sleep at all between frames.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/drnoksnes/player1/keyboard/enable</key>
      <applyto>/apps/maemo/drnoksnes/player1/keyboard/enable</applyto>
      <owner>drnoksnes</owner>
      <type>bool</type>
      <default>true</default>
      <locale name="C">
        <short>Player 1 keyboard</short>
        <long>
          Enable key mappings for player 1.
        </long>
      </locale>
    </schema>
dnl Player 1 keybindings
define(`HELP', `')dnl
define(`BUTTON', `dnl
    <schema>
     <key>/schemas/apps/maemo/drnoksnes/player1/keyboard/$2</key>
      <applyto>/apps/maemo/drnoksnes/player1/keyboard/$2</applyto>
      <owner>drnoksnes</owner>
      <type>int</type>
      <default>choose($4,$5)</default>
      <locale name="C">
        <short>$1 button</short>
      </locale>
    </schema>
dnl')dnl
define(`ACTION', `dnl
    <schema>
     <key>/schemas/apps/maemo/drnoksnes/player1/keyboard/$2</key>
      <applyto>/apps/maemo/drnoksnes/player1/keyboard/$2</applyto>
      <owner>drnoksnes</owner>
      <type>int</type>
      <default>choose($4,$5)</default>
      <locale name="C">
        <short>$1 action</short>
      </locale>
    </schema>
dnl')dnl
define(`LAST', `')dnl
include(buttons.inc)dnl
undefine(`HELP')dnl
undefine(`BUTTON')dnl
undefine(`ACTION')dnl
undefine(`LAST')dnl
    <schema>
     <key>/schemas/apps/maemo/drnoksnes/player1/touchscreen/enable</key>
      <applyto>/apps/maemo/drnoksnes/player1/touchscreen/enable</applyto>
      <owner>drnoksnes</owner>
      <type>bool</type>
      <default>false</default>
      <locale name="C">
        <short>Player 1 touchscreen</short>
        <long>
          Enable touchscreen buttons for player 1.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/drnoksnes/player1/touchscreen/show_buttons</key>
      <applyto>/apps/maemo/drnoksnes/player1/touchscreen/show_buttons</applyto>
      <owner>drnoksnes</owner>
      <type>bool</type>
      <default>false</default>
      <locale name="C">
        <short>Player 1 touchscreen show</short>
        <long>
          Show touchscreen buttons for player 1.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/drnoksnes/player1/zeemote/enable</key>
      <applyto>/apps/maemo/drnoksnes/player1/zeemote/enable</applyto>
      <owner>drnoksnes</owner>
      <type>bool</type>
      <default>false</default>
      <locale name="C">
        <short>Player 1 zeemote</short>
        <long>
          Connect zeemote to player 1.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/drnoksnes/player2/keyboard/enable</key>
      <applyto>/apps/maemo/drnoksnes/player2/keyboard/enable</applyto>
      <owner>drnoksnes</owner>
      <type>bool</type>
      <default>false</default>
      <locale name="C">
        <short>Player 2 keyboard</short>
        <long>
          Enable key mappings for player 2.
        </long>
      </locale>
    </schema>
dnl Player 2 keybindings
define(`HELP', `')dnl
define(`BUTTON', `dnl
    <schema>
     <key>/schemas/apps/maemo/drnoksnes/player2/keyboard/$2</key>
      <applyto>/apps/maemo/drnoksnes/player2/keyboard/$2</applyto>
      <owner>drnoksnes</owner>
      <type>int</type>
      <default>0</default>
      <locale name="C">
        <short>$1 button</short>
      </locale>
    </schema>
dnl')dnl
define(`ACTION', `')dnl
define(`LAST', `')dnl
include(buttons.inc)
undefine(`HELP')dnl
undefine(`BUTTON')dnl
undefine(`ACTION')dnl
undefine(`LAST')dnl
    <schema>
     <key>/schemas/apps/maemo/drnoksnes/player2/touchscreen/enable</key>
      <applyto>/apps/maemo/drnoksnes/player2/touchscreen/enable</applyto>
      <owner>drnoksnes</owner>
      <type>bool</type>
      <default>false</default>
      <locale name="C">
        <short>Player 2 touchscreen</short>
        <long>
          Enable touchscreen buttons for player 2.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/drnoksnes/player2/touchscreen/show_buttons</key>
      <applyto>/apps/maemo/drnoksnes/player2/touchscreen/show_buttons</applyto>
      <owner>drnoksnes</owner>
      <type>bool</type>
      <default>false</default>
      <locale name="C">
        <short>Player 2 touchscreen show</short>
        <long>
          Show touchscreen buttons for player 2.
        </long>
      </locale>
    </schema>
    <schema>
     <key>/schemas/apps/maemo/drnoksnes/player2/zeemote/enable</key>
      <applyto>/apps/maemo/drnoksnes/player2/zeemote/enable</applyto>
      <owner>drnoksnes</owner>
      <type>bool</type>
      <default>false</default>
      <locale name="C">
        <short>Player 2 zeemote</short>
        <long>
          Connect zeemote to player 2.
        </long>
      </locale>
    </schema>
  </schemalist>
</gconfschemafile>
