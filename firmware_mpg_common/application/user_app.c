/**********************************************************************************************************************
File: user_app.c                                                                

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app as a template:
 1. Copy both user_app.c and user_app.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "user_app" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "UserApp" with "YourNewTaskName"
 6. Use ctrl-h to find and replace all instances of "USER_APP" with "YOUR_NEW_TASK_NAME"
 7. Add a call to YourNewTaskNameInitialize() in the init section of main
 8. Add a call to YourNewTaskNameRunActiveState() in the Super Loop section of main
 9. Update yournewtaskname.h per the instructions at the top of yournewtaskname.h
10. Delete this text (between the dashed lines) and update the Description below to describe your task
----------------------------------------------------------------------------------------------------------------------

Description:
This is a user_app.c file template 

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
void UserAppInitialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void UserAppRunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserAppFlags;                       /* Global state flags */

volatile bool G_SpeedFlag;                             /*flag to change speed*/
volatile bool G_AutoFlag;                              /*flag to auto*/
volatile bool G_SleepFlag;                              /*flag to sleep*/

volatile  u8   G_au8WindSpeedArrary3[2]={0,'\0'};          /*arrary to change the wind*/ 
volatile  u8   G_au8AutoArrary4[2]={0,'\0'};               /*arrary to auto close*/
volatile  u8   G_au8SleepArrary5[2]={0,'\0'}; 
/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */

extern volatile u32 G_OutsideTemperature[55];

extern AntSetupDataType G_stAntSetupData;                         /* From ant.c */
extern u32 G_u32AntApiCurrentDataTimeStamp;                       /* From ant_api.c */
extern AntApplicationMessageType G_eAntApiCurrentMessageClass;    /* From ant_api.c */
extern u8 G_au8AntApiCurrentData[ANT_APPLICATION_MESSAGE_BYTES];  /* From ant_api.c */



/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp_StateMachine;            /* The state machine function pointer */
static u32 UserApp_u32Timeout;                      /* Timeout counter used across states */


static bool ControlLcdFlag1;




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
Function: UserAppInitialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - 
*/
void UserAppInitialize(void)
{ 
  /* All discrete LEDs to off */
  LedOff(WHITE);
  LedOff(PURPLE);
  LedOff(BLUE);
  LedOff(CYAN);
  LedOff(GREEN);
  LedOff(YELLOW);
  LedOff(ORANGE);
  LedOff(RED);
  
  /* Backlight to dark */  
  LedOff(LCD_RED);
  LedOff(LCD_GREEN);
  LedOff(LCD_BLUE);
  LCDCommand(LCD_CLEAR_CMD);
 /* Configure ANT for this application */
  G_stAntSetupData.AntChannel          = ANT_CHANNEL_USERAPP;
  G_stAntSetupData.AntSerialLo         = ANT_SERIAL_LO_USERAPP;
  G_stAntSetupData.AntSerialHi         = ANT_SERIAL_HI_USERAPP;
  G_stAntSetupData.AntDeviceType       = ANT_DEVICE_TYPE_USERAPP;
  G_stAntSetupData.AntTransmissionType = ANT_TRANSMISSION_TYPE_USERAPP;
  G_stAntSetupData.AntChannelPeriodLo  = ANT_CHANNEL_PERIOD_LO_USERAPP;
  G_stAntSetupData.AntChannelPeriodHi  = ANT_CHANNEL_PERIOD_HI_USERAPP;
  G_stAntSetupData.AntFrequency        = ANT_FREQUENCY_USERAPP;
  G_stAntSetupData.AntTxPower          = ANT_TX_POWER_USERAPP;
  
  /* If good initialization, set state to Idle */
  if( AntChannelConfig(ANT_SLAVE) )
  {
    UserApp_StateMachine = UserAppSM_Idle;
  }
  else
  {
    UserApp_StateMachine = UserAppSM_Error;
  }

} /* end UserAppInitialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function UserAppRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void UserAppRunActiveState(void)
{
  UserApp_StateMachine();

} /* end UserAppRunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a message to be queued */
static void UserAppSM_Idle(void)
{
  
  /* Look for BUTTON 0 to open channel */
  if(WasButtonPressed(BUTTON0))
  {
    /* Got the button, so complete one-time actions before next state */
    ButtonAcknowledge(BUTTON0); 
    /*Prepare for Opening Lcd*/
    ControlLcdFlag1=FALSE;
    /* Queue open channel and change LED0 from yellow to blinking green to indicate channel is opening */
    AntOpenChannel();
    /* Set timer and advance states */
    UserApp_u32Timeout = G_u32SystemTime1ms;
    UserApp_StateMachine = UserAppSM_WaitChannelOpen;
  }
    
} /* end UserAppSM_Idle() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserAppSM_WaitChannelOpen(void)         
{
  /* Monitor the channel status to check if channel is opened */
  if(AntRadioStatus() == ANT_OPEN)
  {

    UserApp_StateMachine = UserAppSM_ChannelOpen;
  }
  
  /* Check for timeout */
  if( IsTimeUp(&UserApp_u32Timeout, TIMEOUT_VALUE) )
  {
    AntCloseChannel();
    UserApp_StateMachine = UserAppSM_Idle;
  }
} /* end static void UserAppSM_WaitChannelOpen(void) */
     


