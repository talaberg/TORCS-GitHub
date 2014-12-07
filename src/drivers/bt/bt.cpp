/***************************************************************************

    file                 : bt.cpp
    created              : Wed Jan 8 18:31:16 CET 2003
    copyright            : (C) 2002-2004 Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: bt.cpp,v 1.5.2.2 2008/11/09 17:50:19 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

	#ifdef _WIN32
	#include <windows.h>
	#endif

	#include <stdio.h>
	#include <stdlib.h>
	#include <math.h>

	#include <tgf.h>
	#include <track.h>
	#include <car.h>
	#include <raceman.h>
	#include <robottools.h>
	#include <robot.h>
	#include <time.h>
	#include "driver.h"

	#include <string.h>


//===================================================================

	#include <iostream>
	#include "PracticalSocket.h"
	
	void sfloat2fixstring(char* str, float f);

	const int ECHOMAX = 255; // Longest string to echo

    /***************************************************************************/
	const string IPCIM = string("127.0.0.1"); //PXI IP-je, amin a VS Engine fut, mivel IDE KÜLDÖK!! 
	const string PORTSZAM = "8888";					//PORT AHOVA KÜLDÖK!!
	/***************************************************************************/

	const int REC_PORT = 9999;	   //given local PORT to construct UDP socket 
	unsigned short echoServPort;
	#define MAX_STR 80
	#define TMP_STR 10


///////////////////// T H R E A D /////////////////////////////

	#include <tchar.h>

	#define BUF_SIZE 255

	#define ELSOKEREK _wheelSpinVel(1)

	DWORD WINAPI adatKuldThread( LPVOID lpParam );
	void ErrorHandler(LPTSTR lpszFunction);

	#define MAX_THREADS 2

	DWORD   dwThreadIdArray[MAX_THREADS];
	HANDLE  hThreadArray[MAX_THREADS]; 
	
		

///////////////////// E N D  T H R E A D /////////////////////////////


		
///////////////////   VEZETÉSI ÁLLAPOTGÉP  ////////////////////
	float globKormany = 0.0f;
	float globGaz = 0.0f;
	float globFek = 0.0f;
	int globValto = 0;
	tdble globKerekFek[4] = {0.0, 0.0, 0.0, 0.0};
////////////////////////////////////////////////////////////////


	bool globMehet = false;  // Kezdhetem-e a küldést? Van e már adat a globCar-ban?
	bool globKuldes = false;
	tCarElt* globCar = NULL; // Autó adatai
	int valtoHelyzet = 0;
	bool valthat = true;
	bool valtottam = false;
	int elozoValto = 0;
	#define ABS_MINIMAL_SPEED 0.3

//===================================================================


#define NBBOTS 10

static const char* botname[NBBOTS] = {
	"bt 1", "bt 2", "bt 3", "bt 4", "bt 5",
	"bt 6", "bt 7", "bt 8", "bt 9", "bt 10"
};

static const char* botdesc[NBBOTS] = {
	"bt 1", "bt 2", "bt 3", "bt 4", "bt 5",
	"bt 6", "bt 7", "bt 8", "bt 9", "bt 10"
};

static Driver *driver[NBBOTS];

static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s);
static void newRace(int index, tCarElt* car, tSituation *s);
static void drive(int index, tCarElt* car, tSituation *s);
static int pitcmd(int index, tCarElt* car, tSituation *s);
static void shutdown(int index);
static int InitFuncPt(int index, void *pt);
static void endRace(int index, tCarElt *car, tSituation *s);


#pragma comment(lib,"ws2_32.lib")


// Module entry point.
extern "C" int bt(tModInfo *modInfo)
{
	int i;
	
	// Clear all structures.
	memset(modInfo, 0, 10*sizeof(tModInfo));

	for (i = 0; i < NBBOTS; i++) {
		modInfo[i].name    = strdup(botname[i]);	// name of the module (short).
		modInfo[i].desc    = strdup(botdesc[i]);	// Description of the module (can be long).
		modInfo[i].fctInit = InitFuncPt;			// Init function.
		modInfo[i].gfId    = ROB_IDENT;				// Supported framework version.
		modInfo[i].index   = i;						// Indices from 0 to 9.
	}
	return 0;
}



