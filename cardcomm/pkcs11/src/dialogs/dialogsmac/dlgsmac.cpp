/* ****************************************************************************

 * eID Middleware Project.
 * Copyright (C) 2014 FedICT.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version
 * 3.0 as published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, see
 * http://www.gnu.org/licenses/.

**************************************************************************** */

#include <stdlib.h>
#include <signal.h>
#include "errno.h"

#include "../dialogs.h"
#include "../langUtil.h"

#include <map>

//#include "Log.h"
#include "Util.h"
#include "MWException.h"
#include "eidErrors.h"
#include "configuration.h"

//CoreFoundation/CoreFoundation.h
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFUserNotification.h>

using namespace eIDMW;

	/************************
	*       DIALOGS
	************************/
#define BEID_MAX_MESSAGE_ARRAY_LEN 8

typedef std::map<unsigned long, CFUserNotificationRef> TD_MCPINPAD_MAP;

TD_MCPINPAD_MAP mc_pinpad_map;
unsigned long mc_pinpad_map_index = 0;

CFStringRef CreateStringFromWChar(const wchar_t * wcsMessage)
{
    char messagechar[256];
    
    if(wcsMessage == NULL)
        return NULL;
    wcstombs(messagechar,wcsMessage,sizeof(messagechar));
    messagechar[255]='\0';
    
    return CFStringCreateWithBytes (
                                    kCFAllocatorDefault,
                                    (const UInt8 *)messagechar,
                                    strlen(messagechar),
                                    kCFStringEncodingUTF8,
                                    false
                                    );
}

void AppendToStringFromWChar(CFMutableStringRef mutableString, const wchar_t * wcsMessage)
{
    char messagechar[256];
    
    if(wcsMessage != NULL)
    {
    wcstombs(messagechar,wcsMessage,sizeof(messagechar));
    messagechar[255]='\0';
    
    CFStringRef tmpString = CFStringCreateWithBytes (
                                    kCFAllocatorDefault,
                                    (const UInt8 *)messagechar,
                                    strlen(messagechar),
                                    kCFStringEncodingUTF8,
                                    false
                                    );
    
    CFStringAppend(mutableString, tmpString);
    CFRelease(tmpString);
    }
}

void AppendButtonToArrays (unsigned char ulButtons, unsigned char ulButtonToAdd,
                                   CFMutableArrayRef mutArrayKeys,CFMutableArrayRef mutArrayValues,
                                   CFStringRef buttonTitleKey)
{
    if(ulButtonToAdd & ulButtons)
    {
        CFArrayAppendValue(mutArrayKeys, buttonTitleKey);
        //DLG_BUTTON_ALWAYS | DLG_BUTTON_YES | DLG_BUTTON_NO
        switch (ulButtonToAdd) {
            case DLG_BUTTON_YES:
                CFArrayAppendValue(mutArrayValues, CreateStringFromWChar(GETSTRING_DLG(Yes)) );
                break;
            case DLG_BUTTON_NO:
                CFArrayAppendValue(mutArrayValues, CreateStringFromWChar(GETSTRING_DLG(No)) );
                break;
            default:
                CFArrayAppendValue(mutArrayValues, CreateStringFromWChar(GETSTRING_DLG(Ok)) );
                break;
        }
    }
}


