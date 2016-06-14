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


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


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

static u32 OutsideTemperature[55];
static u32 InsideTemperature[28];
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
 /*Set a changed outside temperature */
  static u8 i=0;
  static u32 u32OutsideCount=OutsideTemperatureTime;
  static u8  u8OutsideCurrentTemperature=0;
  OutsideTemperature[0]=0;
  u32OutsideCount--;
  if(u32OutsideCount==0)
  {
    u32OutsideCount=OutsideTemperatureTime;
    i++;
    if(i<=27)
    {
      u8OutsideCurrentTemperature++;
      OutsideTemperature[i]=u8OutsideCurrentTemperature;
    }
    else
    {
      u8OutsideCurrentTemperature--;
      OutsideTemperature[i]=u8OutsideCurrentTemperature;
    }
    if(i==55)
    {
      u8OutsideCurrentTemperature=0;
      i=0;
    }
  }     
  
  
  /* Look for BUTTON 0 to open channel */
  if(WasButtonPressed(BUTTON0))
  {
    /* Got the button, so complete one-time actions before next state */
    ButtonAcknowledge(BUTTON0);    
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
  static u16 TimeCount1=500;
  static u8 String1[]="It is open!";
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
  static u32 u32InsideCount=InsideTemperatureTime;
  static u8 u8LastState = 0xff;
  static u8 au8DataContent[] = "xxxxxxxxxxxxxxxx";
  static u8 au8LastAntData[ANT_APPLICATION_MESSAGE_BYTES] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  static u8 String00[]="Func:Cool";
  static u8 String01[]="Func:Heat";
  static u8 String02[]="Func:Com";
  static u8 String03[]="Func:Are";
  static u8 String04[]="Func:Aer";
  
  static u8 String1[]="Tem:";
  bool bGotNewData;

 
  /* Check for BUTTON0 to close channel */
  if(WasButtonPressed(BUTTON1))
  {
    /* Got the button, so complete one-time actions before next state */
    ButtonAcknowledge(BUTTON1);
    
    /* Queue close channel, initialize the u8LastState variable and change LED to blinking green to indicate channel is closing */
    AntCloseChannel();
    u8LastState = 0xff;   
    /* Set timer and advance states */
    UserApp_u32Timeout = G_u32SystemTime1ms;
    UserApp_StateMachine = UserAppSM_WaitChannelClose;
  } /* end if(WasButtonPressed(BUTTON0)) */
  
  /* A slave channel can close on its own, so explicitly check channel status */  
  if(AntRadioStatus() != ANT_OPEN)
  {
    u8LastState = 0xff;
    
    UserApp_u32Timeout = G_u32SystemTime1ms;
    AntOpenChannel();
  } /* if(AntRadioStatus() != ANT_OPEN) */

  /* Always check for ANT messages */
  if( AntReadData() )
  {
     /* New data message: check what it is */
    if(G_eAntApiCurrentMessageClass == ANT_DATA)
    {
      /* Check if the new data is the same as the old data and update as we go */
      bGotNewData = FALSE;
      for(u8 i = 0; i < ANT_APPLICATION_MESSAGE_BYTES; i++)
      {
        if(G_au8AntApiCurrentData[i] != au8LastAntData[i])
        {
          bGotNewData = TRUE;
          au8LastAntData[i] = G_au8AntApiCurrentData[i];
          au8DataContent[2 * i] = HexToASCIICharUpper(G_au8AntApiCurrentData[i] / 16);
          au8DataContent[2*i+1] = HexToASCIICharUpper(G_au8AntApiCurrentData[i] % 16); 
        }
      }
      if(bGotNewData)
      {
        /*Test*/
        DebugPrintf(au8DataContent);
        DebugLineFeed();
        /*The switch to open led,lcd*/
        if(G_au8AntApiCurrentData[7]==0x00)
        {
          LedOn(WHITE);
          LedOn(PURPLE);
          LedOn(BLUE);
          LedOn(CYAN);
          LedOn(GREEN);
          LedOn(YELLOW);
          LedOn(ORANGE);
          LedOn(RED);
          /* Backlight to white */  
          LedOn(LCD_RED);
          LedOn(LCD_GREEN);
          LedOn(LCD_BLUE);
          /*The function choice*/
          switch(au8DataContent[6])
          {
            case 00:LCDClearChars(LINE1_START_ADDR,8);
                    LCDMessage(LINE1_START_ADDR,String00);
            case 01:LCDClearChars(LINE1_START_ADDR,8);
                    LCDMessage(LINE1_START_ADDR,String01);
            case 02:LCDClearChars(LINE1_START_ADDR,8);
                    LCDMessage(LINE1_START_ADDR,String02);
            case 03:LCDClearChars(LINE1_START_ADDR,8);
                    LCDMessage(LINE1_START_ADDR,String03);
            case 04:LCDClearChars(LINE1_START_ADDR,8);
                    LCDMessage(LINE1_START_ADDR,String04);
          }
          /*The temperature be set*/
          LCDClearChars(LINE1_START_ADDR+10,6);
          LCDMessage(LINE1_START_ADDR+10,au8DataContent+2);
 
        }
      }
      



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
