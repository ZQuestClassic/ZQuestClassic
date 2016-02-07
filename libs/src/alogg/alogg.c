/* Allegro OGG */
/* to play OGG files with Allegro */

/* OGG decoder part of Ogg Vorbis (Xiph.org Foundation) */
/* Allegro OGG is copyright (c) 2002 Javier Gonz lez */

/* See COPYING.txt for license */


#include <string.h>
#include <allegro.h>
#include <alogg.h>
#include "vorbis/vorbisfile.h"
#include "vorbis/codec.h"

#ifdef USEMEMWATCH
#include "../../memwatch.h"
#endif

#include <rubberband/rubberband-c.h>


/* standard ALOGG_OGG structure */

struct ALOGG_OGG {
  /* data info */
  void *data;                      /* ogg data */
  char *data_cursor;               /* pointer to data being read */
  int data_len;                    /* size of the data */
  /* decoder info */
  OggVorbis_File vf;
  int current_section;
  /* playing info */
  AUDIOSTREAM *audiostream;        /* the audiostream we are using to play */
                                   /* also used to know when it's playing */
  int audiostream_buffer_len;      /* len of the audiostream buffer */
  int stereo, freq, loop;          /* audio general info */
  int bitrate;                     /* bitrate info */
  int auto_polling;                /* set if the ogg is auto polling */
  int auto_poll_speed;             /* auto poll speed in msecs */
  int wait_for_audio_stop;         /* set if we are just waiting for the
                                      audiobuffer to stop plaing the last
                                      frame */
  int time_stretch;                /* set if we are using time stretch */
  RubberBandState time_stretch_state;
  float *time_stretch_buffer[2];   /* store samples here for processing */
  int time_stretch_buffer_samples; /* stereo/mono samples per frame */
};


/* standard ALOGG_OGGSTREAM structure */

struct ALOGG_OGGSTREAM {
  /* data info */
  void *databuf1, *databuf2;       /* data buffers */
  char *full_databuf;              /* to be used when the 2 buffers are filled */
  int data_cursor;                 /* pointer to data being read */
  int databuf_selector;            /* 1 = databuf1, -1 = databuf2 */
  int databuf_len;                 /* size of the data buffer */
  int unsel_databuf_free;          /* TRUE when the unselected databuffer */
                                   /* is free and needs to be filled */
  int bytes_used;                  /* number of bytes to use from the buffer */
                                   /* or -1 if the full buffer */
  /* decoder info */
  OggVorbis_File vf;
  int current_section;
  /* playing info */
  AUDIOSTREAM *audiostream;        /* the audiostream we are using to play */
                                   /* also used to know when it's playing */
  int audiostream_buffer_len;      /* len of the audiostream buffer */
  int freq, stereo;                /* audio general info */
  int frames_per_poll;             /* number of frames per play poll */
  int auto_polling;                /* set if the ogg is auto polling */
  int auto_poll_speed;             /* auto poll speed in msecs */
  int wait_for_audio_stop;         /* set if we are just waiting for the
                                      audiobuffer to stop plaing the last
                                      frame */
};



static void(*alogg_buffer_callback)(void * buf, int length) = NULL;

/* API - OGG */

/* callbacks */

static int alogg_endianess = 0; // default to intel

void alogg_set_buffer_callback(void(*callback)(void * buf, int length))
{
	alogg_buffer_callback = callback;
}

void alogg_detect_endianess(void)
{
	unsigned long Value32;
	unsigned char * VPtr = (unsigned char *)&Value32;

	VPtr[0] = VPtr[1] = VPtr[2] = 0; VPtr[3] = 1;

	if(Value32 == 1)
	{
		alogg_endianess = 1; // big endian
	}
	else
	{
		alogg_endianess = 0; // little endian
	}
}

size_t _alogg_ogg_read(void *ptr, size_t size, size_t nmemb, void *datasource) {
  ALOGG_OGG *ogg = (ALOGG_OGG *)datasource;
  int data_pos = ogg->data_cursor - (char *)ogg->data;
  int data_left = ogg->data_len - data_pos;
  int bytes_asked = size * nmemb;

  /* check how much we can copy */
  if (bytes_asked <= data_left) {
    /* we can copy it all */
    memcpy(ptr, (const void *)ogg->data_cursor, bytes_asked);
    ogg->data_cursor += bytes_asked;
    /* all items read */
    return nmemb;
  }
  else {
    int items = data_left / size;
    int bytes = items * size;
    /* check we are copying at least one item */
    if (items < 1)
      return 0;
    /* copy whatever we have left */
    memcpy(ptr, (const void *)ogg->data_cursor, bytes);
    ogg->data_cursor += bytes;
    /* return how many items we read */
    return items;
  }
}

int _alogg_ogg_seek(void *datasource, ogg_int64_t offset, int whence) {
  ALOGG_OGG *ogg = (ALOGG_OGG *)datasource;
  char *new_data_cursor;

  if (whence == SEEK_CUR)
    new_data_cursor = ogg->data_cursor + offset;
  else if (whence == SEEK_SET)
    new_data_cursor = (char *)ogg->data + offset;
  else if (whence == SEEK_END)
    new_data_cursor = (char *)ogg->data + ogg->data_len + offset;
  else
    return -1;

  /* check the new pointer is valid */
  if ((new_data_cursor < (char *)ogg->data) ||
     (new_data_cursor > ((char *)ogg->data + ogg->data_len))) {
    return -1;
  }
  else {
    ogg->data_cursor = new_data_cursor;
    return 0;
  }
}

int _alogg_ogg_close(void *datasource) {
  ALOGG_OGG *ogg = (ALOGG_OGG *)datasource;
  ogg->data_cursor = (char *)ogg->data;

  return 0;
}

long _alogg_ogg_tell(void *datasource) {
  ALOGG_OGG *ogg = (ALOGG_OGG *)datasource;
  return (ogg->data_cursor - (char *)ogg->data);
}

