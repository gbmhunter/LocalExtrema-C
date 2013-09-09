//!
//! @file 		LocalExtrema.c
//! @author 	Geoffrey Hunter <gbmhunter@gmail.com> 
//! @edited 	n/a
//! @date 		17/01/2013
//! @brief 		
//! @details
//!				See README.rst in the root dir for more info.

//===============================================================================================//
//========================================== INCLUDES ===========================================//
//===============================================================================================//

#include "Config.h"
#if(configINCLUDE_LOCAL_EXTREMA == 1)

#ifdef __cplusplus
extern "C" {
#endif
	// PSoC includes
	#include <device.h>
#ifdef __cplusplus
}
#endif

// User includes
#include "PublicObjects.h"
#include "./LocalExtrema/include/LocalExtrema.h"
#include "./CapSense/include/CapSense.h"
#include "./SSD1306/include/SSD1306.h"
#include "./UartDebug/include/UartDebug.h"
#include "./Maths/include/Maths.h"


//===============================================================================================//
//===================================== PRE-COMPILER CHECKS =====================================//
//===============================================================================================//

#ifndef configDEBUG_LOCAL_EXTREMA
	#error Please define the switch configDEBUG_LOCAL_EXTREMA
#endif

//===============================================================================================//
//========================================== DEFINES ============================================//
//===============================================================================================//

// none

//===============================================================================================//
//======================================= PRIVATE TYPEDEFS ======================================//
//===============================================================================================//

//! Used to distinguish a particular extrema type (maximua or minima, or none if nothing detected)
typedef enum
{
	NONE,
	MAXIMA,
	MINIMA
} extremaType_t;

//===============================================================================================//
//====================================== PRIVATE VARIABLES ======================================//
//===============================================================================================//



//===============================================================================================//
//===================================== GLOBAL FUNCTIONS ========================================//
//===============================================================================================//

// See Doxygen documentation or function declarations in LocalExtrema.h for more info.

void LocalExtrema_Init(localExtremaObj_t *obj)
{
	obj->thresholdingEnabled = FALSE;
	obj->thresholdValue = 0;
	obj->alternateExtremaRuleEnabled = FALSE;
	obj->totDataPoints = 0;
	
	obj->statusRegExtrema |= (1<<maxFirstTimeLeft);
	obj->statusRegExtrema &= ~(1<<maxFirstTimeRight);
	obj->statusRegExtrema |= (1<<minFirstTimeLeft);
	obj->statusRegExtrema &= ~(1<<minFirstTimeRight);
	
	obj->statusRegExtrema &= ~(1<<possMaximaDetected);
	obj->statusRegExtrema &= ~(1<<possMinimaDetected);
	
	obj->statusRegExtrema &= ~(1<<maximaFound);
	obj->statusRegExtrema &= ~(1<<minimaFound);
}

void LocalExtrema_EnableThresholding(localExtremaObj_t *obj, bool_t trueFalse)
{
	obj->thresholdingEnabled = trueFalse;
}

void LocalExtrema_SetThresholdValue(localExtremaObj_t *obj, float thresholdValue)
{
	obj->thresholdValue = thresholdValue;
}

void LocalExtrema_EnableAlternateExtremaRule(localExtremaObj_t *obj, bool_t trueFalse)
{
	obj->alternateExtremaRuleEnabled = trueFalse;
}

