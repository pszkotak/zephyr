.. _openthread_integration:

OpenThread Integration
######################

.. contents::
    :local:
    :depth: 2

Overview
********
TODO

File system and shim layer
**************************
TODO

Threads
*******
TODO

Traffic flow
************
TODO

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

4. The work queue thread rx_workq calls the openthread_recv() which inserts
   the frame in the rx_pkt_fifo and returns the NET_OK.

5. The OpenThread thread gets a frame from the FIFO and consumes it.
   It also handles IP header compression, reassembly of fragmented traffic.

6. As soon as it detects a valid IPv6 packet that needs to be handled by the 
   higher layer it calls the registered callback ot_receive_handler()
   wchich calls the net_recv_data() to have it processed.

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

Data transmitting (TX) - TODO: expand, remove/rephrase copy/paste from the zephyrs TX page.


1. The application uses the
   :ref:`BSD socket API <bsd_sockets_interface>` when sending the data.
   However, direct interaction with the OpenThread API is possible - e.g.
   to utilize it's CoAP implementation.

2. The application data is prepared for sending to kernel space and then
   copied to internal net_buf structures.

3. Depending on the socket type, a protocol header is added in front of the
   data. For example, if the socket is a UDP socket, then a UDP header is
   constructed and placed in front of the data.

4. An IP header is added to the network packet for a UDP or TCP packet.

5. The network stack will check that the network interface is properly set
   for the network packet, and also will make sure that the network interface
   is enabled before the data is queued to be sent.

6. The network packet is then classified and placed to the proper transmit
   queue (implemented by :ref:`k_fifo <fifos_v2>`). By default there is only
   one transmit queue in the system, but it is possible to have up to 8
   transmit queues. These queues will process the sent packets with different
   priority. See :ref:`traffic-class-support` for more details.
   After the transmit packet classification, the packet is checked by the
   correct L2 layer module. The L2 module will do additional checks for the
   data and it will also create any L2 headers for the network packet.
   If everything is ok, the data is given to the network device driver to be
   sent out.

7. The device driver will send the packet to the network.

Note that in both the TX and RX data paths, the queues
(:ref:`k_fifo's <fifos_v2>`) form separation points where data is passed from
one :ref:`thread <threads_v2>` to another.
These :ref:`threads <threads_v2>` might run in different contexts
(:ref:`kernel <kernel_api>` vs. :ref:`userspace <usermode_api>`) and with different
:ref:`priorities <scheduling_v2>`.