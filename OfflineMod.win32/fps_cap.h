/**
* @file fps_cap.h
* @brief Optional frame-rate cap for the Brave Frontier Windows client.
*
* The shipped client drives its render loop from the XAML
* CompositionTarget::Rendering event, which fires once per display refresh
* with no internal gating. On displays above 60 Hz the game logic ticks
* faster than designed: animations play too fast and short mouse clicks
* register as held-down. This hook throttles the loop back to FPS_CAP.
*/
#pragma once

#include <serverconfig.h>

#if defined(FPS_CAP) && FPS_CAP > 0

void FpsCap_Attach(void);
void FpsCap_Detach(void);

#else

static inline void FpsCap_Attach(void) {}
static inline void FpsCap_Detach(void) {}

#endif