ov_callbacks _alogg_ogg_callbacks = {
  &_alogg_ogg_read,
  &_alogg_ogg_seek,
  &_alogg_ogg_close,
  &_alogg_ogg_tell
};


/* function called in an interrupt */

static void alogg_autopoll_ogg(void *ogg) {
  alogg_poll_ogg((ALOGG_OGG *)ogg);
}
END_OF_FUNCTION(alogg_autopoll_ogg);


/* API functions */

ALOGG_OGG *alogg_create_ogg_from_buffer(void *data, int data_len) {
  ALOGG_OGG *ogg;
  vorbis_info *vi;
  int ret;

  /* create a new ogg struct */
  ogg = (ALOGG_OGG *)malloc(sizeof(ALOGG_OGG));
  if (ogg == NULL)
    return NULL;

  /* fill in the ogg struct */
  ogg->data = data;
  ogg->data_cursor = (char *)data;
  ogg->data_len = data_len;
  memset((void *)&ogg->vf, 0, sizeof(ogg->vf));
  ogg->audiostream = NULL;
  ogg->loop = FALSE;
  ogg->auto_polling = FALSE;
  ogg->current_section = -1;
  ogg->time_stretch = 0;

  /* use vorbisfile to open it */
  ret = ov_open_callbacks((void *)ogg, &(ogg->vf), NULL, 0, _alogg_ogg_callbacks);

  /* if error */
  if (ret < 0) {
    free((void *)ogg);
    return NULL;
  }

  /* get audio info */
  vi = ov_info(&(ogg->vf), -1);
  if (vi->channels > 1)
    ogg->stereo = TRUE;
  else
    ogg->stereo = FALSE;
  ogg->freq = vi->rate;
  ogg->bitrate = vi->bitrate_nominal;

  return ogg;
}


ALOGG_OGG *alogg_create_ogg_from_file(FILE *f) {
  ALOGG_OGG *ogg;
  vorbis_info *vi;
  int ret;

  /* create a new ogg struct */
  ogg = (ALOGG_OGG *)malloc(sizeof(ALOGG_OGG));
  if (ogg == NULL)
    return NULL;

  /* fill in the ogg struct */
  ogg->data = NULL;
  ogg->data_cursor = NULL;
  {
    long pos = ftell(f);
    /* if error */
    if(pos < 0) {
      free((void *)ogg);
      return NULL;
    }
    ret = fseek(f, 0, SEEK_END);
    /* if error */
    if(ret < 0) {
      free((void *)ogg);
      return NULL;
    }
    ogg->data_len = ftell(f);
    /* if error (use ferror() because data_len is an int instead of a long) */
    if(ferror(f)) {
      free((void *)ogg);
      return NULL;
    }
    ret = fseek(f, pos, SEEK_SET);
    /* if error */
    if(ret < 0) {
      free((void *)ogg);
      return NULL;
    }
  }
  memset((void *)&ogg->vf, 0, sizeof(ogg->vf));
  ogg->audiostream = NULL;
  ogg->loop = FALSE;
  ogg->auto_polling = FALSE;
  ogg->current_section = -1;

  /* use vorbisfile to open it */
  ret = ov_open(f, &(ogg->vf), NULL, 0);

  /* if error */
  if (ret < 0) {
    free((void *)ogg);
    return NULL;
  }

  /* get audio info */
  vi = ov_info(&(ogg->vf), -1);
  if (vi->channels > 1)
    ogg->stereo = TRUE;
  else
    ogg->stereo = FALSE;
  ogg->freq = vi->rate;

  return ogg;
}


void alogg_destroy_ogg(ALOGG_OGG *ogg) {
  if (ogg == NULL)
    return;

  alogg_stop_ogg(ogg);          /* note alogg_stop_ogg will also remove */
                                /* autopolling interrupts */
  ov_clear(&(ogg->vf));
  free(ogg);
}

int alogg_play_ogg(ALOGG_OGG *ogg, int buffer_len, int vol, int pan) {
  return alogg_play_ex_ogg(ogg, buffer_len, vol, pan, 1000, FALSE);
}

int alogg_play_ogg_ts(ALOGG_OGG *ogg, int buffer_len, int vol, int pan, int speed) {
  int ret;

  /* start playing Ogg at normal speed */
  ret = alogg_play_ex_ogg(ogg, buffer_len, vol, pan, 1000, 0);
  if (ret != ALOGG_OK)
    return ret;

  /* don't set up time stretching if we are playing at normal speed */
  if (speed == 1000)
    return ALOGG_OK;

  ogg->time_stretch = 1;
  ogg->time_stretch_buffer_samples = (buffer_len / (ogg->stereo ? 2 : 1)) / 2;
  ogg->time_stretch_state = rubberband_new(ogg->freq, ogg->stereo ? 2 : 1, RubberBandOptionProcessRealTime |  RubberBandOptionThreadingNever, 1000.0 / (float)speed, 1.0);
  rubberband_set_max_process_size(ogg->time_stretch_state, ogg->time_stretch_buffer_samples);
  ogg->time_stretch_buffer[0] = malloc(sizeof(float) * ogg->time_stretch_buffer_samples);
  if (ogg->stereo)
    ogg->time_stretch_buffer[1] = malloc(sizeof(float) * ogg->time_stretch_buffer_samples);
  else
    ogg->time_stretch_buffer[1] = NULL;

  return ALOGG_OK;
}


