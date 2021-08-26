#ifndef PROGRESS_H
#define PROGRESS_H

#include <stdio.h>
#include <unistd.h>

#define PROGRESS_BEGIN(R) do { \
    progress_max = (R); \
    progress_step = (R) / 1000; \
    if (progress_step <= 0) \
      progress_step = 1; \
  } while (0)

#define PROGRESS_UPDATE(S, P) do { \
    if (isatty(fileno(stderr)) && !((P) % progress_step)) { \
      float progress = (P) / (float)progress_max; \
      fprintf(stderr, "\r" S "%c  %.1f %%", "-\\|/"[(i / progress_step) % 4], progress * 100.0); \
    } \
  } while (0)

#define PROGRESS_FINISH(S) do { \
    if (isatty(fileno(stderr))) { \
      fprintf(stderr, "\r" S "*  100.0 %% \n"); \
    } else { \
      fprintf(stderr, S "*  100.0 %% \n"); \
    } \
  } while (0)


extern int progress_max;
extern int progress_step;

#endif