void LocalExtrema_Run(localExtremaObj_t *obj, float dataPoint)
{
	uint8 x;
	float maxResult;
	float minResult;
	float pointToAnalyse;
	
	#if(configDEBUG_LOCAL_EXTREMA == 1)
		UartDebug_PutString("EXTREMA: Local extrema algorithm running...\r\n");
	#endif
	
	
	//===== SHIFT DATA INTO SAMPLE WINDOW ====//
	
	// Shift into sample window data array over and 
	// add new data point on the end
	for(x = 0; x <= SAMPLE_WINDOW_WIDTH - 2; x++)
	{
		obj->sampleWindowArray[x] = obj->sampleWindowArray[x + 1];
	}
	obj->sampleWindowArray[SAMPLE_WINDOW_WIDTH - 1] = dataPoint;

	//===== KEEP TRACK OF TOTAL NUMBER OF DATA POINTS ====//

	// Increment total data points (check for overflow)
	if(obj->totDataPoints != 4294967295u)
		obj->totDataPoints++;
		
	
	//========== RUNNING RECORD OF MAX/MIN IN LEFT(1) =========//
	
	// Exit if not enough points to look at threshold values
	if(obj->totDataPoints < (SAMPLE_WINDOW_WIDTH - 1)/2 + 2)
		return;
	
	// On left of possible extrema
	
	// Special first-time case
	if(obj->statusRegExtrema & (1<<maxFirstTimeLeft))
	{
		obj->maximaMinLeftValue = obj->sampleWindowArray[(SAMPLE_WINDOW_WIDTH - 1)/2 - 1];
		obj->statusRegExtrema &= ~(1<<maxFirstTimeLeft);
	}
	
	// Special first-time case
	if(obj->statusRegExtrema & (1<<minFirstTimeLeft))
	{
		obj->minimaMaxLeftValue = obj->sampleWindowArray[(SAMPLE_WINDOW_WIDTH - 1)/2 - 1];
		obj->statusRegExtrema &= ~(1<<minFirstTimeLeft);
	}
	
	
	// LEFT MAX/MIN
	
	if(!obj->statusRegExtrema & (1<<possMaximaDetected))
	{
		// Set left threshold trackers
		if(obj->sampleWindowArray[(SAMPLE_WINDOW_WIDTH - 1)/2 - 1] < obj->maximaMinLeftValue)
			obj->maximaMinLeftValue = obj->sampleWindowArray[(SAMPLE_WINDOW_WIDTH - 1)/2 - 1];
	}
	
	if(!obj->statusRegExtrema & (1<<possMinimaDetected))
	{
		// Set left threshold trackers
		if(obj->sampleWindowArray[(SAMPLE_WINDOW_WIDTH - 1)/2 - 1] > obj->minimaMaxLeftValue)
			obj->minimaMaxLeftValue = obj->sampleWindowArray[(SAMPLE_WINDOW_WIDTH - 1)/2 - 1];
	}
	
	// Quit if sample window array is not yet full
	// (this will occur for the first SAMPLE_WINDOW_WIDTH
	// data points)
	if(obj->totDataPoints < SAMPLE_WINDOW_WIDTH)
		return;

	//===== FIND MAX/MIN IN SAMPLE WINDOW ====//

	maxResult = Maths_FindMaxOfArrayFloat(obj->sampleWindowArray, sizeof(obj->sampleWindowArray)/sizeof(obj->sampleWindowArray[0]));
	minResult = Maths_FindMinOfArrayFloat(obj->sampleWindowArray, sizeof(obj->sampleWindowArray)/sizeof(obj->sampleWindowArray[0]));
	
    // Analyse the middle data point in the sample window
	pointToAnalyse = obj->sampleWindowArray[(SAMPLE_WINDOW_WIDTH - 1)/2];

	if(pointToAnalyse == maxResult)
	{
		if(obj->statusRegExtrema & (1<<possMaximaDetected))
		{
			// Make sure it is larger than last possible maxima otherwise ignore
			if(pointToAnalyse > obj->lastPossMaxima)
			{
				obj->lastPossMaxima = pointToAnalyse;
			}
		}
		else
		{
			obj->lastPossMaxima = pointToAnalyse;
			obj->statusRegExtrema |= (1<<possMaximaDetected);
			obj->statusRegExtrema |= (1<<maxFirstTimeRight);;
		}
	}
	else if(pointToAnalyse == minResult)
	{
		if(obj->statusRegExtrema & (1<<possMinimaDetected))
		{
			if(pointToAnalyse < obj->lastPossMinima)
				obj->lastPossMinima = pointToAnalyse;
		}
		else
		{
			obj->lastPossMinima = pointToAnalyse;
			obj->statusRegExtrema |= (1<<possMinimaDetected);
			obj->statusRegExtrema |= (1<<minFirstTimeRight);
		}
	}
	
	//========= RIGHT MAX/MIN ============//

	if(obj->statusRegExtrema & (1<<maxFirstTimeRight))
	{
		obj->maximaMinRightValue = obj->sampleWindowArray[(SAMPLE_WINDOW_WIDTH - 1)/2 + 1];
		obj->statusRegExtrema &= ~(1<<maxFirstTimeRight);
	}
	
	if(obj->statusRegExtrema & (1<<minFirstTimeRight))
	{
		obj->minimaMaxRightValue = obj->sampleWindowArray[(SAMPLE_WINDOW_WIDTH - 1)/2 + 1];
		obj->statusRegExtrema &= ~(1<<minFirstTimeRight);
	}

	if(obj->statusRegExtrema & (1<<possMaximaDetected))
	{
		// Set left threshold trackers
		if(obj->sampleWindowArray[(SAMPLE_WINDOW_WIDTH - 1)/2 + 1] < obj->maximaMinRightValue)
			obj->maximaMinRightValue = obj->sampleWindowArray[(SAMPLE_WINDOW_WIDTH - 1)/2 + 1];
	}
	
	if(obj->statusRegExtrema & (1<<possMinimaDetected))
	{
		// Set left threshold trackers
		if(obj->sampleWindowArray[(SAMPLE_WINDOW_WIDTH - 1)/2 + 1] > obj->minimaMaxRightValue)
			obj->minimaMaxRightValue = obj->sampleWindowArray[(SAMPLE_WINDOW_WIDTH - 1)/2 + 1];
	}
	
	//========== THRESHOLD CHECKING =========//
	
	// Check to see if any of the data causes possible maxima/minima to be validated
	// because it meets the threshold
	if(obj->statusRegExtrema & (1<<possMaximaDetected))
	{
		if((obj->lastPossMaxima - obj->maximaMinLeftValue >= obj->thresholdValue) && (obj->lastPossMaxima - obj->maximaMinRightValue >= obj->thresholdValue))
		{
			obj->lastVerifiedMaxima = obj->lastPossMaxima;
			obj->statusRegExtrema &= ~(1<<possMaximaDetected);
			obj->statusRegExtrema |= (1<<maxFirstTimeLeft);
			// Left threshold for next minima is the just-found maxima
			obj->minimaMaxLeftValue = obj->lastPossMaxima;
			// Set user-accessable flag true
			obj->statusRegExtrema |= (1<<maximaFound);
			#if(configDEBUG_LOCAL_EXTREMA == 1)
				UartDebug_PutString("EXTREMA: Maxima found.\r\n");
			#endif
		}
	}
	
	if(obj->statusRegExtrema & (1<<possMinimaDetected))
	{
		if((obj->minimaMaxLeftValue - obj->lastPossMinima >= obj->thresholdValue) && (obj->minimaMaxRightValue - obj->lastPossMinima >= obj->thresholdValue))
		{
			obj->lastVerifiedMinima = obj->lastPossMinima;
			obj->statusRegExtrema &= ~(1<<possMinimaDetected);
			// Left threshold for next maxima is the just-found minima
			obj->maximaMinLeftValue = obj->lastPossMinima;
			obj->statusRegExtrema |= (1<<minFirstTimeLeft);
			// Set user-accessable flag true
			obj->statusRegExtrema |= (1<<minimaFound);
			#if(configDEBUG_LOCAL_EXTREMA == 1)
				UartDebug_PutString("EXTREMA: Minima found.\r\n");
			#endif
		}
	}
}

bool_t LocalExtrema_IsMaximaFound(localExtremaObj_t *obj)
{
	// Reset on read
	if(obj->statusRegExtrema & (1<<maximaFound))
	{
		obj->statusRegExtrema &= ~(1<<maximaFound);
		return TRUE;
	}
	else
		return FALSE;	
}

bool_t LocalExtrema_IsMinimaFound(localExtremaObj_t *obj)
{
	// Reset on read
	if(obj->statusRegExtrema & (1<<minimaFound))
	{
		obj->statusRegExtrema &= ~(1<<minimaFound);
		return TRUE;
	}
	else
		return FALSE;
}

float LocalExtrema_GetMaxima(localExtremaObj_t *obj)
{
	return obj->lastVerifiedMaxima;
}

float LocalExtrema_GetMinima(localExtremaObj_t *obj)
{
	return obj->lastVerifiedMinima;
}

//===============================================================================================//
//====================================== PRIVATE FUNCTIONS ======================================//
//===============================================================================================//

#endif	// #if(configINCLUDE_LOCAL_EXTREMA == 1)