DLGS_EXPORT DlgRet eIDMW::DlgAskPin(DlgPinOperation operation,
                                    DlgPinUsage usage, const wchar_t *wsPinName,
                                    DlgPinInfo pinInfo, wchar_t *wsPin, unsigned long ulPinBufferLen)
{
    DlgRet lRet = DLG_CANCEL;
    
	std::string csReadableFilePath;
    const wchar_t * wcsTitle;
	CFUserNotificationRef userNotificationRef;
	SInt32 error = 0;
    
    size_t textLen = 0;
    char titlechar[256];
    CFOptionFlags optionFlags = CFUserNotificationSecureTextField(0);
    CFOptionFlags responseFlags;
	
    try {
        
        //create header text
        CFStringRef headerString = CreateStringFromWChar(GETSTRING_DLG(EnterYourPin));
        
        //create title text and url
        CFStringRef IconURLString = NULL;
        if( usage == DLG_PIN_SIGN )
        {
            optionFlags |= kCFUserNotificationCautionAlertLevel;
            wcsTitle=GETSTRING_DLG(SigningWith);
            IconURLString = CreateStringFromWChar(L"Users/Frederik/4_0_7_QT45/eid-mw/_src/eidmw/dialogs/dialogsQTsrv/Resources/ICO_CARD_DIGSIG_128x128.png");
        }
        else
        {
            optionFlags |= kCFUserNotificationPlainAlertLevel;
            wcsTitle=GETSTRING_DLG(Asking);
            IconURLString = CreateStringFromWChar(L"Users/Frederik/4_0_7_QT45/eid-mw/_src/eidmw/dialogs/dialogsQTsrv/Resources/ICO_CARD_PIN_128x128.png");
        }
        wcstombs(titlechar,wcsTitle,sizeof(titlechar));
        titlechar[255]='\0';
        
        CFURLRef urlRef = CFURLCreateWithString ( kCFAllocatorDefault, IconURLString, NULL );
        if(urlRef == NULL)
        
        textLen = strlen(titlechar);
        //need room for space and pin name
        if(textLen >= 250)
            return DLG_ERR;
        
        strcpy_s(titlechar+textLen, 1, " ");
        textLen++;
        wcstombs(titlechar+textLen,wsPinName,sizeof(titlechar)-textLen);
        titlechar[255]='\0';
        
        CFStringRef titleString = CFStringCreateWithBytes (
                                                           kCFAllocatorDefault,
                                                           (const UInt8 *)titlechar,
                                                           strlen(titlechar),
                                                           kCFStringEncodingUTF8,
                                                           false
                                                           );
        
        //create message text
        CFStringRef messageString = CreateStringFromWChar(GETSTRING_DLG(Pin));
        
        //create default button text
        CFStringRef defaultButtonString = CreateStringFromWChar(GETSTRING_DLG(Ok));
        
        //create alternate (cancel) button text
        CFStringRef alternateButtonString = CreateStringFromWChar(GETSTRING_DLG(Cancel));
        
        const void* keys[] = {kCFUserNotificationAlertHeaderKey,
            kCFUserNotificationAlertMessageKey,
            kCFUserNotificationTextFieldTitlesKey,
            kCFUserNotificationDefaultButtonTitleKey,
            kCFUserNotificationAlternateButtonTitleKey,
            kCFUserNotificationIconURLKey
        };
        
        const void* values[] = {headerString,
            titleString,
            messageString,
            defaultButtonString,
            alternateButtonString,
            urlRef
        };
        
        CFDictionaryRef parameters = CFDictionaryCreate(0, keys, values,
                                                        sizeof(keys)/sizeof(*keys), NULL,
                                                        NULL);
        
        userNotificationRef = CFUserNotificationCreate (kCFAllocatorDefault, //CFAllocatorRef allocator,
                                                        30, //CFTimeInterval timeout,
                                                        optionFlags,//CFOptionFlags flags,
                                                        &error,//SInt32 *error,
                                                        parameters);//CFDictionaryRef dictionary
        
        error = CFUserNotificationReceiveResponse (
                                                   userNotificationRef,//CFUserNotificationRef userNotification,
                                                   0,//CFTimeInterval timeout,
                                                   &responseFlags//CFOptionFlags responseFlags
                                                   );
        
        CFStringRef PinValue = NULL;
        
        switch (responseFlags & 0x03)
        {
            case kCFUserNotificationDefaultResponse:
                lRet = DLG_OK;
                //get the PIN
                PinValue = CFUserNotificationGetResponseValue ( userNotificationRef, kCFUserNotificationTextFieldValuesKey, 0 );
                break;
            case kCFUserNotificationAlternateResponse:
                lRet = DLG_CANCEL;
                break;
            case kCFUserNotificationOtherResponse:
                lRet = DLG_RETRY;
                break;
            case kCFUserNotificationCancelResponse:
                lRet = DLG_CANCEL;
                break;
            default:
                lRet = DLG_CANCEL;
        }
        
        CFRelease(userNotificationRef);
        CFRelease(parameters);
        CFRelease(messageString);
        CFRelease(titleString);
        CFRelease(headerString);
        
        
    } catch (...) {
        
        return DLG_ERR;
    }
    return lRet;
    
}

DLGS_EXPORT DlgRet eIDMW::DlgAskPins(DlgPinOperation operation,
                                     DlgPinUsage usage, const wchar_t *wsPinName,
                                     DlgPinInfo pin1Info, wchar_t *wsPin1, unsigned long ulPin1BufferLen,
                                     DlgPinInfo pin2Info, wchar_t *wsPin2, unsigned long ulPin2BufferLen)
{
    
    DlgRet lRet = DLG_CANCEL;
    try {
    } catch (...) {
        return DLG_ERR;
    }
    return lRet;
}


