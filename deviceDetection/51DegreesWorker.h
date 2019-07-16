#ifndef IIW_51DEGREESWORKER_H
#define IIW_51DEGREESWORKER_H

#endif //IIW_51DEGREESWORKER_H

#include <stdio.h>
#include "../Device-Detection-3.2.20.4/src/pattern/51Degrees.h"
#include "mbInfoStruct.h"

mbInfoPtr getUserAgentProperties(char *userAgent);
// Global settings and properties.
static fiftyoneDegreesProvider provider;

// Function declarations.
const char* getIsMobile(fiftyoneDegreesWorkset* ws);
const char* getDeviceHeight(fiftyoneDegreesWorkset* ws);
const char* getDeviceWidth(fiftyoneDegreesWorkset* ws);