int alogg_play_ex_ogg(ALOGG_OGG *ogg, int buffer_len, int vol, int pan, int speed, int loop) {
  int samples;

  /* continue only if we are not already playing it */
  if (alogg_is_playing_ogg(ogg))
    return ALOGG_OK;

  /* check the buffer is big enough*/
  if (buffer_len < 1024)
    return ALOGG_PLAY_BUFFERTOOSMALL;

  /* create a new audiostream and play it */
  samples = buffer_len / (ogg->stereo ? 2 : 1) / 2; /* / 2 = 16 bits samples */
  ogg->audiostream = play_audio_stream(samples, 16, ogg->stereo, ogg->freq, vol, pan);
  ogg->audiostream_buffer_len = samples * (ogg->stereo ? 2 : 1) * 2; /* * 2 = 16 bits samples */

  if (speed != 1000)
    adjust_sample(ogg->audiostream->samp, vol, pan, speed, TRUE);

  ogg->loop = loop;
  ogg->wait_for_audio_stop = 0;

  /* if the user asked for autopolling, install the interrupt now */
  if (ogg->auto_polling) {
    LOCK_FUNCTION(alogg_autopoll_ogg);
    install_param_int(&alogg_autopoll_ogg, (void *)ogg, ogg->auto_poll_speed);
  }
  ogg->time_stretch = 0;
  ogg->time_stretch_buffer[0] = NULL;
  ogg->time_stretch_buffer[1] = NULL;

  return ALOGG_OK;
}


void alogg_stop_ogg(ALOGG_OGG *ogg) {
  /* continue if we are playing it */
  if (!alogg_is_playing_ogg(ogg))
    return;

  /* remove the interrupt, if we were using autopolling */
  if (ogg->auto_polling)
    remove_param_int(&alogg_autopoll_ogg, (void *)ogg);

  /* stop the audio stream */
  stop_audio_stream(ogg->audiostream);
  ogg->audiostream = NULL;
  if (ogg->time_stretch_buffer[0])
  {
    free(ogg->time_stretch_buffer[0]);
	ogg->time_stretch_buffer[0] = NULL;
  }
  if (ogg->time_stretch_buffer[1])
  {
    free(ogg->time_stretch_buffer[1]);
    ogg->time_stretch_buffer[1] = NULL;
  }
}


/* process the Ogg from start_time to end_time, passing the decoded data to the supplied callback
   if the start and end time are the same value, the entire OGG is processed from the specified start time until the end of the file
   buffer_samples is the internally allocated buffer size to use */
int alogg_process_ogg(ALOGG_OGG * ogg, void(*callback)(void * buf, int nsamples, int stereo), int buffer_samples, double start_time, double end_time)
{
	int size_done, i, all_done = 0;
	int ret = 0;
	int bits;
	char * buffer;
	char * buffer_p;
	double current_pos;
	int buffer_bytes = buffer_samples * 2;
	unsigned long num_samples_left = 0, num_samples_decoded;
	char all = 0;	//Is set to nonzero if the OGG is to be processed until the end of the file

	buffer = malloc(buffer_bytes);
	if(!buffer)
	{
		return 0;
	}

	current_pos = ov_time_tell(&(ogg->vf));
	ov_time_seek(&(ogg->vf), start_time);

	if(start_time == end_time)
	{	//If the calling function intends to process the OGG until the end of the file
		all = 1;
	}
	else
	{
		num_samples_left = ((end_time - start_time) * alogg_get_wave_freq_ogg(ogg)) * (ogg->stereo ? 2 : 1);	//The number of samples to pass to the callback function
	}
	bits = alogg_get_wave_bits_ogg(ogg);	//The number of bits per sample (expected to be 16)
	if(bits != 16)
	{	//Unexpected sample size
		free(buffer);
		return 0;
	}

	while(!all_done)
	{
		buffer_p = buffer;
		for(i = 0; i < buffer_samples; i++)
		{	//Fill buffer with silent audio
			((unsigned short *)(buffer_p))[i] = 0x8000;
		}
		size_done = num_samples_decoded = 0;

		/* read samples from Ogg Vorbis file */
		for(i = buffer_bytes; i > 0; i -= size_done)
		{
			/* decode */
			size_done = ov_read(&(ogg->vf), buffer_p, i, alogg_endianess, 2, 0, &(ogg->current_section));

			/* check if the decoding was not successful */
			if(size_done < 0)
			{
				if(size_done == OV_HOLE)
				{
					size_done = 0;
				}
				else
				{
					all_done = 2;
				}
			}
			else if(size_done == 0)
			{	//No more samples
				all_done = 1;
				ret = 1;
				break; // playback finished so get out of loop
			}
			num_samples_decoded += size_done / 2;	//Keep track of how many samples have been decoded since the last callback
			buffer_p += size_done;
		}
		if(all || (num_samples_decoded < num_samples_left))
		{	//If all of the newly decoded samples are to be passed to the callback
		   callback(buffer, buffer_samples, ogg->stereo);
		   num_samples_left -= num_samples_decoded;
		}
		else
		{	//If the samples passed to the callback will be limited to the given time range
		   callback(buffer, num_samples_left, ogg->stereo);
		   all_done = 3;
		   ret = 1;
		}
	}
    free(buffer);
    ov_time_seek(&(ogg->vf), current_pos);
    return ret;
}


void alogg_rewind_ogg(ALOGG_OGG *ogg) {
  ov_raw_seek(&(ogg->vf), 0);
}


void alogg_seek_abs_msecs_ogg(ALOGG_OGG *ogg, int msec) {
  /* convert msec to pcm sample position */
  double s = msec;
  s /= 1000;

  ov_time_seek(&(ogg->vf), s);
  if (ogg->time_stretch)
    rubberband_reset(ogg->time_stretch_state);
}


void alogg_seek_abs_secs_ogg(ALOGG_OGG *ogg, int sec) {
  ov_time_seek(&(ogg->vf), sec);
  if (ogg->time_stretch)
    rubberband_reset(ogg->time_stretch_state);
}


void alogg_seek_abs_bytes_ogg(ALOGG_OGG *ogg, int bytes) {
  /* check the pos in bytes is valid */
  if ((bytes < 0) || (bytes >= ogg->data_len))
    return;

  ov_raw_seek(&(ogg->vf), bytes);
}


void alogg_seek_rel_msecs_ogg(ALOGG_OGG *ogg, int msec) {
  double s = msec;
  s /= 1000;
  s += ov_time_tell(&(ogg->vf));

  ov_time_seek(&(ogg->vf), s);
}


