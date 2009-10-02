<?xml version="1.0" encoding="utf-8"?>
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
      <key>/schemas/apps/maemo/drnoksnes/display-controls</key>
      <applyto>/apps/maemo/drnoksnes/display-controls</applyto>
      <owner>drnoksnes</owner>
      <type>bool</type>
      <default>false</default>
      <locale name="C">
        <short>Display onscreen controls</short>
        <long>
          Display a grid with the onscreen controls if touchscreen controls
          are enabled.
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
      <key>/schemas/apps/maemo/drnoksnes/mapping</key>
      <applyto>/apps/maemo/drnoksnes/mapping</applyto>
      <owner>drnoksnes</owner>
      <type>int</type>
      <default>1</default>
      <locale name="C">
        <short>Key mapping setting</short>
        <long>
          Set to 0 for None, 1 for Keyboard only, etc.
        </long>
      </locale>
    </schema>
    <schema>
      <key>/schemas/apps/maemo/drnoksnes/rom</key>
      <applyto>/apps/maemo/drnoksnes/rom</applyto>
      <owner>drnoksnes</owner>
      <type>string</type>
      <locale name="C">
        <short>ROM to load</short>
        <long>
          Full path to the ROM file to load on next startup.
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
ifelse(eval(MAEMO_MAJOR < 5), 1, `dnl
    <schema>
     <key>/schemas/apps/maemo/drnoksnes/xsp</key>
      <applyto>/apps/maemo/drnoksnes/xsp</applyto>
      <owner>drnoksnes</owner>
      <type>bool</type>
      <default>true</default>
      <locale name="C">
        <short>Pixel doubling</short>
        <long>
          Enable the use of the hardware pixel doubler. Without it,
          a slower, lower quality software scaler is used.
        </long>
      </locale>
    </schema>
', `')dnl
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
  </schemalist>
</gconfschemafile>
