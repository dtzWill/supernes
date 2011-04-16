HELP(button name,
	button slug,
	button action,
	diablo binding for player 1,
	fremantle binding for player 1)
BUTTON("A", a, P(A), 48, 40)
BUTTON("B", b, P(B), 20, 53)
BUTTON("X", x, P(X), 32, 25)
BUTTON("Y", y, P(Y), 45, 38)
BUTTON("L", l, P(TL), 24, 37)
BUTTON("R", r, P(TR), 22, 22)
BUTTON("Start", start, P(START), 65, 65)
BUTTON("Select", select, P(SELECT), 135, 58)
BUTTON("↑", dn, P(UP), 111, 111)
BUTTON("↓", ds, P(DOWN), 116, 116)
BUTTON("←", dw, P(LEFT), 113, 113)
BUTTON("→", de, P(RIGHT), 114, 114)
BUTTON("←↑", dnw, P(UP) | P(LEFT), 0, 0)
BUTTON("↑→", dne, P(UP) | P(RIGHT), 0, 0)
BUTTON("←↓", dsw, P(DOWN) | P(LEFT), 0, 0)
BUTTON("↓→", dse, P(DOWN) | P(RIGHT), 0, 0)
ACTION(N_("Return to launcher"), quit, A(Quit), 9, 50)
ACTION(N_("Fullscreen"), fullscreen, A(ToggleFullscreen), 72, 0)
ACTION(N_("Quick Load 1"), quickload1, A(QuickLoad1), 0, 0)
ACTION(N_("Quick Save 1"), quicksave1, A(QuickSave1), 0, 0)
ACTION(N_("Quick Load 2"), quickload2, A(QuickLoad2), 0, 0)
ACTION(N_("Quick Save 2"), quicksave2, A(QuickSave2), 0, 0)
LAST
