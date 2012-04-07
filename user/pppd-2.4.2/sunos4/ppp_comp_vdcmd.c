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

extern struct streamtab ppp_compinfo;
extern int ppp_comp_count;

static struct vdldrv vd = {
    VDMAGIC_USER,
    "ppp_comp"
};

static int fmodsw_index = -1;

int
ppp_comp_vdcmd(fun, vdp, vdi, vds)
    unsigned int fun;
    struct vddrv *vdp;
    addr_t vdi;
    struct vdstat *vds;
{
    int n;

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
	fmodsw[n].f_str = &ppp_compinfo;
	fmodsw_index = n;
	break;

    case VDUNLOAD:
	if (ppp_comp_count > 0)
	    return EBUSY;
	if (fmodsw_index <= 0)
	    return EINVAL;
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