DLGS_EXPORT DlgRet eIDMW::DlgBadPin(
                                    DlgPinUsage usage, const wchar_t *wsPinName,
                                    unsigned long ulRemainingTries)
{
    DlgRet lRet = DLG_CANCEL;

    CFMutableStringRef headerString = NULL;
    CFMutableStringRef titleString = NULL;
    CFStringRef defButtonString = NULL;
    CFStringRef altButtonString = NULL;
    
	std::string csReadableFilePath;
	CFUserNotificationRef userNotificationRef = NULL;
	SInt32 error = 0;
	CFOptionFlags optionFlags;
    CFOptionFlags responseFlags;
    
    optionFlags = kCFUserNotificationCautionAlertLevel;
    CFMutableArrayRef mutArrayKeys = NULL;
    CFMutableArrayRef mutArrayValues = NULL;
    CFDictionaryRef parameters = NULL;

    void* keys[BEID_MAX_MESSAGE_ARRAY_LEN];
    void* values[BEID_MAX_MESSAGE_ARRAY_LEN];
    CFRange range;
    try {
        
        mutArrayKeys = CFArrayCreateMutable (kCFAllocatorDefault,
                                             BEID_MAX_MESSAGE_ARRAY_LEN,//CFIndex capacity,
                                             NULL//const CFArrayCallBacks *callBacks
                                             );
        mutArrayValues = CFArrayCreateMutable (kCFAllocatorDefault,
                                               BEID_MAX_MESSAGE_ARRAY_LEN,//CFIndex capacity,
                                               NULL//const CFArrayCallBacks *callBacks
                                               );
        
        //create header string
        headerString = CFStringCreateMutable(kCFAllocatorDefault, 0);
        AppendToStringFromWChar(headerString, GETSTRING_DLG(Notification));
        CFStringAppend(headerString, CFSTR(": "));
        AppendToStringFromWChar(headerString, GETSTRING_DLG(Bad));
        CFStringAppend(headerString, CFSTR(" "));
        AppendToStringFromWChar(headerString, wsPinName);
        
        //create title string
        titleString = CFStringCreateMutable(kCFAllocatorDefault, 0);
        AppendToStringFromWChar(titleString, GETSTRING_DLG(Bad));
        CFStringAppend(titleString, CFSTR(" "));
        AppendToStringFromWChar(titleString, wsPinName);
        CFStringAppend(titleString, CFSTR(": "));
        
        CFDictionaryRef formatOptions = NULL;
        CFStringAppendFormat(titleString, formatOptions, CFSTR("%lu"), ulRemainingTries);
        CFStringAppend(titleString, CFSTR(" "));
        AppendToStringFromWChar(titleString, GETSTRING_DLG(RemainingAttempts));
        CFStringAppend(titleString, CFSTR("\n\n"));
        
        if( ulRemainingTries == 0 )
        {
            AppendToStringFromWChar(titleString, wsPinName);
            CFStringAppend(titleString, CFSTR(" "));
            AppendToStringFromWChar(titleString, GETSTRING_DLG(PinBlocked));
            defButtonString = CreateStringFromWChar(GETSTRING_DLG(Ok));
       
            CFArrayAppendValue(mutArrayKeys, kCFUserNotificationDefaultButtonTitleKey);
            CFArrayAppendValue(mutArrayValues, defButtonString);
        }
        else
        {
            AppendToStringFromWChar(titleString, GETSTRING_DLG(TryAgainOrCancel));
            defButtonString = CreateStringFromWChar(GETSTRING_DLG(Cancel));
            altButtonString = CreateStringFromWChar(GETSTRING_DLG(Retry));
            
            CFArrayAppendValue(mutArrayKeys, kCFUserNotificationDefaultButtonTitleKey);
            CFArrayAppendValue(mutArrayValues, defButtonString);
            CFArrayAppendValue(mutArrayKeys, kCFUserNotificationAlternateButtonTitleKey);
            CFArrayAppendValue(mutArrayValues, altButtonString);
        }
   
        CFArrayAppendValue(mutArrayKeys, kCFUserNotificationAlertHeaderKey);
        CFArrayAppendValue(mutArrayKeys, kCFUserNotificationAlertMessageKey);
        CFArrayAppendValue(mutArrayValues, headerString);
        CFArrayAppendValue(mutArrayValues, titleString);
        
        //ui.lblIcon->setPixmap( QPixmap( ":/Resources/ICO_CARD_NOK_64x64.png" ) );
        
        range.length = CFArrayGetCount(mutArrayValues);
        range.location = 0;
        CFArrayGetValues (mutArrayValues, range,(const void**)values);
        
        range.length = CFArrayGetCount(mutArrayKeys);
        range.location = 0;
        CFArrayGetValues (mutArrayKeys, range,(const void**)keys);
        
        parameters = CFDictionaryCreate(0, (const void**)keys, (const void**)values,
                                                        CFArrayGetCount(mutArrayKeys), NULL,
                                                        NULL);
        
        userNotificationRef = CFUserNotificationCreate (kCFAllocatorDefault, //CFAllocatorRef allocator,
                                                        30, //CFTimeInterval timeout,
                                                        optionFlags,//CFOptionFlags flags,
                                                        &error,//SInt32 *error,
                                                        parameters);//CFDictionaryRef dictionary
        
        error = CFUserNotificationReceiveResponse (
                                                   userNotificationRef,//CFUserNotificationRef userNotification,
                                                   0,//CFTimeInterval timeout,
                                                   &responseFlags//CFOptionFlags responseFlags
                                                   );
        
        switch (responseFlags & 0x03)
        {
            case kCFUserNotificationDefaultResponse:
                lRet = DLG_OK;
                break;
            case kCFUserNotificationAlternateResponse:
                lRet = DLG_CANCEL;
                break;
            case kCFUserNotificationOtherResponse:
                lRet = DLG_RETRY;
                break;
            case kCFUserNotificationCancelResponse:
                lRet = DLG_CANCEL;
                break;
            default:
                lRet = DLG_CANCEL;
        }
        
        
        if(mutArrayKeys != NULL)
            CFRelease(mutArrayKeys);
        if(mutArrayValues != NULL)
            CFRelease(mutArrayValues);
        if(userNotificationRef != NULL)
            CFRelease(userNotificationRef);
        if(parameters != NULL)
            CFRelease(parameters);
        
        if(headerString != NULL)
            CFRelease(headerString);
        if(titleString != NULL)
            CFRelease(titleString);
        if(defButtonString != NULL)
            CFRelease(defButtonString);
        if(altButtonString != NULL)
            CFRelease(altButtonString);
        
    } catch (...) {
        return DLG_ERR;
    }
   
    return lRet;
    
}


