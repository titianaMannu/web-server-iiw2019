#include "51DegreesWorker.h"
#include <stdlib.h>
#include <string.h>

/**
 * This method gets the userAgent string of Http request field and retrieves the following:
 * - Check if the device is a mobile device
 * - Width of the device screen
 * - Height of the device screen
 * All three of these will be saved into a mobile_info struct and will be used to resize the image.
 * @param userAgent string of user agent
 * @return mobile_info struct pointer
 */
mbInfoPtr getUserAgentProperties(char *userAgent) {

    // intializes struct to return all we need to know about the mobile device
    mbInfoPtr resInfo = malloc(sizeof(struct mobile_info));
    if (resInfo == NULL) {
        perror("unable to allocate memory");
        exit(EXIT_FAILURE);
    }
    memset(resInfo, 0, sizeof(struct mobile_info));

    const char *fileName = "data/51Degrees-LiteV3.2.dat";

    // Declare Resources and properties
    fiftyoneDegreesWorkset *ws = NULL;
    char *properties = "IsMobile,ScreenPixelsHeight,ScreenPixelsWidth";
    int cacheSize = 1000;
    int poolSize = 5;

    // Initialise provider with the needed properties (not used)
    fiftyoneDegreesInitProviderWithPropertyString(
            fileName, &provider, properties, poolSize, cacheSize);

    // Get workset from the pool.
    ws = fiftyoneDegreesProviderWorksetGet(&provider);

    // Perform detection.
    fiftyoneDegreesMatch(ws, userAgent);

    //Writes results into the mobile_info struct
    strcpy(resInfo->isMobile, getIsMobile(ws));
    strcpy(resInfo->pxlHeight, getDeviceHeight(ws));
    strcpy(resInfo->pxlWidth, getDeviceWidth(ws));

    // Return Workset to the pool.
    fiftyoneDegreesWorksetRelease(ws);

    // Free provider when program complete.
    fiftyoneDegreesProviderFree(&provider);

    return resInfo;
}

/**
 * Returns a string representation of the value associated with the IsMobile
 * property.
 * @param initialised workset of type fiftyoneDegreesWorkset
 * @returns a string representation of the value for IsMobile
 */
const char *getIsMobile(fiftyoneDegreesWorkset *ws) {
    int32_t requiredPropertyIndex;
    const char *isMobile;
    const fiftyoneDegreesAsciiString *valueName;
    const char *property = "IsMobile";
    //Get the index of the property "isMobile"
    requiredPropertyIndex = fiftyoneDegreesGetRequiredPropertyIndex(ws->dataSet, property);
    //Sets the value
    fiftyoneDegreesSetValues(ws, requiredPropertyIndex);
    //Retrieve the value in string
    valueName = fiftyoneDegreesGetString(ws->dataSet, ws->values[0]->nameOffset);
    //save the string in the variable and returns it
    isMobile = &(valueName->firstByte);
    return isMobile;
}

/**
 * Gets The Heigth property in pixels of the mobile device  and returns it as a string
 * @param ws workset of 51degrees
 * @return the height in pixel of the device in a string
 */
const char *getDeviceHeight(fiftyoneDegreesWorkset *ws) {
    int32_t requiredPropertyIndex;
    const char *deviceHeight = NULL;
    const fiftyoneDegreesAsciiString *valueName;
    const char *property = "ScreenPixelsHeight";
    requiredPropertyIndex = fiftyoneDegreesGetRequiredPropertyIndex(ws->dataSet, property);
    if (requiredPropertyIndex != -1) {
        fiftyoneDegreesSetValues(ws, requiredPropertyIndex);
        valueName = fiftyoneDegreesGetString(ws->dataSet, ws->values[0]->nameOffset);
        deviceHeight = &(valueName->firstByte);
    }
    return deviceHeight;
}

/**
 * Gets The Width property in pixels of the mobile device  and returns it as a string
 * @param ws workset of 51degrees
 * @return the height in pixel of the device in a string
 */
const char *getDeviceWidth(fiftyoneDegreesWorkset *ws) {
    int32_t requiredPropertyIndex;
    const char *deviceWidth = NULL;
    const fiftyoneDegreesAsciiString *valueName;
    const char *property = "ScreenPixelsWidth";
    requiredPropertyIndex = fiftyoneDegreesGetRequiredPropertyIndex(ws->dataSet, property);
    if (requiredPropertyIndex != -1) {
        fiftyoneDegreesSetValues(ws, requiredPropertyIndex);
        valueName = fiftyoneDegreesGetString(ws->dataSet, ws->values[0]->nameOffset);
        deviceWidth = &(valueName->firstByte);
    }

    return deviceWidth;
}
