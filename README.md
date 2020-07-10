# TCP protocol implemented w/ advancements 
Implementation of a TCP protocol

* The main features of our TCP program
1. The sender sends packets depending on its current window size
2. The receiver gets the packets and sends cumulative ACKs
3. The receiver handles out-of-order packets by placing them on the disc using their sequence number to not mix up order of packets. Therefore the final file is identical to what the sender has sent
4. In case ACK is not received at the sender it resends the corresponding packets
5. To increase the efficiency of a file transmission we implemented congestion controll

All test transmissions resulted in correct file receives with the same checksums.

You can see cwnd chages in cwnd_size_plot.png in our repository as well as efficient exploitation of throughput in throughput_plot.png

The sender puts window sizes with timestamps into csv file. To build the plot run plot_cwnd.py at the same directory as cwnd.csv.

To plot the througput use wireshark.

Congestion controll mechanisms

1. Slow start
The sender starts sending packets with window size of one and increases the size by one per new ACKed packet. It will increase the cwnd size in exponential fasion. In other words, cwnd gets doubled after each RTT.
In the beginning, slow start threshold is set to 64. As soon as window size reaches 64 the congestion control mode is switched to Congestion Avoidance.

2. Congestion avoidance
This mode, as opposed to Slow Start, is not agressive at all. Window size increases by 1/window size per individual ACK which is window size + 1 after full window gets ACKed

3. Fast retransmit
When the sender receives 3 duplicate ACKs, it goes back to Slow start with window size of 1 and ssthresh = max(2, 0.5*window_size). In case of timeout the sender changes the mode to Slow Start as well(mostly because of timeout).

4. Initially, we implemented fast recovery which was supposed to happen after fast retransmit. That is setting ssthrsh to max(2, 0.5window_size) and window size to ssthresh and start increasing the window size in the same pace as in Congestion AVoidance. However, we empirically found out that for big files in high packet loss and timeout rate TCP performs better (two times faster) without fast recovery. Therefore, we had to remove Fast recovery and replace it with slow start.




