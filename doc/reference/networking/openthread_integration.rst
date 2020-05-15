.. _openthread_integration:

OpenThread Integration
######################

.. contents::
    :local:
    :depth: 2

Overview
********
The OpenThread stack is integrated with the Zephyr RTOS as the L2 layer.
The main advantage of this approach is that it can utilize the Zephyr's L3 layer.
The drawback, on the other hand, lies in the reception path in which the IP packet
needs to traverse the OpenThread L3 layer and then Zephyr's L3 layer in order to 
reach the BSD socket.

Note that if one desires the application can use just the OpenThread API 
with it's IPv6 stack and even the internal CoAP implementation 
which is the application protocol.

File system and shim layer
**************************
More details TODO

The OpenThread Network Stack is present in the following paths:
- OpenThread stack location: modules/lib/openthread/
- OpenThread shim layer location: zephyr/subsys/net/lib/openthread/platform/

The nRF IEEE802.15.4 Radio Driver is present in the following paths:
- nRF IEEE802.15.4 Radio Driver shim layer location: zephyr/drivers/ieee802154/{ieee802154_nrf5.c/ieee802154_nrf5.h
- nRF IEEE802.15.4 Radio Driver location: modules/hal/nordic/drivers/nrf_radio_802154

Threads
*******
- openthread - TODO
- rx_workq - TODO
- tx_workq - TODO
- sysworkq - TODO
- workqueue - TODO
- 802154 RX - resposible for the "upper half" processing of the radio frame. 
  Works on the objects of type nrf5_802154_rx_frame which are put to the nrf5_data.rx_fifo
  from the RX IRQ context. Then it is responsible of creating the net_pkt structure
  and passing it the upper layer with the net_recv_data().

Traffic flow
************
The traffic flow is not fully summetrical for the reception (RX) and the transmission (TX) cases.
Each of these flows is described below in an appropriate section.

RX path
*******
An application typically consists of one or more :ref:`threads <threads_v2>`
that execute the application logic. When using the
:ref:`BSD socket API <bsd_sockets_interface>`, the following things will
happen.

.. figure:: zephyr_netstack_openthread-rx_sequence.svg
    :alt: OpenThread Application RX data flow
    :figclass: align-center

    OpenThread Application RX data flow

Data receiving (RX) - TODO: expand, remove/rephrase copy/paste from the zephyrs RX page.
-------------------

1. A network data packet is received by a nRF5 IEEE 802.15.4 Radio Driver.

2. The device driver allocates enough network buffers to store the received
   data. The network packet is placed in the proper RX queue (implemented by
   :ref:`k_fifo <fifos_v2>`). By default there is only one receive queue in
   the system, but it is possible to have up to 8 receive queues.
   These queues will process incoming packets with different priority.
   See :ref:`traffic-class-support` for more details. The receive queues also
   act as a way to separate the data processing pipeline (bottom-half) as
   the device driver is running in an interrupt context and it must do its
   processing as fast as possible.

3. The 802154 RX Radio driver thread does the upper-half processing of the
   received IEEE 802.15.4 radio frame. As a result it puts a work item
   with the net_recv_data() to have the frame processed.

4. The work queue thread rx_workq calls the registered handler for every queued frame.
   In this case the registered handler openthread_recv() checks if the frame is of the 
   IEEE 802.15.4 type and if it is the case it insertd the frame in the rx_pkt_fifo and returns the NET_OK.

5. The OpenThread thread gets a frame from the FIFO and processes it.
   It also handles IP header compression, reassembly of fragmented traffic.

6. As soon as it detects a valid IPv6 packet that needs to be handled by the 
   higher layer it calls the registered callback ot_receive_handler()
   which creates a buffer for a net_pkt that is going to be passed to the Zephyr's IP stack
   and calls the net_recv_data() to have it processed.

7. This time the openthread_recv() called by the work queue returns NET_CONTINUE
   indicating that the valid IPv6 packet is present and needs to be processed by
   the Zephyr's higher layer.

8. The net_ipv6_input() passes the packet the next higher layer.

9. The packet is then passed to L3 processing. If the packet is IP based,
   then the L3 layer processes the IPv6.

10. A socket handler then finds an active socket to which the network packet
   belongs and puts it in a queue for that socket, in order to separate the
   networking code from the application. Typically the application is run in
   userspace context and the network stack is run in kernel context.

11. The application will then receive the data and can process it as needed.
   The application should have used the
   :ref:`BSD socket API <bsd_sockets_interface>` to create a socket
   that will receive the data.

TX path
*******

.. figure:: zephyr_netstack_openthread-tx_sequence.svg
    :alt: OpenThread Application TX data flow
    :figclass: align-center

    OpenThread Application TX data flow

Data transmitting (TX)


1. The application uses the
   :ref:`BSD socket API <bsd_sockets_interface>` when sending the data.
   However, direct interaction with the OpenThread API is possible - e.g.
   to utilize it's CoAP implementation.

2. The application data is prepared for sending to kernel space and then
   copied to internal net_buf structures.

3. Depending on the socket type, a protocol header is added in front of the
   data. For example, if the socket is a UDP socket, then a UDP header is
   constructed and placed in front of the data.

4. A UDP net_pkt is queued to be processed with the process_tx_packet().
   In the call chain the openthread_send() is called wchich converts the
   net_pkt to the otMessage format and invokes the otIp6Send().
   In this step the message is processed by the OpenThread's stack.

5. The tasklet to schedule the transmission is posted and semaphore unlocking the
   openthread thread is given. Mac and Submac operations take place here.

6. The openthread thread creates and schedules a work item used to transmit 
   the IEEE802.15.4 frame.

7. The nRF5 IEEE 802.15.4 Radio Driver sends the packet.