void alogg_seek_rel_secs_ogg(ALOGG_OGG *ogg, int sec) {
  double s = sec;
  s += ov_time_tell(&(ogg->vf));
  ov_time_seek(&(ogg->vf), s);
}


void alogg_seek_rel_bytes_ogg(ALOGG_OGG *ogg, int bytes) {
  int current_byte = ov_raw_tell(&(ogg->vf));
  int wanted_byte = current_byte + bytes;

  /* check the byte is valid */
  if ((wanted_byte < 0) || (wanted_byte >= ogg->data_len))
    return;

  /* get current byte and add to it */
  bytes += ov_raw_tell(&(ogg->vf));
  ov_raw_seek(&(ogg->vf), bytes);
}


int alogg_poll_ogg_ts(ALOGG_OGG *ogg) {
  void *audiobuf;
  char *audiobuf_p;
  unsigned short *audiobuf_sp;
  int i, size_done, finished = 0;

  /* continue only if we are playing it */
  if (!alogg_is_playing_ogg(ogg))
    return ALOGG_POLL_NOTPLAYING;

  /* get the audio stream buffer and only continue if we need to fill it */
  audiobuf = get_audio_stream_buffer(ogg->audiostream);
  if (audiobuf == NULL)
    return ALOGG_OK;

  /* clear the buffer with 16bit unsigned data */
  {
    int i;
    unsigned short *j = (unsigned short *)audiobuf;
    for (i = 0; i < (ogg->audiostream_buffer_len / 2); i++, j++)
      *j = 0x8000;
  }

  /* if we need to fill it, but we were just waiting for it to finish */
  if (!ogg->loop) {
    if (ogg->wait_for_audio_stop > 0) {
      free_audio_stream_buffer(ogg->audiostream);
      if (--ogg->wait_for_audio_stop == 0) {
        /* stop it */
        alogg_stop_ogg(ogg);
        return ALOGG_POLL_PLAYJUSTFINISHED;
      }
      else
        return ALOGG_OK;
    }
  }

  audiobuf_sp = (unsigned short *)audiobuf;
  while (!finished && rubberband_available(ogg->time_stretch_state) < ogg->time_stretch_buffer_samples) {
    /* reset these each iteration so we don't overrun the buffer */
    audiobuf_p = (char *)audiobuf;
    size_done = 0;

    /* read samples from Ogg Vorbis file */
    for (i = ogg->audiostream_buffer_len; i > 0; i -= size_done) {
      /* decode */
      size_done = ov_read(&(ogg->vf), audiobuf_p, i, alogg_endianess, 2, 0, &(ogg->current_section));

      /* check if the decoding was not successful */
      if (size_done < 0) {
        if (size_done == OV_HOLE)
          size_done = 0;
        else {
          free_audio_stream_buffer(ogg->audiostream);
          alogg_stop_ogg(ogg);
          alogg_rewind_ogg(ogg);
          return ALOGG_POLL_FRAMECORRUPT;
        }
      }
      else if (size_done == 0) {
        alogg_rewind_ogg(ogg);
        ogg->wait_for_audio_stop = 2;
        finished = 1;
        break; // playback finished so get out of loop
      }
      audiobuf_p += size_done;
    }

    /* process samples with Rubber Band */
    if (ogg->stereo) {
      for (i = 0; i < ogg->time_stretch_buffer_samples; i++) {
        ogg->time_stretch_buffer[0][i] = (float)((long)audiobuf_sp[i * 2] - 0x8000) / (float)0x8000;		//Convert sample to signed floating point format
        ogg->time_stretch_buffer[1][i] = (float)((long)audiobuf_sp[i * 2 + 1] - 0x8000) / (float)0x8000;	//Repeat for the other channel's sample
      }
    }
    else {
      for (i = 0; i < ogg->time_stretch_buffer_samples; i++) {
        ogg->time_stretch_buffer[0][i] = (float)((long)audiobuf_sp[i] - 0x8000) / (float)0x8000;	//Convert sample to signed floating point format
      }
	}
    rubberband_process(ogg->time_stretch_state, (const float **)ogg->time_stretch_buffer, ogg->time_stretch_buffer_samples, 0);
  }

  /* retrieve audio from rubberband and put it into stream buffer */
  size_done = rubberband_retrieve(ogg->time_stretch_state, ogg->time_stretch_buffer, ogg->time_stretch_buffer_samples);
  if (ogg->stereo) {
    for (i = 0; i < size_done; i++) {
      if(ogg->time_stretch_buffer[0][i] > 1.0)
      {
        audiobuf_sp[i * 2] = 0xFFFF;
      }
      else if(ogg->time_stretch_buffer[0][i] < -1.0)
      {
        audiobuf_sp[i * 2] = 0;
      }
      else
      {
        audiobuf_sp[i * 2] = (ogg->time_stretch_buffer[0][i] * (float)0x8000) + (float)0x8000;		//Convert sample back to unsigned integer format
      }
      if(ogg->time_stretch_buffer[1][i] > 1.0)
      {
        audiobuf_sp[i * 2 + 1] = 0xFFFF;
      }
      else if(ogg->time_stretch_buffer[1][i] < -1.0)
      {
        audiobuf_sp[i * 2 + 1] = 0;
      }
      else
      {
        audiobuf_sp[i * 2 + 1] = (ogg->time_stretch_buffer[1][i] * (float)0x8000) + (float)0x8000;	//Repeat for the other channel's sample
      }
    }
  }
  else {
    for (i = 0; i < size_done; i++) {
      if(ogg->time_stretch_buffer[0][i] > 1.0)
      {
        audiobuf_sp[i] = 0xFFFF;
      }
      else if(ogg->time_stretch_buffer[0][i] < -1.0)
      {
        audiobuf_sp[i] = 0;
      }
      else
      {
        audiobuf_sp[i] = (ogg->time_stretch_buffer[0][i] * (float)0x8000) + (float)0x8000;		//Convert sample back to unsigned integer format
      }
    }
  }

  /* lock the buffer */
  if(alogg_buffer_callback)
  {
     alogg_buffer_callback(audiobuf, ogg->audiostream_buffer_len);
  }
  free_audio_stream_buffer(ogg->audiostream);
  return ALOGG_OK;
}

