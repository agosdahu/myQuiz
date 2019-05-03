/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>

#define NUM_OF_PLAYERS  (6u)
#define PLAYER_1        1u
#define PLAYER_2        2u
#define PLAYER_3        3u
#define PLAYER_4        4u
#define PLAYER_5        5u
#define PLAYER_6        6u

#define IDLE_LIGHT      0xFFu

CY_ISR_PROTO(oneHandler);
CY_ISR_PROTO(twoHandler);
CY_ISR_PROTO(threeHandler);
CY_ISR_PROTO(fourHandler);
CY_ISR_PROTO(fiveHandler);
CY_ISR_PROTO(sixHandler);

CY_ISR_PROTO(rstHandler);
CY_ISR_PROTO(fncHandler);

/*****************************************/

uint8 arrGame [NUM_OF_PLAYERS];
uint8 qIndex = 0;
uint8 dispIndex = 0;
static uint8 EN = 0;
static uint8 game = 0;
//Systick Msec-to-sec
volatile static uint16 msCount = 999u;
volatile uint8 flag;
volatile uint8 gameReset;
volatile uint8 startAndStep;
static uint8 led = 0;

/*****************************************/
void Init();
void StartInts();
void StopInts();
void QueueWrite(uint8 player);
void SysTickISRCallback(void);      // ms tick ISR
void ResetGameState(void);
void ClearGameArr(void);
void FancyLedWorks(uint8 control);
void FancyLedsOff(uint8 except);

/*****************************************/

void Init()
{
    /* Systick Init triggers every milsec, throws flag every sec */
    CySysTickStart();
    CySysTickSetCallback(0, SysTickISRCallback);

    /* Component Inits */
    Buzzer_Start();
    
    /* Interrupt inits */    
    playerOne_StartEx(oneHandler);
    playerTwo_StartEx(twoHandler);
    playerThree_StartEx(threeHandler);
    playerFour_StartEx(fourHandler);
    playerFive_StartEx(fiveHandler);
    playerSix_StartEx(sixHandler);
    
    GM_rstInt_StartEx(rstHandler);
    GM_fncInt_StartEx(fncHandler);
    
    playerOne_ClearPending();
    playerTwo_ClearPending();
    playerThree_ClearPending();
    playerFour_ClearPending();
    playerFive_ClearPending();
    playerSix_ClearPending();
    
    GM_rstInt_ClearPending();
    GM_fncInt_ClearPending();
    
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    ResetGameState();
}

void StartInts()
{
    playerOne_ClearPending();
    playerTwo_ClearPending();
    playerThree_ClearPending();
    playerFour_ClearPending();
    playerFive_ClearPending();
    playerSix_ClearPending();
    
    playerOne_Enable();
    playerTwo_Enable();
    playerThree_Enable();
    playerFour_Enable();
    playerFive_Enable();
    playerSix_Enable();
    
    gameReset = 0;
}

void StopInts()
{
    playerOne_Disable();
    playerTwo_Disable();
    playerThree_Disable();
    playerFour_Disable();
    playerFive_Disable();
    playerSix_Disable();
}

void QueueWrite(uint8 player)
{
    BuzzerEN_Write(1);
    if(qIndex > NUM_OF_PLAYERS-1) 
        EN = 0;
    else 
        EN = 1;
    
    if(EN)
    {
        arrGame[qIndex] = player;
        qIndex++;
    }
}

void SysTickISRCallback(void)       //systick ISR - flag in each sec
{
    if(msCount > 0u)
    {
        --msCount;
    }
    else
    {
        /* Counts from 999 down to 0 */
        msCount = 999u;
        flag = 1;
    }
}

void ResetGameState(void)
{
    uint8 crit;
    crit = CyEnterCriticalSection();
    game = 0;
    qIndex = 0;
    dispIndex = 0;
    startAndStep = 0;
    gameReset = 0;
    EN = 0;
    FancyLedsOff(0);
    void ClearGameArr();
    CyExitCriticalSection(crit);
}

