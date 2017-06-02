/*
*                                       ++++++++++++++++++
*                                  +++++++++++++++++++++++++++++
*                              +++++++                      +++++++++
*                          +++++++                               +++++++++++++
*         ++++++++++++++++++++                                         ++++++++++
*    +++++++++++++++++++++                                                     +++
*   +++++                                                                       +++
*  +++         ######### ######### ########  #########  #########   +++++++      ++
*  +++  +++++ ####  #### ######## ####  #### ##### #### #### ####  +++  ++++    +++
*  +++   ++++ ###     ## ###      ###    ### ###    ### ###    ### ++++++++   +++
*   ++++ ++++ ########## ###      ########## ###    ### ###    ### ++++    +++++
*    +++++++   ###### ## ###       ########  ###     ## ##     ###  ++++++++++
*
* Copyright 2017 Technolution BV  opensource@technolution.eu
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
* associated documentation files (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or substantial
* portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
* LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
* OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
* Sound module
* ============
*
* This module controls the audio output. The main loop sits waiting for a semaphore to be
* set indicating that there's some output needed. The code automatically deals with different
* sample rates etc....but it's only mono.
*
* In a 'full' implementation this probably needs extending to read directly from a file rather
* than from memory....that'll run out quickly.
*
*/

#include "config.h"
#include "audio.h"

#ifdef INCLUDE_AUDIO
#define BURST_SIZE      (1024)                          /* Size of an individual DMA transfer - affects average time to stop */

static struct
{
    uint8_t                     dmaChannelNum;          /* Channel selected for DMA activities */
    xSemaphoreHandle            dmaTransferComplete;    /* From Interrupt to base level - time for more feeding */
    xSemaphoreHandle            playDone;               /* From base level to caller - when play is completed */
    xSemaphoreHandle            playNow;                /* From caller to base level - start playing */
    const uint32_t              *playPsn;               /* Where we are in what we're playing */
    uint32_t                    remaining;              /* Remaining material to be played */
} _audio;

// ============================================================================================
void DMA_IRQHandler( void )

/* Handler that the DMA is complete - just tell the base task. This will need to move
 * elsewhere if the DMA is used by multiple functions, but we only need the kick...deliberately
 * avoided any program logic in here.
 */

{
    portBASE_TYPE higherPriorityTaskWoken = FALSE;

    Chip_GPDMA_Interrupt( LPC_GPDMA, _audio.dmaChannelNum );

    /* Signal that transfer is complete and new status is available */
    xSemaphoreGiveFromISR( _audio.dmaTransferComplete, &higherPriorityTaskWoken );

    portEND_SWITCHING_ISR( higherPriorityTaskWoken );
}
// ============================================================================================
static void _audioTask( void *pvParameters )

/* The main loop for the care and feeding of audio output. Driven via a couple of semaphores.
 */
{
    uint32_t n;

    while ( TRUE )
    {
        /* Wait for something to play */
        while ( !xSemaphoreTake( _audio.playNow, portMAX_DELAY ) );

        /* enable audio amplifier */
        Chip_GPIO_SetPinState( LPC_GPIO_PORT, GETGPIOPORT( AUDIO_ENABLE ), GETGPIOPIN( AUDIO_ENABLE ), TRUE );

        while ( _audio.remaining )
        {
            n = ( _audio.remaining > BURST_SIZE ) ? BURST_SIZE : _audio.remaining;
            Chip_GPDMA_Transfer( LPC_GPDMA, _audio.dmaChannelNum, ( uint32_t )_audio.playPsn, GPDMA_CONN_DAC,
                                 GPDMA_TRANSFERTYPE_M2P_CONTROLLER_DMA, n );

            /* Accommodate the material already transferred */
            _audio.playPsn += n;
            _audio.remaining -= n;

            /* Now calculate the next block, assuming there is one */
            while ( !xSemaphoreTake( _audio.dmaTransferComplete, portMAX_DELAY ) );
        }

        /* disable audio amplifier */
        Chip_GPIO_SetPinState( LPC_GPIO_PORT, GETGPIOPORT( AUDIO_ENABLE ), GETGPIOPIN( AUDIO_ENABLE ), FALSE );

        /* Tell the caller we are done */
        xSemaphoreGive( _audio.playDone );
    }
}
// ============================================================================================
// ============================================================================================
// ============================================================================================
// Publicly available routines
// ============================================================================================
// ============================================================================================
// ============================================================================================
BOOL audioPlaying( void )