int alogg_poll_ogg(ALOGG_OGG *ogg) {
  void *audiobuf;
  char *audiobuf_p;
  int i, size_done;

  /* use alternate poller for time stretching mode */
  if (ogg->time_stretch)
    return alogg_poll_ogg_ts(ogg);

  /* continue only if we are playing it */
  if (!alogg_is_playing_ogg(ogg))
    return ALOGG_POLL_NOTPLAYING;

  /* get the audio stream buffer and only continue if we need to fill it */
  audiobuf = get_audio_stream_buffer(ogg->audiostream);
  if (audiobuf == NULL)
    return ALOGG_OK;

  /* clear the buffer with 16bit unsigned data */
  {
    int i;
    unsigned short *j = (unsigned short *)audiobuf;
    for (i = 0; i < (ogg->audiostream_buffer_len / 2); i++, j++)
      *j = 0x8000;
  }

  /* if we need to fill it, but we were just waiting for it to finish */
  if (!ogg->loop) {
    if (ogg->wait_for_audio_stop > 0) {
      free_audio_stream_buffer(ogg->audiostream);
      if (--ogg->wait_for_audio_stop == 0) {
        /* stop it */
        alogg_stop_ogg(ogg);
        return ALOGG_POLL_PLAYJUSTFINISHED;
      }
      else
        return ALOGG_OK;
    }
  }

  audiobuf_p = (char *)audiobuf;
  size_done = 0;
  for (i = ogg->audiostream_buffer_len; i > 0; i -= size_done) {
    /* decode */
    size_done = ov_read(&(ogg->vf), audiobuf_p, i, alogg_endianess, 2, 0, &(ogg->current_section));

    /* check if the decoding was not successful */
    if (size_done < 0) {
      if (size_done == OV_HOLE)
        size_done = 0;
      else {
        free_audio_stream_buffer(ogg->audiostream);
        alogg_stop_ogg(ogg);
        alogg_rewind_ogg(ogg);
        return ALOGG_POLL_FRAMECORRUPT;
      }
    }
    else if (size_done == 0) {
      /* we have reached the end */
      alogg_rewind_ogg(ogg);
      if (!ogg->loop) {
        free_audio_stream_buffer(ogg->audiostream);
        ogg->wait_for_audio_stop = 2;
        return ALOGG_OK;
      }
    }

    audiobuf_p += size_done;
  }

  /* lock the buffer */
  if(alogg_buffer_callback)
  {
     alogg_buffer_callback(audiobuf, ogg->audiostream_buffer_len);
  }
  free_audio_stream_buffer(ogg->audiostream);

  return ALOGG_OK;
}


void alogg_start_autopoll_ogg(ALOGG_OGG *ogg, int speed) {
  /* return if we are already auto polling */
  if (ogg->auto_polling)
    return;

  ogg->auto_polling = TRUE;
  ogg->auto_poll_speed = speed;

  /* only install the interrupt if we are already playing */
  /* otherwise, play will do */
  if (alogg_is_playing_ogg(ogg)) {
    LOCK_FUNCTION(alogg_autopoll_ogg);
    install_param_int(&alogg_autopoll_ogg, (void *)ogg, ogg->auto_poll_speed);
  }
}


void alogg_stop_autopoll_ogg(ALOGG_OGG *ogg) {
  /* return if we are already not auto polling */
  if (!ogg->auto_polling)
    return;

  ogg->auto_polling = FALSE;

  /* only remove the interrupt if we were already playing */
  /* otherwise, stop will do*/
  if (alogg_is_playing_ogg(ogg))
    remove_param_int(&alogg_autopoll_ogg, (void *)ogg);
}


int alogg_get_pos_msecs_ogg(ALOGG_OGG *ogg) {
  return (int)(ov_time_tell(&(ogg->vf)) * 1000);
}


int alogg_get_pos_secs_ogg(ALOGG_OGG *ogg) {
  return (int)ov_time_tell(&(ogg->vf));
}


int alogg_get_pos_bytes_ogg(ALOGG_OGG *ogg) {
  return ov_raw_tell(&(ogg->vf));
}


int alogg_get_length_msecs_ogg(ALOGG_OGG *ogg) {
  double s = ov_time_total(&(ogg->vf), -1) * 1000;
  return (int)s;
}


int alogg_get_length_secs_ogg(ALOGG_OGG *ogg) {
  return (int)ov_time_total(&(ogg->vf), -1);
}


int alogg_get_length_bytes_ogg(ALOGG_OGG *ogg) {
  return ov_raw_total(&(ogg->vf), -1);
}


int alogg_get_bitrate_ogg(ALOGG_OGG *ogg) {
  return ogg->bitrate;
}


int alogg_get_wave_bits_ogg(ALOGG_OGG *ogg) {
  return 16;
}


int alogg_get_wave_is_stereo_ogg(ALOGG_OGG *ogg) {
  return ogg->stereo;
}


int alogg_get_wave_freq_ogg(ALOGG_OGG *ogg) {
  return ogg->freq;
}