//DlgDisplayModal is only used as dialog askaccess at the moment,
DLGS_EXPORT DlgRet eIDMW::DlgDisplayModal(DlgIcon icon,
                                          DlgMessageID messageID, const wchar_t *csMesg,
                                          unsigned char ulButtons, unsigned char ulEnterButton,
                                          unsigned char ulCancelButton)
{
    DlgRet lRet = DLG_CANCEL;
   
	std::string csReadableFilePath;
	CFUserNotificationRef userNotificationRef;
	SInt32 error = 0;
    char datachar[512];
    UInt32 datacharlen = sizeof(datachar);
	CFOptionFlags optionFlags;
    CFOptionFlags responseFlags;
    CFStringRef tittleStrRef;
    
    void* keys[BEID_MAX_MESSAGE_ARRAY_LEN];   //to store the keys
    void* values[BEID_MAX_MESSAGE_ARRAY_LEN]; //to store the values
    CFRange range;
    try {
    //to create the array of keys
    CFMutableArrayRef mutArrayKeys = CFArrayCreateMutable (kCFAllocatorDefault,
                                             BEID_MAX_MESSAGE_ARRAY_LEN,//CFIndex capacity,
                                             NULL//const CFArrayCallBacks *callBacks
                                             );
    //to create the array of values
    CFMutableArrayRef mutArrayValues = CFArrayCreateMutable (kCFAllocatorDefault,
                                               BEID_MAX_MESSAGE_ARRAY_LEN,//CFIndex capacity,
                                               NULL//const CFArrayCallBacks *callBacks
                                               );
        
    switch (icon) {
        case DLG_ICON_ERROR:
            optionFlags = kCFUserNotificationStopAlertLevel;
            break;
        case DLG_ICON_WARN:
            optionFlags = kCFUserNotificationCautionAlertLevel;
            break;
        case DLG_ICON_INFO:
        case DLG_ICON_QUESTION:
            optionFlags = kCFUserNotificationNoteAlertLevel;
            break;
        case DLG_ICON_NONE:
        default:
            optionFlags = kCFUserNotificationPlainAlertLevel;
            break;
    }
    
    if( csMesg == NULL)
    {
        tittleStrRef = CreateStringFromWChar(L"");
    }
    else
    {
        tittleStrRef = CreateStringFromWChar(csMesg);
    }
    std::wstring translatedMessage(CLang::GetMessageFromID(messageID));
    wcstombs(datachar,translatedMessage.c_str(),sizeof(datachar));
    
    datachar[datacharlen-1]='\0';
    
    CFStringRef datacharRefBytes = CFStringCreateWithBytes (
															kCFAllocatorDefault,
															(const UInt8 *)datachar,
															strlen(datachar),
															kCFStringEncodingUTF8,
															false
															);
    
    //always display tittle
    CFArrayAppendValue(mutArrayKeys, kCFUserNotificationAlertHeaderKey);
    CFArrayAppendValue(mutArrayValues, tittleStrRef);
    //always display message
    CFArrayAppendValue(mutArrayKeys, kCFUserNotificationAlertMessageKey);
    CFArrayAppendValue(mutArrayValues, datacharRefBytes);
        
    //check which buttons are requested to be shown
    //ulEnterButton is the one on the right, only show it when part of the button list
    AppendButtonToArrays (ulButtons, ulEnterButton, mutArrayKeys, mutArrayValues,
                              kCFUserNotificationDefaultButtonTitleKey);
        
    //ulCancelButton is the one on the left
    AppendButtonToArrays (ulButtons, ulCancelButton, mutArrayKeys, mutArrayValues,
                              kCFUserNotificationAlternateButtonTitleKey);
        
    if(ulButtons & DLG_BUTTON_ALWAYS)
    {
        CFArrayAppendValue(mutArrayKeys, kCFUserNotificationOtherButtonTitleKey);
        CFArrayAppendValue(mutArrayValues, CreateStringFromWChar(GETSTRING_DLG(Always)) );
    }
        
    range.length = CFArrayGetCount(mutArrayValues);
    range.location = 0;
    CFArrayGetValues (mutArrayValues, range,(const void**)values);
        
    range.length = CFArrayGetCount(mutArrayKeys);
    range.location = 0;
    CFArrayGetValues (mutArrayKeys, range,(const void**)keys);
        
    CFDictionaryRef parameters = CFDictionaryCreate(0, (const void**)keys, (const void**)values,
                                        CFArrayGetCount(mutArrayKeys), NULL,
                                        NULL);
        
	userNotificationRef = CFUserNotificationCreate (kCFAllocatorDefault, //CFAllocatorRef allocator,
													30, //CFTimeInterval timeout,
													optionFlags,//CFOptionFlags flags,
													&error,//SInt32 *error,
													parameters);//CFDictionaryRef dictionary
	
    error = CFUserNotificationReceiveResponse (userNotificationRef,//CFUserNotificationRef userNotification,
                                              0,//CFTimeInterval timeout,
                                              &responseFlags//CFOptionFlags responseFlags
                                              );
    
    if(error != 0)
    {
        lRet = DLG_ERR;
    }
    else
    {
        switch (responseFlags & 0x03)
        {
            case kCFUserNotificationDefaultResponse:
                lRet = DLG_OK;
                break;
            case kCFUserNotificationAlternateResponse:
                lRet = DLG_CANCEL;
                break;
                
            case kCFUserNotificationOtherResponse:
                if (ulButtons & DLG_BUTTON_ALWAYS)
                    lRet = DLG_RETRY;
                else if(ulButtons & DLG_BUTTON_YES)
                    lRet = DLG_YES;
                else if(ulButtons & DLG_BUTTON_NO)
                    lRet = DLG_NO;
                else if(ulButtons & DLG_BUTTON_OK)
                    lRet = DLG_OK;
                else if(ulButtons & DLG_BUTTON_CANCEL)
                    lRet = DLG_CANCEL;
                break;
                
            case kCFUserNotificationCancelResponse:
                lRet = DLG_CANCEL;
                break;
                
            default:
                lRet = DLG_CANCEL;
        }
    }
    CFRelease(tittleStrRef);
    CFRelease(datacharRefBytes);
    CFRelease(mutArrayKeys);
    CFRelease(mutArrayValues);
    CFRelease(parameters);
    CFRelease(userNotificationRef);
    }
    catch (...) {
        return DLG_ERR;
    }
    return lRet;
}


