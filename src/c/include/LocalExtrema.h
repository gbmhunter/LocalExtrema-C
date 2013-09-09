//!
//! @file 		LocalExtrema.h
//! @author 	Geoffrey Hunter <gbmhunter@gmail.com>
//! @edited 	n/a
//! @date 		17/01/2013
//! @brief 		
//!
//! @details
//!				See README.rst in the root dir for more info.
//!
//! NOTES:
//! - Does not return the indicies of where the extrema are
//! - Object orientated, so that multiple local extrema algorithms
//!   	can be run simultaneously.
//!
//! UNIT TEST STATUS:
//! - Works correctly with test data
//!

//===============================================================================================//
//======================================= HEADER GAURD ==========================================//
//===============================================================================================//

#ifndef LOCAL_EXTREMA_H
#define LOCAL_EXTREMA_H

//===============================================================================================//
//======================================= PUBLIC DEFINES ========================================//
//===============================================================================================//

#define SAMPLE_WINDOW_WIDTH (2*configSAMPLE_WINDOW_RADIUS + 1)

//===============================================================================================//
//=================================== PUBLIC TYPEDEFS ===========================================//
//===============================================================================================//

typedef enum
{
	maxFirstTimeLeft,
	maxFirstTimeRight,
	minFirstTimeLeft,
	minFirstTimeRight,
	possMaximaDetected,
	possMinimaDetected,
	maximaFound,
	minimaFound
} statusRegExtrema_t;

//! Holdes all the data for the local extrema algorithm. Used like a "class"
//! so that multiple local extrema algorithms can run simultaneously. 
//! Consumes 84+SAMPLE_WINDOW_WIDTH bytes
typedef struct 
{
	bool_t thresholdingEnabled;
	float thresholdValue;
	bool_t alternateExtremaRuleEnabled;
	//! @warning	Uses RAM!
	float sampleWindowArray[SAMPLE_WINDOW_WIDTH];
	
	//! @note	This restricts the total number of data points that can be
	//!			counted to 2^32-1. Algorithm still works if number of
	//!			points exceeds this, but this counter stops incrementing.
	uint32 totDataPoints;
	
	//! Used to determine special procedure which happens the first
	// time LocalExreme_Calculate() is called
	
	uint8 statusRegGeneral;
	uint8_t statusRegExtrema;
	
	//bool_t maxFirstTimeLeft;
	//bool_t maxFirstTimeRight;
	//bool_t minFirstTimeLeft;
	//bool_t minFirstTimeRight;
	
	float maximaMinLeftValue;
	float maximaMinRightValue;
	float minimaMaxLeftValue;
	float minimaMaxRightValue;
	
	//bool_t possMaximaDetected;
	//bool_t possMinimaDetected;
	float lastPossMaxima;
	float lastPossMinima;
	float lastVerifiedMaxima;
	float lastVerifiedMinima;
	
	//! TRUE when a maxima has been found and verified.
	//bool_t maximaFound;
	
	//! TRUE when a minima has been found and verified.
	//bool_t minimaFound;
} localExtremaObj_t;


//===============================================================================================//
//================================ PUBLIC FUNCTION DECLARATIONS =================================//
//===============================================================================================//

//! @brief		Initialisation routine. Call before calling any other
//!				function.
//! @public
void LocalExtrema_Init(localExtremaObj_t *obj);

//! @brief		Sets the search window radius size
//! @details	Make sure that this small enough to not incorporate
//!				two local maxima in the window, otherwise they will
//!				not be correctly found
//! @param		radius The radius (in terms of the number of data points)
//!					of the seach window.
//! @public
void LocalExtrema_SetSearchWindowRadius(localExtremaObj_t *obj, uint8 radius);

//! @brief		Enables thresholding
//! @param		trueFalse TRUE enables thresholding, FALSE disables thresholding
//! @public
void LocalExtrema_EnableThresholding(localExtremaObj_t *obj, bool_t trueFalse);

//! @brief		Sets the threshold value. Only applicable if thesholding
//!				is enabled.
//! @param		thresholdValue Value to set for thresholding
//! @public
void LocalExtrema_SetThresholdValue(localExtremaObj_t *obj, float thresholdValue);

//! @brief		Enables the alternate extrema rule
//! @details	If enabled, it forces a maxima to be found after a minima, and
//!				vise versa
//! @param		trueFalse TRUE enables the rule, FALSE disables the rule
//! @public
void LocalExtrema_EnableAlternateExtremaRule(localExtremaObj_t *obj, bool_t trueFalse);

//! @brief		Finds local extrema (maxima/minima) values
//! @details	Online algorithm to save embedded memory space (it does not require an entire data
//!				array to be passed to it, just one point at a time). Only retains information
//!				that is useful.
//! @public
void LocalExtrema_Run(localExtremaObj_t *obj, float dataPoint);

//! @brief		Resets on read.
//! @public
bool_t LocalExtrema_IsMaximaFound(localExtremaObj_t *obj);

//! @brief		Resets on read.
//! @public
bool_t LocalExtrema_IsMinimaFound(localExtremaObj_t *obj);

//! @public
float LocalExtrema_GetMaxima(localExtremaObj_t *obj);

//! @public
float LocalExtrema_GetMinima(localExtremaObj_t *obj);

#endif 	// #ifndef LOCAL_EXTREMA_H

// EOF
