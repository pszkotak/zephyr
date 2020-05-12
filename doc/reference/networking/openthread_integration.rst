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
TODO: svg