DWORD WINAPI adatFogadThread( LPVOID lpPara )
{
	#define BME_BT_UDP_DATAPOS 8
	HANDLE hStdout;

    TCHAR msgBuf[BUF_SIZE];
    size_t cchStringSize;
  //  DWORD dwChars;

    // Make sure there is a console to receive output results. 
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if( hStdout == INVALID_HANDLE_VALUE )
        return 1;
	
	  //time_t seconds;
	  //seconds = time (NULL);
	 // printf ("%ld \n", seconds);
	//int BmeTimeCounter = 0;
  
	while(1)
	{
		if(globMehet == true)
		{
			/*printf("*******************Thread2***************\n");
			Sleep(30);
			globCar->ctrl.accelCmd = 1.0f ; // 30% a c c e l e r a t o r p e d a
			globCar->ctrl.brakeCmd = 0.0f ; // nobrakes
			*/
			float f;

			printf("******** MOST VESZEK ********\n");

			 try 
			 {
					UDPSocket sock(REC_PORT);         

					/**UDPSocket(unsigned short localPort) throw(SocketException);
						
					   *   Construct a UDP socket with the given local port
					   *   @param localPort local port
					   *   @exception SocketException thrown if unable to create UDP socket
					   */
					  					
  					char echoBuffer[ECHOMAX];         // Buffer for echo string
					int recvMsgSize;                  // Size of received message
					string sourceAddress;             // Address of datagram source
					unsigned short sourcePort;        // Port of datagram source

					printf("    /-------------------------------------\\\n");
					printf("   *                                       *\n");
					printf("  *\t\t  UDP Szerver\t\t    *\n");
					printf("  *\t   Hallgatozas a %d porton\t    *\n",REC_PORT);
					printf("   *                                       *\n");
					printf("    \\_____________________________________/\n");
					printf("\n");

					globKormany = 0.0;
					globGaz = 0.0;
					globFek = 0.0;

					for (;;) 
					{
						// Block until receive message from a client

						recvMsgSize = sock.recvFrom(echoBuffer, ECHOMAX, sourceAddress, sourcePort);

						char id[6];

						 //strncpy(id,echoBuffer,7);
						 char arr[6];
						 arr[0]=echoBuffer[8];
						 arr[1]=echoBuffer[9];
						 arr[2]=echoBuffer[10];
						 arr[3]=echoBuffer[11];
						 arr[4]=echoBuffer[12];
						 arr[5]=echoBuffer[13];
						 

						f = atof(arr);
					//	printf("Ertek: %f\n", f);

						 if(echoBuffer[5] == '0' && echoBuffer[6] == '1')
						 { // Kormány
							globKormany = -f;
							//printf("KORM: %f\n",f);
						 }
						 else if(echoBuffer[5] == '0' && echoBuffer[6] == '2')
						 { // Gáz
							globGaz = f;
							//printf("GAZ: %f\n",f);
						 }
						 else if(echoBuffer[5] == '0' && echoBuffer[6] == '3')
						 { // Fék
							globFek = f;
							//printf("FEK: %f\n",f);
							for(int k = 0; k < 4; k++)
							{
								for(int i = 0; i < 6; i++)
								 {
									arr[i] = echoBuffer[BME_BT_UDP_DATAPOS + (k+1)*8 + i];
								 }								 
								 globKerekFek[k] = atof(arr);

							 }
								
						 }
						 else if(echoBuffer[5] == '0' && echoBuffer[6] == '4')
						 { // Váltó		
							if((f == 1))
							{ // Fel
								globValto = 1;
								//printf("VALT: %f\n",f);
							}
							else if((f == -1))
							{
								globValto = -1;
								//printf("VALT: %f\n",f);
							}
							else
							{
								globValto = 0;
							}	 
						 }
						  else if(echoBuffer[5] == '0' && echoBuffer[6] == '5')
						  {//SEND or DON't send
							  if((f == 1))
								{ 
									globKuldes = true;
							//		printf("SEND: %f\n",f);
								}
							  else
								{
									globKuldes = false;
							//		printf("SEND: %f\n",f);
								}	 
						  }
						Sleep(2);
					}
			  } 
			 catch (SocketException &e) 
			 {
					cerr << e.what() << endl;	
					exit(1);
			 }
		}
		Sleep(1);
	}
}