DLGS_EXPORT DlgRet eIDMW::DlgDisplayPinpadInfo(DlgPinOperation operation,
											   const wchar_t *wsReader, DlgPinUsage usage, const wchar_t *wsPinName,
											   const wchar_t *wsMessage,
											   unsigned long *pulHandle)
{
    DlgRet lRet = DLG_CANCEL;
	CFArrayRef           titlesArray;
    
	std::string csReadableFilePath;
    const wchar_t * Title;
	CFUserNotificationRef userNotificationRef;
	SInt32 error = 0;
    char datachar[256];
    char titlechar[256];
    CFOptionFlags optionFlags;
	
	const void* keys[] = {kCFUserNotificationAlertHeaderKey,
		kCFUserNotificationAlertMessageKey};
	
    if( usage == DLG_PIN_SIGN )
	{
        optionFlags = kCFUserNotificationCautionAlertLevel;
		Title=GETSTRING_DLG(YouAreAboutToMakeALegallyBindingElectronic);//SigningWith);
	}
	else
	{
        optionFlags = kCFUserNotificationPlainAlertLevel;
		Title=GETSTRING_DLG(PleaseEnterYourPinOnThePinpadReader);
	}
    
    wcstombs(datachar,wsReader,sizeof(datachar));
    datachar[255]='\0';
    
    CFStringRef datacharRefBytes = CFStringCreateWithBytes (
															kCFAllocatorDefault,
															(const UInt8 *)datachar,
															strlen(datachar),
															kCFStringEncodingUTF8,
															false
															);
    
    wcstombs(titlechar,Title,sizeof(titlechar));
    titlechar[255]='\0';
    CFStringRef titlecharRefBytes = CFStringCreateWithBytes (
															 kCFAllocatorDefault,
															 (const UInt8 *)titlechar,
															 strlen(titlechar),
															 kCFStringEncodingUTF8,
															 false
															 );
    
	const void* textFieldTitles[] = {Title};
    
    titlesArray = CFArrayCreate(NULL, textFieldTitles, 1, NULL);
	
	const void* values[] = {titlecharRefBytes,
        datacharRefBytes};
	
	CFDictionaryRef parameters = CFDictionaryCreate(0, keys, values,
													sizeof(keys)/sizeof(*keys), NULL,
													NULL);
    
	userNotificationRef = CFUserNotificationCreate (kCFAllocatorDefault, //CFAllocatorRef allocator,
													30, //CFTimeInterval timeout,
													optionFlags,//CFOptionFlags flags,
													&error,//SInt32 *error,
													parameters);//CFDictionaryRef dictionary
	mc_pinpad_map[mc_pinpad_map_index++] = userNotificationRef;
    if(pulHandle) {
        *pulHandle = mc_pinpad_map_index;
    }
    
	CFRelease(titlesArray);
	CFRelease(parameters);
    
    return lRet;
}



DLGS_EXPORT void eIDMW::DlgClosePinpadInfo( unsigned long theUserNotificationRef )
{
    SInt32 error = 0;
    
    CFUserNotificationRef userNotificationRef = mc_pinpad_map[theUserNotificationRef];
    error = CFUserNotificationCancel (userNotificationRef);
    
    CFRelease(userNotificationRef);
	
}


/*
//TODO: Add Keypad possibility in DlgAskPin(s)                                      
DLGS_EXPORT DlgRet eIDMW::DlgAskPin(DlgPinOperation operation,
     DlgPinUsage usage, const wchar_t *wsPinName,
     DlgPinInfo pinInfo, wchar_t *wsPin, unsigned long ulPinBufferLen)
{
    DlgRet lRet = DLG_CANCEL;
	CFArrayRef           titlesArray;
	//CFArrayRef           valuesArray;
	//DlgAskAccessArguments *oData = NULL;
	SharedMem oShMemory;
	std::string csReadableFilePath;
    const wchar_t * Title;
	//https://developer.apple.com/library/mac/documentation/CoreFoundation/Reference/CFUserNotificationRef/Reference/reference.html#//apple_ref/c/data/kCFUserNotificationAlertHeaderKey
	CFUserNotificationRef userNotificationRef;
	SInt32 error = 0;
	const void* keys[] = {kCFUserNotificationAlertHeaderKey,
		kCFUserNotificationAlertMessageKey,
		kCFUserNotificationTextFieldTitlesKey,
		//kCFUserNotificationTextFieldValuesKey,
		kCFUserNotificationProgressIndicatorValueKey,
		kCFUserNotificationDefaultButtonTitleKey,
        kCFUserNotificationAlternateButtonTitleKey};
	
    if( usage == DLG_PIN_SIGN )
	{
		Title=GETSTRING_DLG(SigningWith);
		//Title+= ": ";
	}
	else
	{
		Title=GETSTRING_DLG(Asking);
		//Title+= " ";
	}
    
	
	const void* textFieldTitles[] = {Title};
    
    titlesArray = CFArrayCreate(NULL, textFieldTitles, 1, NULL);
	//valuesArray = CFArrayCreate(NULL, textFieldValues, 3, NULL);
	
	//if (array) {
	//	CFArrayAppendValue(array, CFSTR("0"));
	//	CFArrayAppendValue(array, CFSTR("1"));
	//	CFArrayAppendValue(array, CFSTR("2"));
	//  }
	
	const void* values[] = {CFSTR("NotificationAlertHeader"),
        CFSTR("NotificationAlertMessage"),
        CFSTR("NotificationTextFieldTitles"),
        //CFSTR("NotificationTextFieldValuesKey"),
        CFSTR("NotificationProgressIndicatorValue"),
        CFSTR("NotificationDefaultButtonTitle"),
        CFSTR("NotificationAlternateButtonTitle")};
              

	
              CFOptionFlags optionFlags = kCFUserNotificationCautionAlertLevel | CFUserNotificationSecureTextField (0);
	
	CFDictionaryRef parameters = CFDictionaryCreate(0, keys, values,
													
													sizeof(keys)/sizeof(*keys), NULL,
													
													NULL);
	
    //CFDictionaryAddValue(dict, kCFUserNotificationAlertHeaderKey, CFSTR("my title"));
    
    //CFDictionaryAddValue(dict, kCFUserNotificationAlternateButtonTitleKey, CFSTR("Cancel"));

    
	userNotificationRef = CFUserNotificationCreate (kCFAllocatorDefault, //CFAllocatorRef allocator,
													10000, //CFTimeInterval timeout,
													optionFlags,//CFOptionFlags flags,
													&error,//SInt32 *error,
													parameters);//CFDictionaryRef dictionary
	
	
	error = CFUserNotificationReceiveResponse (	userNotificationRef,
											   10000,//CFTimeInterval timeout,
											   &optionFlags);//CFOptionFlags *responseFlags
	//CFUserNotificationCancel
	CFRelease(titlesArray);
	CFRelease(parameters);
	CFRelease(userNotificationRef);

 
    return lRet;
}
 */

