/*
 *  Hamlib R&S backend - main file
 *  Copyright (c) 2009 by Stéphane Fillod
 *
 *	$Id: rft.c,v 1.3 2006/10/07 18:48:34 csete Exp $
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <math.h>

#include "hamlib/rig.h"
#include "serial.h"
#include "misc.h"
#include "register.h"

#include "rs.h"


#define BUFSZ 64

#define LF "\x0a"
#define CR "\x0d"
#define BOM CR
#define EOM CR

/*
 * R&S GB2 protocol ?
 */


/*
 * rs_transaction
 * We assume that rig!=NULL, rig->state!= NULL, data!=NULL, data_len!=NULL
 */
int rs_transaction(RIG *rig, const char *cmd, int cmd_len, char *data, int *data_len)
{
	int retval;
	struct rig_state *rs;

	rs = &rig->state;

	serial_flush(&rs->rigport);

	retval = write_block(&rs->rigport, cmd, cmd_len);
	if (retval != RIG_OK)
		return retval;


	/* no data expected, TODO: flush input? */
	if (!data || !data_len)
		return 0;

	retval = read_string(&rs->rigport, data, BUFSZ, CR, 1);
	if (retval == -RIG_ETIMEOUT)
		retval = 0;
	if (retval < 0)
		return retval;
	*data_len = retval;

	return RIG_OK;
}

/*
 * rs_set_freq
 * Assumes rig!=NULL
 */
int rs_set_freq(RIG *rig, vfo_t vfo, freq_t freq)
{
	char freqbuf[16], ackbuf[16];
	int freq_len, ack_len, retval;

	/* 
	 */
	freq_len = sprintf(freqbuf, BOM "FREQ %f MHz" EOM, freq/1e6);
	retval = rs_transaction(rig, freqbuf, freq_len, ackbuf, &ack_len);

	return retval;
}

/*
 * initrigs_rs is called by rig_backend_load
 */
DECLARE_INITRIG_BACKEND(rs)
{
	rig_debug(RIG_DEBUG_VERBOSE, "rs: _init called\n");

	rig_register(&esmc_caps);

	return RIG_OK;
}

