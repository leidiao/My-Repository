/**********************************************************************************************************************
File: user_app2.c                                                                

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app2 as a template:
 1. Copy both user_app2.c and user_app2.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "user_app2" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "UserApp2" with "YourNewTaskName"
 6. Use ctrl-h to find and replace all instances of "USER_APP2" with "YOUR_NEW_TASK_NAME"
 7. Add a call to YourNewTaskNameInitialize() in the init section of main
 8. Add a call to YourNewTaskNameRunActiveState() in the Super Loop section of main
 9. Update yournewtaskname.h per the instructions at the top of yournewtaskname.h
10. Delete this text (between the dashed lines) and update the Description below to describe your task
----------------------------------------------------------------------------------------------------------------------

Description:
This is a user_app2.c file template 

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
void UserApp2Initialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void UserApp2RunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp2Flags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */

extern volatile bool G_SpeedFlag;                              /*flag to change speed*/
extern volatile bool G_AutoFlag;                               /*flag to auto*/
extern volatile bool G_SleepFlag;                              /*flag to sleep*/

extern  u8 G_au8WindSpeedArrary3[2];          /*arrary to change the wind*/ 
extern  u8 G_au8AutoArrary4[2];               /*arrary to auto close*/
extern  u8 G_au8SleepArrary5[2];              /*arrary to sleep*/
/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp2_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp2_StateMachine;            /* The state machine function pointer */
static u32 UserApp2_u32Timeout;                      /* Timeout counter used across states */


 


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: UserApp2Initialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - 
*/
void UserApp2Initialize(void)
{ 
  
  /* If good initialization, set state to Idle */
  if( 1 )
  {
    UserApp2_StateMachine = UserApp2SM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp2_StateMachine = UserApp2SM_FailedInit;
  }
} /* end UserApp2Initialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function UserApp2RunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void UserApp2RunActiveState(void)
{
  UserApp2_StateMachine();

} /* end UserApp2RunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a message to be queued */
static void UserApp2SM_Idle(void)
{
  static LedNumberType LedColourChar[]={WHITE,PURPLE,BLUE,CYAN,GREEN,YELLOW,ORANGE,RED};
  static u8 au8TestMessage[] = {0x66, 0, 0, 0, 0, 0, 0, 0};
  static LedRateType   DutyCycle=0;
  static LedRateType   u8DutyCycle=0;
  static u16 u16count=500;
  static u32 u32TimeCount;

  
  /*Windspeed*/
  if(G_SpeedFlag)
  {
    for(u8 j=0;j<4;j++)
    {
      LedOn(LedColourChar[j]);
    }
    /*L*/
    if(G_au8WindSpeedArrary3[0]==0x01)
    {
      for(u8 i=0;i<4;i++)
      {
        LedBlink(LedColourChar[i], LED_2HZ);
      }
      G_SpeedFlag=FALSE;
    }/*end 0x01*/
    /*M*/
    if(G_au8WindSpeedArrary3[0]==0x02)
    {
      for(u8 i=0;i<4;i++)
      {
        LedBlink(LedColourChar[i], LED_4HZ);
      }
      G_SpeedFlag=FALSE;
    }/*end 0x02*/
    /*H*/
    if(G_au8WindSpeedArrary3[0]==0x03)
    {
      for(u8 i=0;i<4;i++)
      {
        LedBlink(LedColourChar[i], LED_8HZ);
      }
      G_SpeedFlag=FALSE;
    }/*end 0x03*/
  }/*end speedflag*/
  
  
  /*Sleep*/
  if(G_au8SleepArrary5[0]==0x00)
  {
   if(G_SleepFlag)
   {
     for(u8 j=0;j<4;j++)
     {
       LedOff(LedColourChar[j]);
     }
     
     
     for(u8 j=5;j<8;j++)
     {
       LedOn(LedColourChar[j]);
       G_SleepFlag=FALSE;
     }
   }/*end G_flag*/
   u16count--;
   if(u16count==0)
   {
     u16count=500;
     for(u8 i=5;i<8;i++)
     {
        LedPWM(LedColourChar[i],DutyCycle);
     }
     DutyCycle++;
     if(DutyCycle==20)
     {
       DutyCycle=0;
     }
   }
 }
 if(G_au8SleepArrary5[0]==0xFF)
 {
   if(G_SleepFlag)
   {
     for(u8 j=5;j<8;j++)
     {
       LedOff(LedColourChar[j]);
       G_SleepFlag=FALSE;
     }
   }
   
 }
 /*Auto close*/
 if(G_AutoFlag)
 {
   u32TimeCount=(G_au8AutoArrary4[0])*60000;
   u32TimeCount--;
   if(u32TimeCount==0)
   {
     AntQueueBroadcastMessage(au8TestMessage);
     
   }
 }
    
} /* end UserApp2SM_Idle() */

 
/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp2SM_Error(void)          
{
  
} /* end UserApp2SM_Error() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* State to sit in if init failed */
static void UserApp2SM_FailedInit(void)          
{
    
} /* end UserApp2SM_FailedInit() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/