SAMPLE *alogg_create_sample_from_ogg(ALOGG_OGG *ogg) {
  SAMPLE *sample;
  char *data;
  int i, sample_len_bytes, sample_len, size_done, done;

  /* first we need to calculate the len of the sample in bytes */
  sample_len = ov_pcm_total(&(ogg->vf), -1);
  sample_len_bytes = (sample_len * (ogg->stereo ? 2 : 1)) * 2; /* / 2 = 16 bits */

  /* create the sample */
  sample = create_sample(16, ogg->stereo, ogg->freq, sample_len);

  /* return NULL if we were not able to allocate the memory for it */
  if (sample == NULL)
    return NULL;

  /* we need to stop and rewind the ogg */
  alogg_stop_ogg(ogg);
  alogg_rewind_ogg(ogg);

  /* set our pointer */
  data = (char *)sample->data;
  /* clear the sample buffer in unsigned format */
  {
    int i;
    unsigned short *j = (unsigned short *)data;
    for (i = 0; i < (sample_len_bytes / 2); i++, j++)
      *j = 0x8000;
  }

  /* decode */
  done = FALSE;
  size_done = 0;
  for (i = sample_len_bytes; !done && (i > 0); i -= size_done) {

    /* decode */
    size_done = ov_read(&(ogg->vf), data, i, alogg_endianess, 2, 0, &(ogg->current_section));

    /* check if the decoding was not successful */
    if (size_done < 0) {
      if (size_done == OV_HOLE)
        size_done = 0;
      else {
        alogg_rewind_ogg(ogg);
        destroy_sample(sample);
        return NULL;
      }
    }
    else if (size_done == 0)
      done = TRUE;

    data += size_done;
  }

  alogg_rewind_ogg(ogg);

  return sample;
}


void alogg_adjust_ogg(ALOGG_OGG *ogg, int vol, int pan, int speed, int loop) {
  /* return if we are not playing */
  if (!alogg_is_playing_ogg(ogg))
    return;

  /* adjust the sample */
  adjust_sample(ogg->audiostream->samp, vol, pan, speed, TRUE);
  ogg->loop = loop;
}


char * alogg_get_ogg_comment(ALOGG_OGG *ogg, const char *comment, char * out)
{
	int i, j, k;
	vorbis_comment * vc = ov_comment(&(ogg->vf), -1);
	for (i = 0; i < vc->comments; i++)
	{
		for(j = 0; j < strlen(comment); j++)
		{
			if(vc->user_comments[i][j] != comment[j])
			{
				break;
			}
		}
		if(j == strlen(comment))
		{
			for(k = 0; k < vc->comment_lengths[i]; k++)
			{
				out[k] = vc->user_comments[i][j + 1 + k];
			}
			out[k] = '\0';
			return out;
		}
	}
	return NULL;
}

void *alogg_get_output_wave_ogg(ALOGG_OGG *ogg, int *buffer_size) {
  /* return if we are not playing */
  if (!alogg_is_playing_ogg(ogg))
    return NULL;

  /* set the buffer_size */
  *buffer_size = ogg->audiostream_buffer_len;

  /* return the current audiostream sample data */
  return (ogg->audiostream->samp->data);
}


int alogg_is_playing_ogg(ALOGG_OGG *ogg) {
  if (ogg->audiostream == NULL)
    return FALSE;
  else
    return TRUE;
}


int alogg_is_looping_ogg(ALOGG_OGG *ogg) {
  if (!alogg_is_playing_ogg(ogg))
    return FALSE;
  return ogg->loop;
}


void alogg_set_loop_ogg(ALOGG_OGG *ogg, int loop) {
  /* return if we are not playing */
  if (!alogg_is_playing_ogg(ogg))
    return;

  ogg->loop = loop;
}


AUDIOSTREAM *alogg_get_audiostream_ogg(ALOGG_OGG *ogg) {
  return ogg->audiostream;
}




/* API - OGGSTREAM */

/* callbacks */

size_t _alogg_oggstream_read(void *ptr, size_t size, size_t nmemb, void *datasource) {
  ALOGG_OGGSTREAM *ogg = (ALOGG_OGGSTREAM *)datasource;
  int bytes_asked = size * nmemb, data_left;
  int last_block, temp_buf_size, using_full_databuf, items;
  char *temp_databuf;

  /* get if this will be the last block to be processed */
  if (ogg->bytes_used != -1)
    last_block = TRUE;
  else
    last_block = FALSE;

  /* if the 2 buffers are available, use the concatenated one */
  if (!ogg->unsel_databuf_free) {
    temp_databuf = ogg->full_databuf;
    using_full_databuf = TRUE;
    if (!last_block)
      temp_buf_size = ogg->databuf_len * 2;
    else
      temp_buf_size = ogg->databuf_len + ogg->bytes_used;
  }
  /* else just point to the current buffer */
  else {
    using_full_databuf = FALSE;
    if (ogg->databuf_selector == 1)
      temp_databuf = (char *)ogg->databuf1;
    else
      temp_databuf = (char *)ogg->databuf2;
    if (!last_block)
      temp_buf_size = ogg->databuf_len;
    else
      temp_buf_size = ogg->bytes_used;
  }

  /* get the data left */
  data_left = temp_buf_size - ogg->data_cursor;

  /* check how much we can copy */
  if (bytes_asked <= data_left) {
    /* we can copy it all */
    memcpy(ptr, (const void *)(temp_databuf + ogg->data_cursor), bytes_asked);
    ogg->data_cursor += bytes_asked;
    /* all items read */
    items = nmemb;
  }
  else {
    int bytes;
    items = data_left / size;
    bytes = items * size;
    /* check we are copying at least one item */
    if (items < 1)
      return 0;
    /* copy whatever we have left */
    memcpy(ptr, (const void *)(temp_databuf + ogg->data_cursor), bytes);
    ogg->data_cursor += bytes;
    /* return how many items we read */
    /* items = items */
  }

  /* adjust the databuf selectors */
  /* if we were using the full_databuf */
  if (using_full_databuf) {
    /* if the cursor passed the first buffer len */
    if (ogg->data_cursor >= ogg->databuf_len) {
      /* set the cursor to a good position and mark as free the old buffer */
      ogg->data_cursor -= ogg->databuf_len;
      ogg->databuf_selector = -ogg->databuf_selector;
      ogg->unsel_databuf_free = TRUE;
    }
  }

  return items;
}


int _alogg_oggstream_seek(void *datasource, ogg_int64_t offset, int whence) {
  return -1;
}

int _alogg_oggstream_close(void *datasource) {
  return 0;
}

long _alogg_oggstream_tell(void *datasource) {
  return 0;
}

