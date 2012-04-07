/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/conf.h>
#include <sys/modctl.h>
#include <sys/sunddi.h>

extern struct streamtab ppp_ahdlcinfo;

static struct fmodsw fsw = {
    "ppp_ahdl",
    &ppp_ahdlcinfo,
    D_NEW | D_MP | D_MTQPAIR
};

extern struct mod_ops mod_strmodops;

static struct modlstrmod modlstrmod = {
    &mod_strmodops,
    "PPP async HDLC module",
    &fsw
};

static struct modlinkage modlinkage = {
    MODREV_1,
    (void *) &modlstrmod,
    NULL
};

/*
 * Entry points for modloading.
 */
int
_init(void)
{
    return mod_install(&modlinkage);
}

int
_fini(void)
{
    return mod_remove(&modlinkage);
}

int
_info(mip)
    struct modinfo *mip;
{
    return mod_info(&modlinkage, mip);
}
