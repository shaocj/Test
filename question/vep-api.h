#ifndef __VCP_API_H__
#define __VCP_API_H__


/************************************************************************/
/************************************************************************/
/*                                                                      */
/*                  Alango Voice Enhancement Package (VEP)              */
/*                                                                      */
/************************************************************************/
/************************************************************************/

/************************************************************************/
/*                     !!! DO NOT EDIT THIS FILE !!!                    */
/************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(_WIN32) || defined(_WIN64)) && (defined( _WINDOWS) || defined(WINFORMS))
__pragma(warning(disable : 4200))                // Zero-sized array is really defined. For further purposes RVV
#endif

#define VERSION_STRING      "Alango Voice Enchancement Package (VEP)"
#define PROFILE_VERSION     51

#define NUM_MEM_REGIONS     2

/************************************************************************/
/*  profile item type. Normally 16 bits per item.                       */
/************************************************************************/
typedef short           pitem_t;            // 16 bits
typedef unsigned short  uitem_t;
typedef short           pcm_t;



/************************************************************************/
/*  MEMORY regions                                                      */
/************************************************************************/
typedef struct mem_reg_s
{
    void        *mem;                           // memory provided
    int         size;                           // overall size of the region in bytes.
} mem_reg_t;

/************************************************************************/
/*  ERRORS and codes                                                    */
/************************************************************************/
typedef struct err_s {
    short err;                                    // error's id
    short pid;                                    // error profile's ID
    short memb;                                   // member number
} err_t;


#undef ERR_NO_ERROR            
#undef ERR_OUT_OF_RANGE        
#undef ERR_BLK_NOT_IMPLEMENTED 
#undef ERR_NULL_POINTER        
#undef ERR_OBJ_CORRUPTED       
#undef ERR_DEMO_EXPIRED        
#undef ERR_INVALID_VALUE       
#undef ERR_INVALID_COMB        
#undef ERR_INVALID_CRC         
#undef ERR_NOT_ENOUGH_MEMORY   
#undef ERR_BUILD_NOT_CORRECT   
#undef ERR_STEREO_EC_DISABLED  
#undef ERR_ES_NLD_DISABLED     
#undef ERR_UNKNOWN             

#define ERR_NO_ERROR                0        // no errors at all
#define ERR_OUT_OF_RANGE            1        // parameter is out of range
#define ERR_BLK_NOT_IMPLEMENTED     2        // block is not implemented
#define ERR_NULL_POINTER            3        // wrong parameter
#define ERR_OBJ_CORRUPTED           4        // object override happened in vep_process
#define ERR_DEMO_EXPIRED            5        // Demo interval ended
#define ERR_INVALID_VALUE           6        // just invalid
#define ERR_INVALID_COMB            7        // invalid value combination
#define ERR_INVALID_CRC             8        // invalid CRC
#define ERR_NOT_ENOUGH_MEMORY       9        // not enough memory
#define ERR_BUILD_NOT_CORRECT       10        // Build and profile are not matched Lite and not Lite
#define ERR_STEREO_EC_DISABLED      11        // Stereo EC is not allowed in current build
#define ERR_ES_NLD_DISABLED         12        // NLD is not allowed in current build 

#define ERR_UNKNOWN                 -1

#if defined(__TEXT_ERRORS_PRINT)
    // check 'main()' for details
    static char *__text_error[] = {
        "no error",
        "parameter is out of range",
        "block is not implemented",
        "parameter is a null pointer",
        "object has been corrupted",
        "demo license has expired",
        "invalid parameter's value",
        "invalid parameters combination",
        "invalid CRC",
        "not enough memory allocated",
        "build and profile do not match",
        "stereo EC is not allowed",
        "nld is not allowed"
    };
#endif


/************************************************************************/
/* BASIC ROUTINES:                                                      */
/************************************************************************/

/************************************************************************
FUNCTION:
vep_init() -- initializes VCP object with given profile and memory area(s).
ARGS:
p       - pointer to actual profile.
reg     - pointer to memory region structure(s).
RETURNS: integer values, corresponding to the following errors:
ERR_NO_ERROR,
ERR_INVALID_CRC,
ERR_INVALID_VALUE
For precise error detection use vep_init_debug()
OTHER:
Memory region(s) must be allocated prior vep_init() call,
size field of 'reg' must be filled with allocated size.
Once inited, neither 'reg' not its fields can be altered.
Profile 'p' can be destroyed after exit.
************************************************************************/
extern err_t vepug_init(void *p, mem_reg_t *reg);





