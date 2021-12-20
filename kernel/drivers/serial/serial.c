// SPDX-License-Identifier: (GPL-2.0 OR MPL-1.1)
/*======================================================================

    A driver for PCMCIA serial devices

    serial_cs.c 1.134 2002/05/04 05:48:53

    The contents of this file are subject to the Mozilla Public
    License Version 1.1 (the "License"); you may not use this file
    except in compliance with the License. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

    Software distributed under the License is distributed on an "AS
    IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
    implied. See the License for the specific language governing
    rights and limitations under the License.

    The initial developer of the original code is David A. Hinds
    <dahinds@users.sourceforge.net>.  Portions created by David A. Hinds
    are Copyright (C) 1999 David A. Hinds.  All Rights Reserved.

    Alternatively, the contents of this file may be used under the
    terms of the GNU General Public License version 2 (the "GPL"), in which
    case the provisions of the GPL are applicable instead of the
    above.  If you wish to allow the use of your version of this file
    only under the terms of the GPL and not to allow others to use
    your version of this file under the MPL, indicate your decision
    by deleting the provisions above and replace them with the notice
    and other provisions required by the GPL.  If you do not delete
    the provisions above, a recipient may use your version of this
    file under either the MPL or the GPL.

======================================================================*/
#include <base/common.h>

#include <rtochius/of.h>
#include <rtochius/of_fdt.h>
#include <rtochius/serial.h>

#include <asm/fixmap.h>

static struct earlycon_device early_console_dev = {
    .lock = __SPIN_LOCK_UNLOCKED(earlycon_device.lock),
    .available = 0,
};

static void __init earlycon_init(struct earlycon_device *device,
				 const char *name)
{
	const char *s;
	size_t len;

	/* scan backwards from end of string for first non-numeral */
	for (s = name + strlen(name);
	     s > name && s[-1] >= '0' && s[-1] <= '9';
	     s--)
		;
	if (*s)
		device->index = simple_strtoul(s, NULL, 10);
	len = s - name;
	strlcpy(device->name, name, min(len + 1, sizeof(device->name)));

	if (device->iotype == SERIAL_IO_MEM || device->iotype == SERIAL_IO_MEM16 ||
	    device->iotype == SERIAL_IO_MEM32 || device->iotype == SERIAL_IO_MEM32BE)
		pr_info("%s%d at MMIO%s %pa (options '%s')\n",
			device->name, device->index,
			(device->iotype == SERIAL_IO_MEM) ? "" :
			(device->iotype == SERIAL_IO_MEM16) ? "16" :
			(device->iotype == SERIAL_IO_MEM32) ? "32" : "32be",
			&device->mapbase, device->options);
	else
		pr_info("%s%d at I/O port 0x%lx (options '%s')\n",
			device->name, device->index,
			device->iobase, device->options);
}

int __init of_setup_earlycon(const struct earlycon_id *match,
			     unsigned long node,
			     const char *options)
{
    int err;
    const __be32 *val;    
	bool big_endian;
	u64 addr;

    early_console_dev.iotype = SERIAL_IO_MEM;
	addr = of_flat_dt_translate_address(node);
	if (addr == OF_BAD_ADDR) {
		pr_warn("[%s] bad address\n", match->name);
		return -ENXIO;
	}
    early_console_dev.mapbase = addr;

	val = of_get_flat_dt_prop(node, "reg-offset", NULL);
	if (val)
		early_console_dev.mapbase += be32_to_cpu(*val);
    early_console_dev.membase = __fixmap_remap_console(early_console_dev.mapbase, FIXMAP_PAGE_IO);

	val = of_get_flat_dt_prop(node, "reg-shift", NULL);
	if (val)
		early_console_dev.regshift = be32_to_cpu(*val);
	big_endian = of_get_flat_dt_prop(node, "big-endian", NULL) != NULL ||
		(IS_ENABLED(CONFIG_CPU_BIG_ENDIAN) &&
		 of_get_flat_dt_prop(node, "native-endian", NULL) != NULL);
	val = of_get_flat_dt_prop(node, "reg-io-width", NULL);
	if (val) {
		switch (be32_to_cpu(*val)) {
		case 1:
			early_console_dev.iotype = SERIAL_IO_MEM;
			break;
		case 2:
			early_console_dev.iotype = SERIAL_IO_MEM16;
			break;
		case 4:
			early_console_dev.iotype = (big_endian) ? SERIAL_IO_MEM32BE : SERIAL_IO_MEM32;
			break;
		default:
			pr_warn("[%s] unsupported reg-io-width\n", match->name);
			return -EINVAL;
		}
	}

	val = of_get_flat_dt_prop(node, "current-speed", NULL);
	if (val)
		early_console_dev.baud = be32_to_cpu(*val);

	val = of_get_flat_dt_prop(node, "clock-frequency", NULL);
	if (val)
		early_console_dev.clk = be32_to_cpu(*val);

	if (options) {
		early_console_dev.baud = simple_strtoul(options, NULL, 0);
		strlcpy(early_console_dev.options, options,
			sizeof(early_console_dev.options));
	}
    earlycon_init(&early_console_dev, match->name);
    strlcpy(early_console_dev.compatible, match->compatible, sizeof (early_console_dev.compatible));
    err = match->setup(&early_console_dev, options);
	if (err < 0)
		return err;
	if (!early_console_dev.write)
		return -ENODEV;

    early_console_dev.available = 1;

    return 0;
}

void earlycon_write(const char *s, unsigned int count)
{
    unsigned long flags;

    spin_lock_irqsave(&early_console_dev.lock, flags);
    if (unlikely(early_console_dev.available))
        early_console_dev.write(&early_console_dev, s, count);
    spin_unlock_irqrestore(&early_console_dev.lock, flags);
}

bool earlycon_device_available(void)
{
	return early_console_dev.available;
}