DWORD WINAPI adatKuldThread( LPVOID lpParam ) 
{    
	HANDLE hStdout;

    TCHAR msgBuf[BUF_SIZE];
    size_t cchStringSize;
  //  DWORD dwChars;

    // Make sure there is a console to receive output results. 

    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if( hStdout == INVALID_HANDLE_VALUE )
        return 1;		

			int strlength = MAX_STR;

			char* str = new char[MAX_STR];
			char* str2 = new char[TMP_STR];

			UDPSocket sock; //Construct a UDP socket

			while(1)
			{
				
				if(globMehet == true)
				{ // Driver már 1x lefutott-e, hogy a globCar ne legyen NULL
				
						if(globCar->_enginerpm !=0)
						{
							strcpy(str,"*010001$");
							sprintf(str2, "%.2f",globCar->_enginerpm);
							strcat(str,str2);	
							sock.sendTo(str, 16, IPCIM, echoServPort); //SEND *010001$
							Sleep(2);
							//printf("engine rpm: %s\n",str);
						 //printf("Elkuldtem es varok 100ms-t\n");
						}
						//--------------------------------------------------------------------
						//*140001$ => Slip value							
						//ABS functionality
						if(globCar->_speed_x != 0)
						{								
							float slip = 0.0;
							int i;					
							
							strcpy(str,"*140001$");
							
							if((globCar->_speed_x < 5) && globCar->_wheelSpinVel(1) < 5 )	slip = 0;
							else	slip = 1-(globCar->_wheelSpinVel(1)/(globCar->_speed_x/globCar->_wheelRadius(1)));

							if(slip < 0)	slip = 0;
							if(slip > 1)	slip = 1;
					
							sprintf(str2, "%.5f",(slip) );
							strcat(str,str2);
							sock.sendTo(str, 24, IPCIM, echoServPort); //SEND *140001$
							Sleep(2);
							//printf("slip: %s\n",str);						
						}
						//--------------------------------------------------------------------
						//SPIN VELOCITY						
						strcpy(str,"*150001$");						
						sfloat2fixstring(str2,globCar->_wheelSpinVel(0));
						strcat(str,str2);						
						sfloat2fixstring(str2,globCar->_wheelSpinVel(1));
						strcat(str,str2);
						sfloat2fixstring(str2,globCar->_wheelSpinVel(2));
						strcat(str,str2);
						sfloat2fixstring(str2,globCar->_wheelSpinVel(3));
						strcat(str,str2);
						sock.sendTo(str, 40, IPCIM, echoServPort); //SEND *150001$
						Sleep(2);
						//printf("kuld_Wheel_spinvel: %s\n",str);
						//--------------------------------------------------------------------
						//SLIP SIDE
						strcpy(str,"*150002$");
						sfloat2fixstring(str2,globCar->_wheelSlipSide(0));
						strcat(str,str2);	
						sfloat2fixstring(str2,globCar->_wheelSlipSide(1));
						strcat(str,str2);
						sfloat2fixstring(str2,globCar->_wheelSlipSide(2));
						strcat(str,str2);
						sfloat2fixstring(str2,globCar->_wheelSlipSide(3));
						strcat(str,str2);	
						sock.sendTo(str, 40, IPCIM, echoServPort); //SEND *150002$
						Sleep(2);
						//printf("kuld__wheel_SlipSide: %s\n",str);
						//--------------------------------------------------------------------
						//SLIP ACCEL
						strcpy(str,"*150003$");
						sfloat2fixstring(str2,globCar->_wheelSlipAccel(0));
						strcat(str,str2);
						sfloat2fixstring(str2,globCar->_wheelSlipAccel(1));
						strcat(str,str2);
						sfloat2fixstring(str2,globCar->_wheelSlipAccel(2));
						strcat(str,str2);
						sfloat2fixstring(str2,globCar->_wheelSlipAccel(3));
						strcat(str,str2);
						sock.sendTo(str, 40, IPCIM, echoServPort); //SEND *150003$
						Sleep(2);
						//printf("kuld_Wheel_SlipAccel: %s\n",str);	


					//These messages are only sent when a request arrives.
					if (globKuldes == true )
					{
						//FUEL => *010004$ 
						strcpy(str,"*010004$");
						sfloat2fixstring(str2,globCar->_fuelInstant);
						strcat(str,str2);
						sfloat2fixstring(str2,globCar->_fuel);
						strcat(str,str2);
						sock.sendTo(str, 24, IPCIM, echoServPort); //SEND *010002$
						Sleep(2);
						//printf("fuel: %s\n",str);	
						//--------------------------------------------------------------------
						//PUBLIC INFO on the cars => *020010$ 
						strcpy(str,"*020010$");
						//POS XYZ
						sfloat2fixstring(str2,globCar->_pos_X);							
						strcat(str,str2);	
						sfloat2fixstring(str2,globCar->_pos_Y);
						strcat(str,str2);
						sfloat2fixstring(str2,globCar->_pos_Z);
						strcat(str,str2);
						//SPEED XYZ
						sfloat2fixstring(str2,globCar->_speed_x);
						strcat(str,str2);
						sfloat2fixstring(str2,globCar->_speed_y);
						strcat(str,str2);
						sfloat2fixstring(str2,globCar->_speed_z);
						strcat(str,str2);
						//ACCEL XYZ
						sfloat2fixstring(str2,globCar->_accel_x);
						strcat(str,str2);
						sfloat2fixstring(str2,globCar->_accel_y);
						strcat(str,str2);
						sfloat2fixstring(str2,globCar->_accel_z);
						strcat(str,str2);	
						sock.sendTo(str, 80, IPCIM, echoServPort); //SEND *020010$
						Sleep(2);
						//printf("kuld_car_info_X: %s\n",str);

					}
					
				} // globMehet if vége
			}
    return 0; 
} 