ov_callbacks _alogg_oggstream_callbacks = {
  &_alogg_oggstream_read,
  &_alogg_oggstream_seek,
  &_alogg_oggstream_close,
  &_alogg_oggstream_tell
};


/* function called in an interrupt */

static void alogg_autopoll_oggstream(void *ogg) {
  alogg_poll_oggstream((ALOGG_OGGSTREAM *)ogg);
}
END_OF_FUNCTION(alogg_autopoll_oggstream);


/* API functions */

ALOGG_OGGSTREAM *alogg_create_oggstream(void *first_data_buffer, int data_buffer_len, int last_block) {
  ALOGG_OGGSTREAM *ogg;
  vorbis_info *vi;
  int ret;
  void *databuf1, *databuf2, *full_databuf;

  /* create a new ogg struct */
  ogg = (ALOGG_OGGSTREAM *)malloc(sizeof(ALOGG_OGGSTREAM));
  if (ogg == NULL)
    return NULL;

  /* allocate the buffers */
  databuf1 = malloc(data_buffer_len);
  if (databuf1 == NULL) {
    free((void *)ogg);
    return NULL;
  }
  databuf2 = malloc(data_buffer_len);
  if (databuf2 == NULL) {
    free((void *)ogg);
    free(databuf1);
    return NULL;
  }
  full_databuf = malloc(data_buffer_len * 2);
  if (full_databuf == NULL) {
    free((void *)ogg);
    free(databuf1);
    free(databuf2);
    return NULL;
  }

  /* copy the first data */
  memcpy(databuf1, first_data_buffer, data_buffer_len);

  /* fill in the ogg struct */
  ogg->databuf1 = databuf1;
  ogg->databuf2 = databuf2;
  ogg->full_databuf = (char *)full_databuf;
  ogg->data_cursor = 0;
  ogg->databuf_selector = 1;
  ogg->databuf_len = data_buffer_len;
  ogg->unsel_databuf_free = TRUE;
  if (last_block)
    ogg->bytes_used = data_buffer_len;
  else
    ogg->bytes_used = -1;
  ogg->audiostream = NULL;
  ogg->auto_polling = FALSE;

  /* use vorbisfile to open it */
  ret = ov_open_callbacks((void *)ogg, &(ogg->vf), NULL, 0, _alogg_oggstream_callbacks);

  /* if error */
  if (ret < 0) {
    free((void *)ogg);
    free((void *)databuf1);
    free((void *)databuf2);
    free((void *)full_databuf);
    return NULL;
  }

  /* get audio info */
  vi = ov_info(&(ogg->vf), -1);
  if (vi->channels > 1)
    ogg->stereo = TRUE;
  else
    ogg->stereo = FALSE;
  ogg->freq = vi->rate;

  return ogg;
}


void alogg_destroy_oggstream(ALOGG_OGGSTREAM *ogg) {
  if (ogg == NULL)
    return;

  alogg_stop_oggstream(ogg);          /* note alogg_stop_oggstream will */
                                      /* remove autopolling interrupts */
  ov_clear(&(ogg->vf));
  free(ogg->full_databuf);
  free(ogg->databuf1);
  free(ogg->databuf2);
  free(ogg);
}


int alogg_play_oggstream(ALOGG_OGGSTREAM *ogg, int buffer_len, int vol, int pan) {
  return alogg_play_ex_oggstream(ogg, buffer_len, vol, pan, 1000);
}


int alogg_play_ex_oggstream(ALOGG_OGGSTREAM *ogg, int buffer_len, int vol, int pan, int speed) {
  int samples;

  /* continue only if we are not already playing it */
  if (alogg_is_playing_oggstream(ogg))
    return ALOGG_OK;

  /* check the buffer is big enough*/
  if (buffer_len < 1024)
    return ALOGG_PLAY_BUFFERTOOSMALL;

  /* create a new audiostream and play it */
  samples = buffer_len / (ogg->stereo ? 2 : 1) / 2; /* / 2 = 16 bits samples */
  ogg->audiostream = play_audio_stream(samples, 16, ogg->stereo, ogg->freq, vol, pan);
  ogg->audiostream_buffer_len = samples * (ogg->stereo ? 2 : 1) * 2; /* * 2 = 16 bits samples */

  ogg->wait_for_audio_stop = 0;

  if (speed != 1000)
    adjust_sample(ogg->audiostream->samp, vol, pan, speed, TRUE);

  /* if the user asked for autopolling, install the interrupt now */
  if (ogg->auto_polling) {
    LOCK_FUNCTION(alogg_autopoll_oggstream);
    install_param_int(&alogg_autopoll_oggstream, (void *)ogg, ogg->auto_poll_speed);
  }

  return ALOGG_OK;
}


void alogg_stop_oggstream(ALOGG_OGGSTREAM *ogg) {
  /* continue if we are playing it */
  if (!alogg_is_playing_oggstream(ogg))
    return;

  /* remove the interrupt, if we were using autopolling */
  if (ogg->auto_polling)
    remove_param_int(&alogg_autopoll_oggstream, (void *)ogg);

  /* stop the audio stream */
  stop_audio_stream(ogg->audiostream);
  ogg->audiostream = NULL;
}