/************************************************************************
FUNCTION:
vep_get_mem_size() -- computes required memory.
ARGS:
p       - pointer to actual profile.
reg     - pointer to memory region structure(s).
mem     - memory allocated with size returned by vep_get_hook_size()
RETURNS:
error code
OTHER:
upon return memory region(s) will have 'size'
field reflecting memory required.
************************************************************************/
extern err_t vepug_get_mem_size(void *p, mem_reg_t *reg, void *mem);


/************************************************************************
FUNCTION:
vep_get_hook_size()     -- returns the size of memory required
for low stack use in vep_get_mem_size().
RETURNS:
size required
************************************************************************/
extern int vepug_get_hook_size(void);

/************************************************************************
FUNCTION:
vep_process_multibeam     -- performs VEP processing 

ARGS:
txin,rxin  - pointers to input streams
reg     - pointer to memory region structure(s).
pcm_t *tmp - pointer to buffer  were output beams will be put
size of this buffer is equal to frame size * number of output beams 
RETURNS:
ERR_BLK_NOT_IMPLEMENTED
ERR_OBJ_CORRUPTED,
ERR_DEMO_EXPIRED,
ERR_UNKNOWN,
ERR_NO_ERROR
OTHER:
input buffers must be filled with data with size defined in
general profile field 'frlen'. Output sizes will be the same.

Input buffers must not be modified during call.
RETURNS:
0 or error
!!! NOTE: this function may not be avvailable in the specific VEP library
!!! variant. 
************************************************************************/
extern err_t vepug_process_multibeam(mem_reg_t *reg, pcm_t *txin, pcm_t *rxin, pcm_t *tmp);
extern err_t vepug_process(mem_reg_t *reg, pcm_t *txin, pcm_t *rxin, pcm_t *tmp);

/************************************************************************
FUNCTION:
vep_process_beammux     -- performs VEP processing and selection
of the most proper beam

ARGS:
txin,rxin  - pointers to input streams
reg     - pointer to memory region structure(s).
pcm_t *tmp - pointer to buffer  were output beam will be put
size of this buffer is equal to one frame length
RETURNS:
ERR_BLK_NOT_IMPLEMENTED
ERR_OBJ_CORRUPTED,
ERR_DEMO_EXPIRED,
ERR_UNKNOWN,
ERR_NO_ERROR
OTHER:
input buffers must be filled with data with size defined in
general profile field 'frlen'. Output sizes will be the same.

Input buffers must not be modified during call.
RETURNS:
0 or error
************************************************************************/
extern err_t vepug_process_beammux(mem_reg_t *reg, pcm_t *txin, pcm_t *rxin, pcm_t *tmp);


/************************************************************************
FUNCTION:
vep_get_beammux_chan()

RETURNS:
returns the number of the currently selected beam number in the mix
************************************************************************/
extern int vepug_get_beammux_chan(mem_reg_t *reg);
extern int vepug_get_beammux_chan_real(mem_reg_t *reg);

/************************************************************************
FUNCTION:
vep_bli_get     -- fill BLI data (band signal level index)
(BLI "instant" and "background" values, as well
as the "score" which is the difference of the two)
to arrays: bli_score[], bli_inst[], bli_bkg[]
shall be called after vep_process_multibeam() and vep_process_beammux()
RETURNS:
0 or error
************************************************************************/
extern err_t vepug_bli_get(mem_reg_t *reg, pcm_t *bli_score, pcm_t *bli_inst, pcm_t *bli_bkg, pcm_t *bli_peak);


/************************************************************************
FUNCTION:
vep_bli_bkg_freeze  -- freezes calculation of background BLI data to
itime milliseconds -- must be called immediately after the key-word 
recognition occurred
shall be called before vep_process_multibeam() and vep_process_beammux()
RETURNS:
0 or error
************************************************************************/
extern err_t vepug_bli_bkg_freeze(mem_reg_t *reg, pcm_t itime);

#ifdef USE_TIMERS
#include "timer.h"
    typedef enum {
        TIMER_ANALYSIS,
        TIMER_SYNTHESYS,
        TIMER_AF,
        TIMER_MIXER,
        TIMER_ADM,
        TIMER_TOTAL,
        NUM_TIMERS
    } timer_index_t;

    extern mytimer_t vcp_timer[NUM_TIMERS];

#define TIMER_START(timer_index) \
    timer_start(&vcp_timer[timer_index]);
#define TIMER_STOP(timer_index) \
    timer_stop(&vcp_timer[timer_index]);
#else
#define TIMER_START(timer_index)
#define TIMER_STOP(timer_index)
#endif

#ifdef __cplusplus
};
#endif

#endif //__VCP_API_H__
