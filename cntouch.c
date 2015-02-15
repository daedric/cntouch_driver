/*
 *  Copyright (c) 2015-2016 Thomas Sanchez
 *
 *  CNTouch driver
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/usb/input.h>

#define USB_VENDOR_ID_FOXCONN		0x294e
#define USB_DEVICE_ID_FOXCONN_CNTOUCH	0x1001

#define DRIVER_DESC	"CNTouch USB Driver"

static const struct usb_device_id cntouch_devices[] = {
	{USB_DEVICE(USB_VENDOR_ID_FOXCONN, USB_DEVICE_ID_FOXCONN_CNTOUCH)},
	{},
};

MODULE_AUTHOR("Thomas Sanchez, thomas.sanchz@gmail.com");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

MODULE_DEVICE_TABLE(usb, cntouch_devices);

struct cntouch_device {
	struct usb_device *usb_dev;
	struct input_dev *input_dev;
	struct urb *irq;
	signed char *data;
	dma_addr_t data_dma;
	char usb_path[64];
};

static void cntouch_irq(struct urb *urb)
{
	struct cntouch_device *cn_dev = urb->context;
	signed char *data = cn_dev->data;
	struct input_dev *dev = cn_dev->input_dev;
	int status;

	switch (urb->status) {
	case 0:
		break;
	case -ECONNRESET:
	case -ENOENT:
	case -ESHUTDOWN:
		return;
	default:
		goto resubmit;
	}

	input_report_rel(dev, REL_X, data[1]);
	input_report_rel(dev, REL_Y, data[2]);

	input_report_rel(dev, REL_WHEEL, data[3]);
	input_report_rel(dev, REL_HWHEEL, data[4]);

	input_report_key(dev, BTN_LEFT, data[0] & 0x01);
	input_report_key(dev, BTN_RIGHT, data[0] & 0x02);

	input_report_key(
	    dev, data[4] == 0 ? BTN_TOOL_FINGER : BTN_TOOL_DOUBLETAP, 1);

	input_sync(dev);

resubmit:
	status = usb_submit_urb(urb, GFP_ATOMIC);
	if (status) {
		dev_err(&cn_dev->usb_dev->dev,
			"can't resubmit intr, %s-%s/input0, status %d\n",
			cn_dev->usb_dev->bus->bus_name,
			cn_dev->usb_dev->devpath, status);
	}
}

static int cntouch_open(struct input_dev *dev)
{
	struct cntouch_device *cn_dev = input_get_drvdata(dev);

	if (usb_submit_urb(cn_dev->irq, GFP_KERNEL))
		return -EIO;
	return 0;
}

static void cntouch_close(struct input_dev *dev)
{
	struct cntouch_device *cn_dev = input_get_drvdata(dev);

	usb_kill_urb(cn_dev->irq);
}

static int cntouch_probe(struct usb_interface *interface,
			 const struct usb_device_id *id)
{
	struct usb_device *usb_dev = interface_to_usbdev(interface);
	struct cntouch_device *cn_dev;
	struct usb_host_interface *host_interface;
	struct usb_endpoint_descriptor *endpoint;
	int pipe, maxp;
	int error;

	host_interface = interface->cur_altsetting;

	if (host_interface->desc.bNumEndpoints != 1)
		return -ENODEV;

	endpoint = &host_interface->endpoint[0].desc;
	if (!usb_endpoint_is_int_in(endpoint))
		return -ENODEV;

	pipe = usb_rcvintpipe(usb_dev, endpoint->bEndpointAddress);
	maxp = usb_maxpacket(usb_dev, pipe, usb_pipeout(pipe));

	cn_dev = kzalloc(sizeof(*cn_dev), GFP_KERNEL);
	if (cn_dev == NULL) {
		error = -ENOMEM;
		goto err_1;
	}

	cn_dev->input_dev = input_allocate_device();
	if (!cn_dev->input_dev) {
		error = -ENOMEM;
		goto err_2;
	}

	cn_dev->data =
	    usb_alloc_coherent(usb_dev, 8, GFP_ATOMIC, &cn_dev->data_dma);
	if (!cn_dev->data) {
		error = -ENOMEM;
		goto err_3;
	}

	cn_dev->irq = usb_alloc_urb(0, GFP_KERNEL);
	if (!cn_dev->irq) {
		error = -ENOMEM;
		goto err_4;
	}

	usb_set_intfdata(interface, cn_dev);

	usb_make_path(usb_dev, cn_dev->usb_path, sizeof(cn_dev->usb_path));
	strlcat(cn_dev->usb_path, "/input0", sizeof(cn_dev->usb_path));

	cn_dev->input_dev->name = "CNTouch";
	cn_dev->input_dev->phys = cn_dev->usb_path;
	usb_to_input_id(usb_dev, &cn_dev->input_dev->id);
	cn_dev->input_dev->dev.parent = &interface->dev;

	__set_bit(EV_REL, cn_dev->input_dev->evbit);
	__set_bit(EV_KEY, cn_dev->input_dev->evbit);

	__set_bit(BTN_MOUSE          , cn_dev->input_dev->keybit);
	__set_bit(BTN_LEFT           , cn_dev->input_dev->keybit);
	__set_bit(BTN_RIGHT          , cn_dev->input_dev->keybit);
	__set_bit(BTN_TOOL_FINGER    , cn_dev->input_dev->keybit);
	__set_bit(BTN_TOOL_DOUBLETAP , cn_dev->input_dev->keybit);

	__set_bit(REL_X      , cn_dev->input_dev->relbit);
	__set_bit(REL_Y      , cn_dev->input_dev->relbit);
	__set_bit(REL_WHEEL  , cn_dev->input_dev->relbit);
	__set_bit(REL_HWHEEL , cn_dev->input_dev->relbit);

	input_set_drvdata(cn_dev->input_dev, cn_dev);

	cn_dev->input_dev->open = cntouch_open;
	cn_dev->input_dev->close = cntouch_close;

	usb_fill_int_urb(cn_dev->irq, usb_dev, pipe, cn_dev->data,
			 (maxp > 8 ? 8 : maxp), cntouch_irq, cn_dev,
			 endpoint->bInterval);
	cn_dev->irq->transfer_dma = cn_dev->data_dma;
	cn_dev->irq->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	error = input_register_device(cn_dev->input_dev);
	if (error)
		goto err_5;

	return 0;

err_5:
	usb_free_urb(cn_dev->irq);
err_4:
	usb_free_coherent(usb_dev, 8, cn_dev->data, cn_dev->data_dma);
err_3:
	input_free_device(cn_dev->input_dev);
err_2:
	usb_set_intfdata(interface, NULL);
	usb_put_dev(cn_dev->usb_dev);
	kfree(cn_dev);
err_1:
	return error;
}

static void cntouch_disconnect(struct usb_interface *interface)
{
	struct cntouch_device *cn_dev = usb_get_intfdata(interface);

	input_unregister_device(cn_dev->input_dev);
	usb_free_urb(cn_dev->irq);
	usb_free_coherent(cn_dev->usb_dev, 8, cn_dev->data, cn_dev->data_dma);
	usb_set_intfdata(interface, NULL);
	usb_put_dev(cn_dev->usb_dev);
	kfree(cn_dev);
}

static struct usb_driver cntouch_driver = {
	.name = "CNTouch",
	.probe = &cntouch_probe,
	.disconnect = &cntouch_disconnect,
	.id_table = cntouch_devices,
};

module_usb_driver(cntouch_driver);