static void initUDP()
{
	echoServPort = Socket::resolveService(PORTSZAM,"udp");


  /**  static unsigned short resolveService(const string &service, const string &protocol = "tcp");

   *   Resolve the specified service for the specified protocol to the corresponding port number in host byte order.
   *   
   *   @param service service to resolve (e.g., "http")
   *   @param protocol protocol of service to resolve.  Default is "tcp".
   */
  
}

// Module interface initialization.
static int InitFuncPt(int index, void *pt)
{
	tRobotItf *itf = (tRobotItf *)pt;

	// Create robot instance for index.
	driver[index] = new Driver(index);
	itf->rbNewTrack = initTrack;	// Give the robot the track view called.
	itf->rbNewRace  = newRace;		// Start a new race.
	itf->rbDrive    = drive;		// Drive during race.
	itf->rbPitCmd   = pitcmd;		// Pit commands.
	itf->rbEndRace  = endRace;		// End of the current race.
	itf->rbShutdown = shutdown;		// Called before the module is unloaded.
	itf->index      = index;		// Index used if multiple interfaces.

	initUDP();

	///////////////////////////////////////////////////////////
		hThreadArray[0] = CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            adatKuldThread,			// thread function name
            0,						// argument to thread function 
            0,                      // use default creation flags 
            &dwThreadIdArray[0]);   // returns the thread identifier 
		
		hThreadArray[1] = CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            adatFogadThread,		// thread function name
            0,						// argument to thread function 
            0,                      // use default creation flags 
            &dwThreadIdArray[1]);   // returns the thread identifier 

        // Check the return value for success.
        // If CreateThread fails, terminate execution. 
        // This will automatically clean up threads and memory. 

        if (hThreadArray[0] == NULL) 
        {
           ErrorHandler(TEXT("CreateThread"));
           ExitProcess(3);
        }

		if (hThreadArray[1] == NULL) 
        {
           ErrorHandler(TEXT("CreateThread"));
           ExitProcess(3);
        }

		printf("Megcsináltam a Threadot!");
	//////////////////////////////////////////////////////////

	return 0;
}