int alogg_poll_oggstream(ALOGG_OGGSTREAM *ogg) {
  void *audiobuf;
  char *audiobuf_p;
  int i, size_done;
  int last_block;

  /* continue only if we are playing it */
  if (!alogg_is_playing_oggstream(ogg))
    return ALOGG_POLL_NOTPLAYING;

  /* get the audio stream buffer and only continue if we need to fill it */
  audiobuf = get_audio_stream_buffer(ogg->audiostream);
  if (audiobuf == NULL)
    return ALOGG_OK;

  /* clear the buffer with unsinged data*/
  {
    int i;
    unsigned short *j = (unsigned short *)audiobuf;
    for (i = 0; i < (ogg->audiostream_buffer_len / 2); i++, j++)
      *j = 0x8000;
  }

  /* if we need to fill it, but we were just waiting for it to finish */
  if (ogg->wait_for_audio_stop > 0) {
    free_audio_stream_buffer(ogg->audiostream);
    if (--ogg->wait_for_audio_stop == 0) {
      /* stop it */
      alogg_stop_oggstream(ogg);
      return ALOGG_POLL_PLAYJUSTFINISHED;
    }
    else
      return ALOGG_OK;
  }

  /* get if this will be the last block to be processed */
  if (ogg->bytes_used != -1)
    last_block = TRUE;
  else
    last_block = FALSE;

  audiobuf_p = (char *)audiobuf;
  size_done = 0;
  for (i = ogg->audiostream_buffer_len; i > 0; i -= size_done) {
    size_done = ov_read(&(ogg->vf), audiobuf_p, i, alogg_endianess, 2, 0, &(ogg->current_section));

    /* check if the decoding was not successful */
    if (size_done < 0) {
      if (size_done == OV_HOLE)
        size_done = 0;
      else {
        free_audio_stream_buffer(ogg->audiostream);
        alogg_stop_oggstream(ogg);
        return ALOGG_POLL_FRAMECORRUPT;
      }
    }
    else if (size_done == 0) {
      free_audio_stream_buffer(ogg->audiostream);
      /* if this was not the last block, buffer underrun */
      if (!last_block) {
        alogg_stop_oggstream(ogg);
        return ALOGG_POLL_BUFFERUNDERRUN;
      }
      /* else we just finished playing, we need to wait for audio to stop */
      else {
        ogg->wait_for_audio_stop = 2;
        return ALOGG_OK;
      }
    }

    audiobuf_p += size_done;
  }

  /* lock the buffer */
  free_audio_stream_buffer(ogg->audiostream);

  return ALOGG_OK;
}


void alogg_start_autopoll_oggstream(ALOGG_OGGSTREAM *ogg, int speed) {
  /* return if we are already auto polling */
  if (ogg->auto_polling)
    return;

  ogg->auto_polling = TRUE;
  ogg->auto_poll_speed = speed;

  /* only install the interrupt if we are already playing */
  /* otherwise, play will do */
  if (alogg_is_playing_oggstream(ogg)) {
    LOCK_FUNCTION(alogg_autopoll_oggstream);
    install_param_int(&alogg_autopoll_oggstream, (void *)ogg, ogg->auto_poll_speed);
  }
}


void alogg_stop_autopoll_oggstream(ALOGG_OGGSTREAM *ogg) {
  /* return if we are already not auto polling */
  if (!ogg->auto_polling)
    return;

  ogg->auto_polling = FALSE;

  /* only remove the interrupt if we were already playing */
  /* otherwise, stop will do*/
  if (alogg_is_playing_oggstream(ogg))
    remove_param_int(&alogg_autopoll_oggstream, (void *)ogg);
}


void *alogg_get_oggstream_buffer(ALOGG_OGGSTREAM *ogg) {
  if (!ogg->unsel_databuf_free)
    return NULL;

  /* if the last block was already passed, we don't need more data */
  if (ogg->bytes_used != -1)
    return NULL;

  if (ogg->databuf_selector == 1)
    return ogg->databuf2;
  else
    return ogg->databuf1;
}


void alogg_free_oggstream_buffer(ALOGG_OGGSTREAM *ogg, int bytes_used) {
  if (ogg->unsel_databuf_free) {
    ogg->unsel_databuf_free = FALSE;
    ogg->bytes_used = bytes_used;
    if (ogg->databuf_selector == 1) {
      memcpy(ogg->full_databuf, ogg->databuf1, ogg->databuf_len);
      memcpy(ogg->full_databuf + ogg->databuf_len, ogg->databuf2, ogg->databuf_len);
    }
    else {
      memcpy(ogg->full_databuf, ogg->databuf2, ogg->databuf_len);
      memcpy(ogg->full_databuf + ogg->databuf_len, ogg->databuf1, ogg->databuf_len);
    }
  }
}


int alogg_get_pos_msecs_oggstream(ALOGG_OGGSTREAM *ogg) {
  return (int)(ov_time_tell(&(ogg->vf)) * 1000);
}


int alogg_get_pos_secs_oggstream(ALOGG_OGGSTREAM *ogg) {
  return (int)ov_time_tell(&(ogg->vf));
}


int alogg_get_pos_bytes_oggstream(ALOGG_OGGSTREAM *ogg) {
  return ov_raw_tell(&(ogg->vf));
}


int alogg_get_bitrate_oggstream(ALOGG_OGGSTREAM *ogg) {
  return ov_bitrate(&(ogg->vf), -1);
}


int alogg_get_wave_bits_oggstream(ALOGG_OGGSTREAM *ogg) {
  return 16;
}


int alogg_get_wave_is_stereo_oggstream(ALOGG_OGGSTREAM *ogg) {
  return ogg->stereo;
}


int alogg_get_wave_freq_oggstream(ALOGG_OGGSTREAM *ogg) {
  return ogg->freq;
}


void alogg_adjust_oggstream(ALOGG_OGGSTREAM *ogg, int vol, int pan, int speed) {
  /* return if we are not playing */
  if (!alogg_is_playing_oggstream(ogg))
    return;

  /* adjust the sample */
  adjust_sample(ogg->audiostream->samp, vol, pan, speed, TRUE);
}


void *alogg_get_output_wave_oggstream(ALOGG_OGGSTREAM *ogg, int *buffer_size) {
  /* return if we are not playing */
  if (!alogg_is_playing_oggstream(ogg))
    return NULL;

  /* set the buffer_size */
  *buffer_size = ogg->audiostream_buffer_len;

  /* return the current audiostream sample data */
  return (ogg->audiostream->samp->data);
}


int alogg_is_playing_oggstream(ALOGG_OGGSTREAM *ogg) {
  if (ogg->audiostream == NULL)
    return FALSE;
  else
    return TRUE;
}


AUDIOSTREAM *alogg_get_audiostream_oggstream(ALOGG_OGGSTREAM *ogg) {
  return ogg->audiostream;
}