/*
DLGS_EXPORT DlgRet eIDMW::DlgAskPins(DlgPinOperation operation,
      DlgPinUsage usage, const wchar_t *wsPinName,
      DlgPinInfo pin1Info, wchar_t *wsPin1, unsigned long ulPin1BufferLen,
      DlgPinInfo pin2Info, wchar_t *wsPin2, unsigned long ulPin2BufferLen)
{

  DlgRet lRet = DLG_CANCEL;

  DlgAskPINsArguments* oData;
  SharedMem oShMemory;
  std::string csReadableFilePath;
  
  try {
    csReadableFilePath = CreateRandomFile();

    // creating the shared memory segment
    // attach oData 
    oShMemory.Attach(sizeof(DlgAskPINsArguments),csReadableFilePath.c_str(),(void**)&oData);
  
    // collect the arguments into the struct placed 
    // on the shared memory segment
    oData->operation = operation;
    oData->usage = usage;
    wcscpy_s(oData->pinName,sizeof(oData->pinName)/sizeof(wchar_t),wsPinName);
    oData->pin1Info = pin1Info;
    oData->pin2Info = pin2Info;
    wcscpy_s(oData->pin1,sizeof(oData->pin1)/sizeof(wchar_t),wsPin1);
    wcscpy_s(oData->pin2,sizeof(oData->pin2)/sizeof(wchar_t),wsPin2);
    
    CallQTServer(DLG_ASK_PINS,csReadableFilePath.c_str());
    lRet = oData->returnValue;

    if(lRet == DLG_OK) {
      wcscpy_s(wsPin1,ulPin1BufferLen,oData->pin1);
      wcscpy_s(wsPin2,ulPin2BufferLen,oData->pin2);
    }

    // detach from the segment
    oShMemory.Detach(oData);

    // delete the random file
    DeleteFile(csReadableFilePath.c_str());
  } catch (...) {
    // detach from the segment
    oShMemory.Detach(oData);

    // delete the random file
    DeleteFile(csReadableFilePath.c_str());

    return DLG_ERR;
  }
  return lRet;
}*/







/*done
DLGS_EXPORT DlgRet eIDMW::DlgDisplayPinpadInfo(DlgPinOperation operation,
      const wchar_t *wsReader, DlgPinUsage usage, const wchar_t *wsPinName,
      const wchar_t *wsMessage,
      void **puserNotificationRef)
{
    DlgRet lRet = DLG_CANCEL;
	CFArrayRef           titlesArray;
	//CFArrayRef           valuesArray;
	//DlgAskAccessArguments *oData = NULL;
	SharedMem oShMemory;
	std::string csReadableFilePath;
    const wchar_t * Title;
	//https://developer.apple.com/library/mac/documentation/CoreFoundation/Reference/CFUserNotificationRef/Reference/reference.html#//apple_ref/c/data/kCFUserNotificationAlertHeaderKey
	CFUserNotificationRef userNotificationRef;
	SInt32 error = 0;
    char datachar[256];
    char titlechar[256];
    CFOptionFlags optionFlags;
	
	const void* keys[] = {kCFUserNotificationAlertHeaderKey,
		kCFUserNotificationAlertMessageKey};//,
		//kCFUserNotificationTextFieldTitlesKey};//,
		//kCFUserNotificationTextFieldValuesKey,
		//kCFUserNotificationProgressIndicatorValueKey,
		//kCFUserNotificationDefaultButtonTitleKey,
        //kCFUserNotificationAlternateButtonTitleKey};
	
    if( usage == DLG_PIN_SIGN )
	{
        optionFlags = kCFUserNotificationCautionAlertLevel;
		Title=GETSTRING_DLG(YouAreAboutToMakeALegallyBindingElectronic);//SigningWith);
		//Title+= ": ";
	}
	else
	{
        optionFlags = kCFUserNotificationPlainAlertLevel;
		Title=GETSTRING_DLG(PleaseEnterYourPinOnThePinpadReader);
        
		//Title+= " ";
	}
    
    //CFDataRef
    CFDataRef dataRef = CFDataCreate (
                            kCFAllocatorDefault,
                            (const UInt8 *)wsReader,
                            sizeof(wsReader)
                            );
    
    CFStringRef NotificationAlertMessage = CFStringCreateFromExternalRepresentation (
                                                          kCFAllocatorDefault,
                                                          dataRef,
                                                          kCFStringEncodingUTF16
                                                          );
    
    wcstombs(datachar,wsReader,sizeof(datachar));
    datachar[255]='\0';
    CFStringRef datacharRef = CFStringCreateWithCharacters (
                                              kCFAllocatorDefault,
                                              (const UniChar *)datachar,
                                              (CFIndex)strlen(datachar)
                                              );
    
    CFStringRef datacharRefBytes = CFStringCreateWithBytes (
                                         kCFAllocatorDefault,
                                         (const UInt8 *)datachar,
                                         strlen(datachar),
                                         kCFStringEncodingUTF8,
                                         false
                                         );
    
    wcstombs(titlechar,Title,sizeof(titlechar));
    titlechar[255]='\0';
    CFStringRef titlecharRefBytes = CFStringCreateWithBytes (
                                                            kCFAllocatorDefault,
                                                            (const UInt8 *)titlechar,
                                                            strlen(titlechar),
                                                            kCFStringEncodingUTF8,
                                                            false
                                                            );
    
	//wcscpy_s(oData->message,sizeof(oData->message)/sizeof(wchar_t),wsMessage);
	const void* textFieldTitles[] = {Title};
    
    titlesArray = CFArrayCreate(NULL, textFieldTitles, 1, NULL);
	//valuesArray = CFArrayCreate(NULL, textFieldValues, 3, NULL);
	
	//if (array) {
	//	CFArrayAppendValue(array, CFSTR("0"));
	//	CFArrayAppendValue(array, CFSTR("1"));
	//	CFArrayAppendValue(array, CFSTR("2"));
	//  }
	
	const void* values[] = {titlecharRefBytes,
        datacharRefBytes};//,
        //CFSTR("NotificationTextFieldTitles")};//,
        //CFSTR("NotificationTextFieldValuesKey"),
        //CFSTR("NotificationProgressIndicatorValue"),
        //CFSTR("NotificationDefaultButtonTitle"),
        //CFSTR("NotificationAlternateButtonTitle")};
    */
    /*const void* values[] = {
     Title,
     titlesArray};
     //valuesArray,
     //CFSTR("YES"),
     //CFSTR("CANCEL")};*/
	