void ClearGameArr()
{
    uint8 i;
    for(i = 0; i < NUM_OF_PLAYERS; i++)
    {
        arrGame[i] = 0;
    }
}

void FancyLedWorks(uint8 control)
{
    led++;
    led = led % 0x08;
    
    if(control == 0xFF)
    {
        LED_1_Write(led);
        LED_2_Write(led);
        LED_3_Write(led);
        LED_4_Write(led);
        LED_5_Write(led);
        LED_6_Write(led);
    }
    else
    {
        FancyLedsOff(control);
        
        switch(control)
        {
            case PLAYER_1:
                LED_1_Write(led);
                break;
            case PLAYER_2:
                LED_2_Write(led);
                break;
            case PLAYER_3:
                LED_3_Write(led);
                break;
            case PLAYER_4:
                LED_4_Write(led);
                break;
            case PLAYER_5:
                LED_5_Write(led);
                break;
            case PLAYER_6:
                LED_6_Write(led);
                break;
            default:
                Red_LED_Write(1);
                break;
        }
    }
}
void FancyLedsOff(uint8 except)
{
    if(except != PLAYER_1)
        LED_1_Write(0);
    if(except != PLAYER_2)
        LED_2_Write(0);
    if(except != PLAYER_3)
        LED_3_Write(0);
    if(except != PLAYER_4)
        LED_4_Write(0);
    if(except != PLAYER_5)
        LED_5_Write(0);
    if(except != PLAYER_6)
        LED_6_Write(0);
        
    Red_LED_Write(0);
    Green_LED_Write(0);

}

/*****************************************/

int main()
{
    CyDelay(1000);
    //Welcome blinking?
    Green_LED_Write(1);
    Red_LED_Write(1);
    CyDelay(3000);
    Green_LED_Write(0);
    Red_LED_Write(0);
    
    Init();
    
    for(;;)
    {
        //Fancy LED villogtatás
        if(flag)
        {
            flag = 0;
            if(game)
                if(qIndex)      // van már válasz
                    FancyLedWorks(arrGame[dispIndex]);
                else //qIndex @ 0 -> no answers yet
                    FancyLedWorks(IDLE_LIGHT);
            else
                {
                    FancyLedsOff(0);
                    Green_LED_Write(~Green_LED_Read());
                }
        }
        
        //Játékállapotok
        if(gameReset)
        {
            Red_LED_Write(1);
            if(game)
                ResetGameState();
            else
                StartInts();
                
            game = (game == 1) ? 0 : 1;
            CyDelay(500);
            Red_LED_Write(0);
        }
        
        // Eredmény léptetés
        if(game)
        {
            uint8 mask = BuzzerEN_Read();
            if((mask & 0x01) == 0x01) //Buzzer ON
            {
                CyDelay(750);
                BuzzerEN_Write(0);
            }
            
            if(startAndStep)
            {
                startAndStep = 0;
                dispIndex++;
            }
        }
        
        //num players?
    }
}
/*****************************************/
/*              ISR                      */
/*****************************************/
CY_ISR(oneHandler)
{
    QueueWrite(PLAYER_1);
    playerOne_Disable();
}
CY_ISR(twoHandler)
{
    QueueWrite(PLAYER_2);
    playerTwo_Disable();
}
CY_ISR(threeHandler)
{
    QueueWrite(PLAYER_3);
    playerThree_Disable();
}
CY_ISR(fourHandler)
{
    QueueWrite(PLAYER_4);
    playerFour_Disable();
}
CY_ISR(fiveHandler)
{
    QueueWrite(PLAYER_5);
    playerFive_Disable();
}
CY_ISR(sixHandler)
{
    QueueWrite(PLAYER_6);
    playerSix_Disable();
}

CY_ISR(rstHandler)
{
    gameReset = 1;
    StopInts();
    GM_rstInt_ClearPending();
}

CY_ISR(fncHandler)
{
    startAndStep = 1;
    GM_fncInt_ClearPending();
}
/* [] END OF FILE */