// Called for every track change or new race.
static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s)
{
	driver[index]->initTrack(track, carHandle, carParmHandle, s);
}

// Start a new race.
static void newRace(int index, tCarElt* car, tSituation *s)
{
	driver[index]->newRace(car, s);
}

// Drive during race.
static void drive(int index, tCarElt* car, tSituation *s)
{
	memset(&car->ctrl,0,sizeof( tCarCtrl ) ) ;

//	if(globMehet == false){
	//	printf("*********************************************************************************************************************************");
		globCar = car;
	//	car->ctrl.gear = 1;
		globMehet = true;
	//}

//	car->_speed_x != 0
	car->ctrl.steer = globKormany/(((car->_speed_x)/200)+1.4);
	car->ctrl.accelCmd = globGaz;
	car->ctrl.brakeCmd = globFek;

	car->ctrl.BmeAbsbrakeCmd[FRNT_RGT] = globKerekFek[FRNT_RGT];//Egyes kerékfék értékek átadása
	car->ctrl.BmeAbsbrakeCmd[FRNT_LFT] = globKerekFek[FRNT_LFT];
	car->ctrl.BmeAbsbrakeCmd[REAR_RGT] = globKerekFek[REAR_RGT];
	car->ctrl.BmeAbsbrakeCmd[REAR_LFT] = globKerekFek[REAR_LFT];
	car->ctrl.BmeAbsEnable = 1;

	if(elozoValto != globValto)
	{
		valtoHelyzet += globValto;
		
		if(valtoHelyzet > 6 || valtoHelyzet < -1)
		{
			valtoHelyzet -= globValto;
		}

		car->ctrl.gear = valtoHelyzet; 
		valtottam = false;
	}
	else
	{
		car->ctrl.gear = valtoHelyzet;
	}
	elozoValto = globValto;

}


// Pitstop callback.
static int pitcmd(int index, tCarElt* car, tSituation *s)
{
	return driver[index]->pitCommand(s);
}


// End of the current race.
static void endRace(int index, tCarElt *car, tSituation *s)
{
	driver[index]->endRace(s);
}


// Called before the module is unloaded.
static void shutdown(int index)
{
	CloseHandle(hThreadArray[0]);
	CloseHandle(hThreadArray[1]);
	delete driver[index];
}

void ErrorHandler(LPTSTR lpszFunction) 
{ 
    // Retrieve the system error message for the last-error code.

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message.

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR) lpMsgBuf) + lstrlen((LPCTSTR) lpszFunction) + 40) * sizeof(TCHAR)); 
   

    // Free error-handling buffer allocations.

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}
//Creates a srting with a format:
//SXXX.XXX, where S is the sign
void sfloat2fixstring(char* str, float f)
{
	int k = 0;
	sprintf(str,"%+3.3f",f);
	if(f < 0 ) f = -f;
	if( (f < 100) && (f >= 10)) k = 1;
	if(f <= 10) k = 2;
	if(k)
	{
		for(int i = TMP_STR - 1;i > 1; i--)
		{
			str[i] = str[i-k];
		}
		str[1] = '0';
		if(k == 2)str[2] = '0';
	}
}