/*
	CFDictionaryRef parameters = CFDictionaryCreate(0, keys, values,
													
													sizeof(keys)/sizeof(*keys), NULL,
													
													NULL);
*/
    //CFDictionaryAddValue(dict, kCFUserNotificationAlertHeaderKey, CFSTR("my title"));
    
    //CFDictionaryAddValue(dict, kCFUserNotificationAlternateButtonTitleKey, CFSTR("Cancel"));
    
    
/*	userNotificationRef = CFUserNotificationCreate (kCFAllocatorDefault, //CFAllocatorRef allocator,
													30, //CFTimeInterval timeout,
													optionFlags,//CFOptionFlags flags,
													&error,//SInt32 *error,
													parameters);//CFDictionaryRef dictionary
*/
//	*puserNotificationRef = userNotificationRef;
	//error = CFUserNotificationReceiveResponse (	userNotificationRef,
	//										   10000,//CFTimeInterval timeout,
	//										   &optionFlags);//CFOptionFlags *responseFlags
	//CFUserNotificationCancel
//	CFRelease(titlesArray);
//	CFRelease(parameters);
	//CFRelease(userNotificationRef);
    /*
     */
 //   return lRet;
    
    
    
    /*
  DlgRet lRet = DLG_CANCEL;

  DlgDisplayPinpadInfoArguments* oData;
  SharedMem oShMemory;
  std::string csReadableFilePath;

  try {
	MWLOG(LEV_DEBUG, MOD_DLG,L"  eIDMW::DlgDisplayPinpadInfo called");
    csReadableFilePath = CreateRandomFile();

    oShMemory.Attach(sizeof(DlgDisplayPinpadInfoArguments),csReadableFilePath.c_str(),(void**)&oData);

    // collect the arguments into the struct placed 
    // on the shared memory segment

    oData->operation = operation;
    wcscpy_s(oData->reader,sizeof(oData->reader)/sizeof(wchar_t),wsReader);
    oData->usage = usage;
    wcscpy_s(oData->pinName,sizeof(oData->pinName)/sizeof(wchar_t),wsPinName);
    wcscpy_s(oData->message,sizeof(oData->message)/sizeof(wchar_t),wsMessage);
    oData->infoCollectorIndex = ++dlgPinPadInfoCollectorIndex;

    CallQTServer(DLG_DISPLAY_PINPAD_INFO,csReadableFilePath.c_str());
    lRet = oData->returnValue;

    if (lRet != DLG_OK) {
      throw CMWEXCEPTION(EIDMW_ERR_SYSTEM);
    }

    // for the killing need to store: 
    // - the shared memory area to be released (unique with the filename?)
    // - the child process ID
    // - the handle (because the user will use it)

    DlgRunningProc *ptRunningProc = new DlgRunningProc();
    ptRunningProc->iSharedMemSegmentID = oShMemory.getID();
    ptRunningProc->csRandomFilename = csReadableFilePath;
  
    ptRunningProc->tRunningProcess = oData->tRunningProcess;

    dlgPinPadInfoCollector[dlgPinPadInfoCollectorIndex] = ptRunningProc;

    if( pulHandle )
      *pulHandle = dlgPinPadInfoCollectorIndex;


  } catch(...) {


    return DLG_ERR;
  }
  //return lRet;
  
}*/



