/*
 * signals.hh
 *
 *  Created on: 21.01.2015
 *      Author: Daniel
 */

#ifndef SOURCE_INC_SIGNALS_HH_
#define SOURCE_INC_SIGNALS_HH_


typedef enum {
    SIGNAL_GENERIC,
    SIGNAL_COND
} SignalType;


/* Signals that correspond to an event of a specific task */
#define SIGNAL_SPACE_TASK(x) ((x & 0xff) << 16)

/*
 *  SIGNAL DEFINITIONS
 */
#define SIG_TASK_TERMINATED     0
#define SIG_CHILD_TERMINATED    1


#endif /* SOURCE_INC_SIGNALS_HH_ */