/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserAppSM_ChannelOpen(void)
{
  static u16 TimeCount=500;
  static u32 u32InsideCount=InsideTemperatureTime;
  static u8 u8LastState = 0xff;
  static u8 au8SeparateArrary1[3]={0,0,'\0'};
  static u8 au8SeparateArrary2[3]={0,0,'\0'};
  
  static u8 au8DataContent1[] = "xxxxxxxxxxxxxxxx";
  static u8 au8DataContent2[] = "xxxxxxxxxxxxxxxx";
  static u8 au8LastAntData[ANT_APPLICATION_MESSAGE_BYTES] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  static u8 u8String00[]="Func:Cool ";
  static u8 u8String01[]="Func:Heat ";
  static u8 u8String02[]="Func:Com  ";
  static u8 u8String03[]="Func:Are  ";
  static u8 u8String04[]="Func:Aer  ";  
  static u8 u8String1[]="Tem:";
  static u8 u8String20[]="Speed:L ";
  static u8 u8String21[]="Speed:M ";
  static u8 u8String22[]="Speed:H ";
  static u8 u8String3[]="AutoTime: ";
  static u8 u8String4[]="Sleep  ";
  static bool ControlLcdFlag=TRUE;
  
 
  if(ControlLcdFlag)
  {
    if(TimeCount)
    {
      LedOn(LCD_RED);
      LedOn(LCD_GREEN);
      LedOn(LCD_BLUE);
      PWMAudioSetFrequency(BUZZER1, 500);
      PWMAudioOn(BUZZER1);
      TimeCount--;
    }
    else
   {
     LedOff(LCD_RED);
     LedOff(LCD_GREEN);
     LedOff(LCD_BLUE);
     PWMAudioOff(BUZZER1);
     ControlLcdFlag=FALSE;
   }
  }
    

 
  /* Check for BUTTON0 to close channel */
  if(WasButtonPressed(BUTTON0))
  {
    /* Got the button, so complete one-time actions before next state */
    ButtonAcknowledge(BUTTON0);
    /*Close Lcd*/
    ControlLcdFlag1=TRUE;
    /* Queue close channel, initialize the u8LastState variable and change LED to blinking green to indicate channel is closing */
    AntCloseChannel();
    u8LastState = 0xff;   
    /* Set timer and advance states */
    UserApp_u32Timeout = G_u32SystemTime1ms;
    UserApp_StateMachine = UserAppSM_WaitChannelClose;
  } /* end if(WasButtonPressed(BUTTON0)) */
  
  /* A slave channel can close on its own, so explicitly check channel status */  
  /* Check for timeout */
  if(AntRadioStatus() != ANT_OPEN)
  {
    u8LastState = 0xff;   
    UserApp_u32Timeout = G_u32SystemTime1ms;
    
  } /* if(AntRadioStatus() != ANT_OPEN) */

  /* Always check for ANT messages */
  if( AntReadData() )
  {
     /* New data message: check what it is */
    if(G_eAntApiCurrentMessageClass == ANT_DATA)
    {
      /* Check if the new data is the same as the old data and update as we go */
      /*Sleep*/
      G_au8SleepArrary5[0]=G_au8AntApiCurrentData[5];
      
      for(u8 i = 0; i < ANT_APPLICATION_MESSAGE_BYTES; i++)
      {
        if(G_au8AntApiCurrentData[i] != au8LastAntData[i])
        {
          au8LastAntData[i] = G_au8AntApiCurrentData[i];
          
           if(G_au8AntApiCurrentData[0]==0x00)
           {
                /* Backlight to white */  
                LedOn(LCD_RED);
                LedOn(LCD_GREEN);
                LedOn(LCD_BLUE);
             /*The function choice*/
             if(i==1)
             {
                switch(G_au8AntApiCurrentData[1])
                {
                  case 0x00:LCDClearChars(LINE1_START_ADDR,9);
                            LCDMessage(LINE1_START_ADDR,u8String00);
                            break;
                  case 0x01:LCDClearChars(LINE1_START_ADDR,9);
                            LCDMessage(LINE1_START_ADDR,u8String01);
                            break;
                  case 0x02:LCDClearChars(LINE1_START_ADDR,9);
                            LCDMessage(LINE1_START_ADDR,u8String02);
                            break;
                  case 0x03:LCDClearChars(LINE1_START_ADDR,9);
                            LCDMessage(LINE1_START_ADDR,u8String03);
                            break;
                  case 0x04:LCDClearChars(LINE1_START_ADDR,9);
                            LCDMessage(LINE1_START_ADDR,u8String04);
                            break;
                }
             }
             /*The temperature be set*/
             if(i==2)
             { 
               au8DataContent1[0] = (G_au8AntApiCurrentData[2]/10+48);
               au8DataContent1[1] = (G_au8AntApiCurrentData[2]%10+48);
               au8SeparateArrary1[0]=au8DataContent1[0];
               au8SeparateArrary1[1]=au8DataContent1[1];
               
               LCDClearChars(LINE1_START_ADDR+11,5);
               LCDMessage(LINE1_START_ADDR+10,u8String1);
               LCDMessage(LINE1_START_ADDR+14,au8SeparateArrary1);
             }
             /*wind speed*/
             if(i==3)
             {
               G_SpeedFlag=TRUE;
               /*store G_au8AntApiCurrentData[3]*/
               G_au8WindSpeedArrary3[0]=G_au8AntApiCurrentData[3];
               
               switch(G_au8AntApiCurrentData[3])
                {
                  case 0x01:LCDClearChars(LINE2_START_ADDR,7);
                            LCDMessage(LINE2_START_ADDR,u8String20);
                            break;
                  case 0x02:LCDClearChars(LINE2_START_ADDR,7);
                            LCDMessage(LINE2_START_ADDR,u8String21);
                            break;
                  case 0x03:LCDClearChars(LINE2_START_ADDR,7);
                            LCDMessage(LINE2_START_ADDR,u8String22);
                            break;
                }
             }/*end i==3*/
             /*Auto close*/
             if(i==4)
             {
               G_AutoFlag=TRUE;
               /*store G_au8AntApiCurrentData[4]*/
               G_au8AutoArrary4[0]=G_au8AntApiCurrentData[4];
               
               au8DataContent2[0] = (G_au8AntApiCurrentData[4]/10+48);
               au8DataContent2[1] = (G_au8AntApiCurrentData[4]%10+48);
               au8SeparateArrary2[0]=au8DataContent2[0];
               au8SeparateArrary2[1]=au8DataContent2[1];
               
               
               LCDClearChars(LINE2_START_ADDR+8,11);
               LCDMessage(LINE2_START_ADDR+8,u8String3);
               LCDMessage(LINE2_START_ADDR+17,au8SeparateArrary2);
             }
             if(i==5)
             {

               G_SleepFlag=TRUE;
             }
           }/*end open*/
           if(G_au8AntApiCurrentData[0]==0xFF)
           {
             /*Close AutoClose*/
             G_AutoFlag=FALSE;
             
             LedOff(WHITE);
             LedOff(PURPLE);
             LedOff(BLUE);
             LedOff(CYAN);
             LedOff(GREEN);
             LedOff(YELLOW);
             LedOff(ORANGE);
             LedOff(RED);
             
             LedOff(LCD_RED);
             LedOff(LCD_GREEN);
             LedOff(LCD_BLUE);
             LCDCommand(LCD_CLEAR_CMD);
           }/*end close*/
        }/*end compare*/
      }/*end loop*/
      



    } /* end if(G_eAntApiCurrentMessageClass == ANT_DATA) */

    else if(G_eAntApiCurrentMessageClass == ANT_TICK)
    {

    } /* end else if(G_eAntApiCurrentMessageClass == ANT_TICK) */
  } /* end AntReadData() */

}/* end static void UserAppSM_ChannelOpen(void) */



/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserAppSM_WaitChannelClose(void)          
{
  /* Monitor the channel status to check if channel is closed */
  if(AntRadioStatus() == ANT_CLOSED)
  {
     UserApp_StateMachine = UserAppSM_Idle;
     if(ControlLcdFlag1)
     {
        LedOff(WHITE);
        LedOff(PURPLE);
        LedOff(BLUE);
        LedOff(CYAN);
        LedOff(GREEN);
        LedOff(YELLOW);
        LedOff(ORANGE);
        LedOff(RED);
        LedOff(LCD_RED);
        LedOff(LCD_GREEN);
        LedOff(LCD_BLUE);
        LCDCommand(LCD_CLEAR_CMD);
     }
  }
  
  /* Check for timeout */
  if( IsTimeUp(&UserApp_u32Timeout, TIMEOUT_VALUE) )
  {
    UserApp_StateMachine = UserAppSM_Error;
  }
  
} /* end UserAppSM_WaitChannelClose(void) */

/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserAppSM_Error(void)          
{
  
} /* end UserAppSM_Error() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* State to sit in if init failed */
static void UserAppSM_FailedInit(void)          
{
    
} /* end UserAppSM_FailedInit() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/