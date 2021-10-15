#ifndef K4H
#define K4_H

#include "quantum.h"

#if defined(KEYBOARD_keychron_k4_v1)
  #include "v1.h"
#elif defined(KEYBOARD_keychron_k4_v2)
  #include "v2.h"
#endif // Keychron K4 revisions

#endif