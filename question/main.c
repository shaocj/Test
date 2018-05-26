#include <math.h>
#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <time.h>

#define __EXTRA_CMDL_SERVICE
#define __TEXT_ERRORS_PRINT

#if (defined(_WIN32) || defined(_WIN64))&& (defined( _WINDOWS) ||defined(WINFORMS))
   __pragma(warning(disable:4127))
   __pragma(warning(disable:4200))
#endif


#include "vep-api.h"

extern int read_wave_in(short *prm, short *ref);
extern void close_wave_in(void);
extern void write_wave_out(short *in_prm, short *in_ref, short *out_prm, short *out_ref);
extern void write_wave_out_bli_cur(short *out_bli);
extern void write_wave_out_bli_dlt(short *out);
extern void write_wave_out_bli_bkg(short *out_bli);
extern void write_wave_out_blip(short *bli_peak);
extern void write_wave_out_chn_real(int chan_real);
extern void write_wave_out_chn(int chan);
extern void write_wave_out_mux(short *out_mux);
extern void finalize_outputs(void);
extern void delete_outputs_on_error(void);


typedef  int int32_t;
typedef  short int16_t;


extern void *parse_input_arguments(int argc, char **argv, pitem_t *);


// #define FRAME 64 // for 8 kHz
#define FRAME 32*256*2 // for 16 kHz
short mic[FRAME], mic_c[FRAME];
short spk[FRAME ], spk_c[FRAME];
short mout[FRAME*2];
short sout[FRAME*2];
short sout_t[FRAME*2];

short mux_out[FRAME];

#define NUM_OF_BEAMS 20
short bli_cur[NUM_OF_BEAMS];
short bli_bkg[NUM_OF_BEAMS];
short bli_sco[NUM_OF_BEAMS];
short bli_peak[NUM_OF_BEAMS];




void 
error_exit(int err)
{
#if defined(_DEBUG) && ((defined(_WIN32) || defined(_WIN64)) || (defined(_WIN32) || defined(_WIN64)))
    __asm int 3;
#endif
    exit(err);
}

pitem_t *built_in_profile = 0;

int main(int argc, char **argv)
{
    unsigned int smem = 16000; //1792;
    void *mem;
    void *p;
    void *smr[NUM_MEM_REGIONS];
    mem_reg_t reg[NUM_MEM_REGIONS];
    int i,j;
    err_t err;
    int icounter = 0;
    int chan;
    int chan_real;


    p = parse_input_arguments(argc, argv, built_in_profile);

    if (!p)
    {
        fprintf(stderr, "Cannot find configuration.\n");
        error_exit(-3);
    }

 
    smem = vepug_get_hook_size();
    mem = malloc(smem);

    vepug_get_mem_size(p, reg, mem);

    free(mem);

    printf("Hello, I am VEP!\n");

    for (i = 0; i < NUM_MEM_REGIONS; i++)
    {
        reg[i].mem = smr[i] = (void *)malloc(reg[i].size);
        fprintf(stderr, "I need %d bytes of memory in memory region %d to work.\n", reg[i].size, i + 1);
    }

    err = vepug_init(p, reg);

	if (err.err)
	{
		fprintf(stderr, "Config error %s %d memb:%d pid:%d\n", __text_error[err.err], err.err, err.memb, err.pid);
		error_exit(-3);
	}

    while (1)
    {
        if (read_wave_in(spk, mic))
            break;

        memmove(mic_c, mic, sizeof(mic));
        memmove(spk_c, spk, sizeof(spk));
//        vepug_bli_bkg_freeze(reg, 200);


       vepug_process_multibeam(reg, mic, spk, mout); //must not be used if MUX_OUTPUT_ONLY
       if (err.err != 0) // only MUX output is used vep_process_multibeam() does not work
       {
         exit(-5);
       }
       else
       {
         write_wave_out(spk_c, mic_c, spk_c, mout);
       }
       
        err = vepug_process_beammux(reg, mic, spk, mux_out);

        chan = vepug_get_beammux_chan(reg);
//        fprintf(stderr, "channel = %d \n", chan);

        for (j = 0; j < NUM_OF_BEAMS; j++)
        {
          bli_cur[j] = 0;
          bli_bkg[j] = 0;
          bli_sco[j] = 0;
          bli_peak[j] = 0;
        }
        err = vepug_bli_get(reg, bli_sco, bli_cur, bli_bkg, bli_peak);
        chan_real = vepug_get_beammux_chan_real(reg);
        chan = vepug_get_beammux_chan(reg);

        write_wave_out_bli_cur(bli_cur);
        write_wave_out_bli_bkg(bli_bkg);
        write_wave_out_bli_dlt(bli_sco);
        write_wave_out_blip(bli_peak);
        write_wave_out_chn_real(chan_real);
        write_wave_out_chn(chan);

        write_wave_out_mux(mux_out);
        
    }

    close_wave_in();
    finalize_outputs();

    for (i = 0; i < NUM_MEM_REGIONS; i++)
        free(smr[i]);

    return 0;
}