/* Let callers know if there is audio playing */

{
    return ( _audio.remaining != 0 );
}
// ============================================================================================
BOOL audioPlay( uint32_t playLenSet, uint32_t sampleRateSet, BOOL waitSet,
                const uint32_t *dataToPlay )

/* Play new audio at appropriate sample rate etc. Returns FALSE if there's already something playing. */
{
    if ( audioPlaying() )
    {
        return FALSE;
    }

    xSemaphoreTake( _audio.dmaTransferComplete, 0 );
    xSemaphoreTake( _audio.playDone, 0 );
    xSemaphoreTake( _audio.playNow, 0 );

    _audio.playPsn = dataToPlay;
    _audio.remaining = playLenSet;

    // set sample rate for DAC - This is how often a new sample is requested from the DMA
    Chip_DAC_SetDMATimeOut( LPC_DAC, ( Chip_Clock_GetRate( CLK_APB3_DAC ) / sampleRateSet ) );

    /* OK, make it happen */
    xSemaphoreGive( _audio.playNow );

    /* ...and wait if we've been asked to */
    if ( waitSet )
        while ( !xSemaphoreTake( _audio.playDone, portMAX_DELAY ) );

    return TRUE;
}
// ============================================================================================
void audioStop( void )

{
    /* ...it's not quite immediate, but good enough for most purposes */
    _audio.remaining = 0;
}
// ============================================================================================
void audioInit( void )

{
    // audio enable pin & audio out pin
    Chip_SCU_PinMuxSet( GETPORT( AUDIO_ENABLE ), GETPIN( AUDIO_ENABLE ), GETFUNC( AUDIO_ENABLE ) );
    Chip_GPIO_SetPinDIROutput( LPC_GPIO_PORT, GETGPIOPORT( AUDIO_ENABLE ), GETGPIOPIN( AUDIO_ENABLE ) );
    Chip_GPIO_SetPinState( LPC_GPIO_PORT, GETGPIOPORT( AUDIO_ENABLE ), GETGPIOPIN( AUDIO_ENABLE ), FALSE );

    Chip_SCU_PinMuxSet( GETPORT( AUDIO_OUT ), GETPIN( AUDIO_OUT ), GETFUNC( AUDIO_OUT ) );
    Chip_GPIO_SetPinDIRInput( LPC_GPIO_PORT, GETGPIOPORT( AUDIO_OUT ), GETGPIOPIN( AUDIO_OUT ) );

    /* Make sure pin is configured for analogue output */
    Chip_SCU_DAC_Analog_Config();

    /* DAC HW init */
    Chip_DAC_Init( LPC_DAC );

    /* High current mode, relatively speaking */
    Chip_DAC_SetBias( LPC_DAC, DAC_MAX_UPDATE_RATE_1MHz );

    /* enable DMA control by DAC */
    Chip_DAC_ConfigDAConverterControl( LPC_DAC, ( DAC_CNT_ENA | DAC_DMA_ENA | DAC_DBLBUF_ENA ) );

    /* Initialize GPDMA controller */
    Chip_GPDMA_Init( LPC_GPDMA );

    /* Get the free channel for DMA transfer */
    _audio.dmaChannelNum = Chip_GPDMA_GetFreeChannel( LPC_GPDMA, GPDMA_CONN_DAC );

    /* ...and create a set of semaphores */
    _audio.dmaTransferComplete = xSemaphoreCreateBinary();
    _audio.playDone = xSemaphoreCreateBinary();
    _audio.playNow = xSemaphoreCreateBinary();

    /* Setting GPDMA interrupt */
    NVIC_DisableIRQ( DMA_IRQn );
    NVIC_SetPriority( DMA_IRQn, AUDIO_INT_PRIORITY );
    NVIC_EnableIRQ( DMA_IRQn );

    xTaskCreate( _audioTask, "sound", 70, NULL, ( tskIDLE_PRIORITY + 2UL ), NULL );
}
// ============================================================================================
#endif
