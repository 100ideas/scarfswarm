/* KnobSettings.h (2023.25 knob hello world based off liteswarm another-try branch)
** https://github.com/counterbeing/liteswarm/blob/da3435ba0c8b6389217ee285d9a3c42f69a240b2/src/KnobSettings.h
**
** 
*/

#pragma once
#include <stdint.h>

struct KnobSettings {
  int32_t currentValue;
  int32_t minValue;
  int32_t maxValue;
  bool loopRotary;
};