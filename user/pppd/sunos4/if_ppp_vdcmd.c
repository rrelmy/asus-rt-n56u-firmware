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
#include <sys/errno.h>
#include <sys/conf.h>
#include <sun/vddrv.h>

extern struct streamtab if_pppinfo;

static struct vdldrv vd = {
    VDMAGIC_USER,
    "if_ppp"
};

static int fmodsw_index = -1;

int
if_ppp_vdcmd(fun, vdp, vdi, vds)
    unsigned int fun;
    struct vddrv *vdp;
    addr_t vdi;
    struct vdstat *vds;
{
    int n, error;

    switch (fun) {
    case VDLOAD:
	vdp->vdd_vdtab = (struct vdlinkage *) &vd;
	if (fmodsw_index >= 0)
	    return EBUSY;
	for (n = 0; n < fmodcnt; ++n)
	    if (fmodsw[n].f_str == 0)
		break;
	if (n >= fmodcnt)
	    return ENODEV;
	strncpy(fmodsw[n].f_name, vd.Drv_name, FMNAMESZ+1);
	fmodsw[n].f_str = &if_pppinfo;
	fmodsw_index = n;
	break;

    case VDUNLOAD:
	if (fmodsw_index <= 0)
	    return EINVAL;
	error = if_ppp_unload();
	if (error != 0)
	    return error;
	fmodsw[fmodsw_index].f_name[0] = 0;
	fmodsw[fmodsw_index].f_str = 0;
	fmodsw_index = -1;
	break;

    case VDSTAT:
	break;

    default:
	return EIO;
    }
    return 0;
}