/* only for linux, dlgmodal for win and apple */
/*
DLGS_EXPORT DlgRet eIDMW::DlgAskAccess(
  const wchar_t *wsAppPath, const wchar_t *wsReaderName,
	DlgPFOperation ulOperation, int *piForAllOperations)
{

  DlgRet lRet = DLG_CANCEL;

  //DlgAskAccessArguments *oData = NULL;
  SharedMem oShMemory;
  std::string csReadableFilePath;

	CFUserNotificationRef userNotificationRef;
	SInt32 error = 0;
	const void* keys[] = {kCFUserNotificationAlertHeaderKey,
		
		kCFUserNotificationProgressIndicatorValueKey,
		
		kCFUserNotificationDefaultButtonTitleKey};
	
	const void* values[] = {CFSTR("Progress"),
		
		kCFBooleanTrue,
		
		CFSTR("Cancel")};
	CFOptionFlags optionFlags;
	*?
	CFDictionaryRef parameters = CFDictionaryCreate(0, keys, values,
													
													sizeof(keys)/sizeof(*keys), &kCFTypeDictionaryKeyCallBacks,
													
													&kCFTypeDictionaryValueCallBacks);
	CFDictionaryRef parameters = CFDictionaryCreate(0, keys, values,
													
													sizeof(keys)/sizeof(*keys), NULL,
													
													NULL);
	
	userNotificationRef = CFUserNotificationCreate (NULL, //CFAllocatorRef allocator,
													10000, //CFTimeInterval timeout,
													kCFUserNotificationCautionAlertLevel,//CFOptionFlags flags,
													&error,//SInt32 *error,
													parameters);//CFDictionaryRef dictionary
							
	
	error = CFUserNotificationReceiveResponse (	userNotificationRef,
												10000,//CFTimeInterval timeout,
												&optionFlags);//CFOptionFlags *responseFlags
													
												
	
	
	

  try {
    csReadableFilePath = CreateRandomFile();

    // attach to the segment and get a pointer
    oShMemory.Attach( sizeof(DlgAskAccessArguments), csReadableFilePath.c_str(),(void **) &oData);

    wcscpy_s(oData->appPath,sizeof(oData->appPath)/sizeof(wchar_t),wsAppPath);
    wcscpy_s(oData->readerName,sizeof(oData->readerName)/sizeof(wchar_t),wsReaderName);
    oData->operation = ulOperation;
    oData->forAllOperations = *piForAllOperations;

    CallQTServer(DLG_ASK_ACCESS,csReadableFilePath.c_str());
    lRet = oData->returnValue;
    
    if(lRet == DLG_OK) *piForAllOperations = oData->forAllOperations;

    // detach from the segment
    oShMemory.Detach(oData);

    // delete the random file
    DeleteFile(csReadableFilePath.c_str());
  } catch (...) {
    // detach from the segment
    oShMemory.Detach(oData);

    // delete the random file
    DeleteFile(csReadableFilePath.c_str());
    return DLG_ERR;
  }
  return lRet;

}

{
	DlgRet lRet = DLG_CANCEL;
	CFArrayRef           titlesArray;
	//CFArrayRef           valuesArray;
	//DlgAskAccessArguments *oData = NULL;
	SharedMem oShMemory;
	std::string csReadableFilePath;
	
	CFUserNotificationRef userNotificationRef;
	SInt32 error = 0;
	const void* keys[] = {kCFUserNotificationAlertHeaderKey,
		kCFUserNotificationAlertMessageKey,
		kCFUserNotificationTextFieldTitlesKey,
		//kCFUserNotificationTextFieldValuesKey,
		//kCFUserNotificationAlertMessageKey,
		kCFUserNotificationAlternateButtonTitleKey,
		kCFUserNotificationDefaultButtonTitleKey};
	
	
	const void* textFieldTitles[] = {CFSTR("old PIN"),CFSTR("new PIN"),CFSTR("new PIN")};
	//const void* textFieldValues[] = {CFSTR("0"),CFSTR("1"),CFSTR("2")};
	
	
    	DlgRet lRet = DLG_CANCEL;
     
     DlgDisplayModalArguments* oData;
     SharedMem oShMemory;
     std::string csReadableFilePath;
     
     try {
     csReadableFilePath = CreateRandomFile();
     
     oShMemory.Attach( sizeof(DlgDisplayModalArguments), csReadableFilePath.c_str(),(void **) &oData);
     
     oData->icon = icon;
     if(wcslen(csMesg)==0)
     {
     std::wstring translatedMessage(CLang::GetMessageFromID(messageID));
     wcscpy_s(oData->mesg,sizeof(oData->mesg)/sizeof(wchar_t),translatedMessage.c_str());
     }
     else
     {
     wcscpy_s(oData->mesg,sizeof(oData->mesg)/sizeof(wchar_t),csMesg);
     }
     oData->buttons = ulButtons;
     oData->EnterButton = ulEnterButton;
     oData->CancelButton = ulCancelButton;
     
     CallQTServer(DLG_DISPLAY_MODAL,csReadableFilePath.c_str());
     lRet = oData->returnValue;
     
     // detach from the segment
     oShMemory.Detach(oData);
     
     // delete the random file
     DeleteFile(csReadableFilePath.c_str());
     
     } catch(...) {
     // detach from the segment
     oShMemory.Detach(oData);
     
     // delete the random file
     DeleteFile(csReadableFilePath.c_str());
     
     return DLG_ERR;
     
     }
 
	
	
	//array = CFArrayCreate(NULL, 0, &kCFTypeArrayCallBacks);
	
	titlesArray = CFArrayCreate(NULL, textFieldTitles, 3, NULL);
	//valuesArray = CFArrayCreate(NULL, textFieldValues, 3, NULL);
	
	//if (array) {
	//	CFArrayAppendValue(array, CFSTR("0"));
	//	CFArrayAppendValue(array, CFSTR("1"));
	//	CFArrayAppendValue(array, CFSTR("2"));
	//  }
	
	const void* values[] = {CFSTR("Warning"),
		CFSTR("The following application is trying to read data on the eID card\nDo you allow that?\n\n PATH of the application: "),
		//CFSTR("please enter PIN"),//kCFBooleanTrue,
		titlesArray,
		//valuesArray,
		CFSTR("YES"),
		CFSTR("NO")};
	
	CFOptionFlags optionFlags = kCFUserNotificationCautionAlertLevel | CFUserNotificationSecureTextField (0);
	
	CFDictionaryRef parameters = CFDictionaryCreate(0, keys, values,
													
													sizeof(keys)/sizeof(*keys), NULL,
													
													NULL);
	
	userNotificationRef = CFUserNotificationCreate (NULL, //CFAllocatorRef allocator,
													10000, //CFTimeInterval timeout,
													optionFlags,//CFOptionFlags flags,
													&error,//SInt32 *error,
													parameters);//CFDictionaryRef dictionary
	
	
	error = CFUserNotificationReceiveResponse (	userNotificationRef,
											   10000,//CFTimeInterval timeout,
											   &optionFlags);//CFOptionFlags *responseFlags
	
	CFRelease(titlesArray);
	CFRelease(parameters);
	CFRelease(userNotificationRef);

 
    return lRet;
} */
